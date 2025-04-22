#include <stdint.h>
#include <cstdlib>
#include <math.h>
#include <queue>
#include <thread>
#include <atomic>
#include <future>
#include <Eigen/Dense>
#include <filesystem> 
#include "gaussianProcess.cpp"

using namespace Eigen;

float MCTS_CONST = 0.42;


/**
 * Represents a node in the game tree for the Quoridor AI.
 * Each node contains game state information and MCTS statistics.
 */
class Node
{
public:
    Node** children = nullptr;
    Node* parent;
    bool player;
    bool expanded;
    int whiteWins;
    int blackWins;
    int heuristicValue;


    /**
     * Expands the current node by generating its child nodes.
     * Optionally calculates heuristics for children and weights them.
     *
     * @param board Pointer to the current board state
     * @param heuristicsWeight Multiplier for heuristic value
     * @param model model used for prediction
     * @return void
     */
    void expandNode(Board* board, int heuristicsWeight = 10, Quoridor_GP* model = nullptr){
        uint8_t possibleMoves[256];
        size_t moveCount = 0;
        board->generatePossibleMoves(this->player, possibleMoves, moveCount);

        // Do not want to preallocate, as that would result in a ~100x memory usage.
        this->children = (Node**) malloc(256 * sizeof(Node*));

        // Init to nullptr, so we avoid that nasty-nasty segfault.
        for(int i = 0; i < 256; i++){
            this->children[i] = nullptr;
        }

        this->expanded = true;

        if (model) {
            VectorXd childrenHeuristic = model->predict(board->toInputVector(player));
            for(int i = 0; i < moveCount; i++){
                uint8_t child = possibleMoves[i];    
                this->children[child] = new Node(this, !this->player, (float) childrenHeuristic(child) * heuristicsWeight);
            }

            return;
        }

        for(int i = 0; i < moveCount; i++){
            uint8_t child = possibleMoves[i];    
            this->children[child] = new Node(this, !this->player, 0);
        }
    }


    /**
     * Calculates the value of the node based on MCTS and heuristic evaluation.
     *
     * @param player Perspective of the player (true = white, false = black)
     * @param mctsParameter Exploration constant used in UCT formula
     * @return float Value of the node
     */
    float getValue(bool player, float mctsParameter){
        float n_node = (float)(this->whiteWins + this->blackWins);

        if (n_node == 0){
            return -1;
        }

        float n_parent = (float)(parent ? this->parent->whiteWins + this->parent->blackWins : n_node);

        float wins = (float)(player ? whiteWins : blackWins);

        return (this->heuristicValue + wins) / n_node + mctsParameter * sqrt(log(n_parent) / n_node);
    }


    /**
     * Node constructor
     * 
     * @param parent Pointer to the parent node
     * @param player true: white's turn, false: black's turn
     * @param heuristicValue Heuristic value assigned to this node
     */
    Node(Node* parent, bool player, float heuristicValue){
        this->parent = parent;
        this->player = player;
        this->expanded = false;
        this->whiteWins = 0;
        this->blackWins = 0;
        this->heuristicValue = heuristicValue;
    }


    /**
     * Node destructor
     * Recursively deletes all child nodes and frees memory
     */
    ~Node() {
        if(!this->children){
            return;
        }

        for(int i = 0; i < 256; i++){
            if(this->children[i]){
                delete this->children[i];
            }
        }

        // Free the array of child pointers, yes, it has been pointed out, that I should've used new and delete instead...
        free(this->children); 
    }
};


class MCTS
{
    public: 

    Quoridor_GP whiteModels[21] = {};
    Quoridor_GP blackModels[21] = {};
    Quoridor_GP smallWhiteModels[21] = {};
    Quoridor_GP smallBlackModels[21] = {};

    int rollouts = 50000;
    int simulationsPerRollout = 5;

    float mctsParameter = 0.5;
    int rolloutPolicyParameter = 4;

    string modelDirectory = "GPmodels";
    bool useModelForUCT = true;
    int useModelForRollout = 0;

    /**
     * Loads in all the GP models from the model directory
     * 
     * @param modelDirectory the directory of the models
     * @return void
     */
    void loadModels() {
        if (modelDirectory == ""){
            return;
        }
        
        std::filesystem::path modelDir = std::filesystem::absolute(modelDirectory);
    
        for (int i = 0; i < 21; i++) {
            whiteModels[i] = Quoridor_GP::load((modelDir / ("whiteModel" + to_string(i))).string());
            blackModels[i] = Quoridor_GP::load((modelDir / ("blackModel" + to_string(i))).string());
            smallWhiteModels[i] = Quoridor_GP::load((modelDir / ("whiteModelSmall" + to_string(i))).string());
            smallBlackModels[i] = Quoridor_GP::load((modelDir / ("blackModelSmall" + to_string(i))).string());
        }
    }


    /**
     * Runs MCTS and returns the best move according to visit count.
     * This is used when predicting agent's next move.
     * 
     * @param state Current board state
     * @param whiteTurn Whether it's white's turn
     * @return uint8_t Best move determined by MCTS
     */
    uint8_t predictBestMove(Board state, bool whiteTurn){
        Node* mctsTree = buildTree(state, whiteTurn);
        uint8_t bestMove = mostVisitedMove(mctsTree);

        delete(mctsTree);
        return bestMove;
    }


    /**
     * Runs MCTS and fills a distribution array with visit counts for all moves.
     * This is used for data generation and data relabelling.
     * 
     * @param state Current board state
     * @param whiteTurn Whether it's white's turn
     * @param distribution Array to fill with visit counts
     * @return void
     */
    void predictDistribution(Board state, bool whiteTurn, int* distribution){
        Node *mctsTree = buildTree(state, whiteTurn);
        nodeVisits(mctsTree, distribution);
        delete(mctsTree);
    }


    /**
     * Builds the MCTS tree from the given state.
     * 
     * @param state Initial board state
     * @param whiteTurn Whether it's white's turn
     * @return Node* Root of the built MCTS tree
     */
    Node* buildTree(Board state, bool whiteTurn){
        srand(time(NULL));
        Node *root = new Node(nullptr, whiteTurn, 0);
        Board board = Board(state);

        int rolloutsCompleted = 0;
        while(rollouts > rolloutsCompleted){
            Node* leaf = findLeaf(root, &board);
            vector<bool> simulationResult(simulationsPerRollout);
            for(int i = 0; i < simulationsPerRollout; i++){
                Board boardCopy = board;
                simulationResult[i] = rollout(&boardCopy, leaf->player);
            }

            int whiteWins = 0;
            int blackWins = 0;

            for(int i = 0; i < simulationsPerRollout; i++){
                if(simulationResult[i]){
                    whiteWins++;
                }else{
                    blackWins++;
                }
            }

            backpropagate(leaf, whiteWins, blackWins);

            board = state;
            rolloutsCompleted++;
        }

        return root;
    }


    /**
     * Traverses the MCTS tree to find a leaf node to expand.
     * Applies UCT to descend, and expands node if not terminal.
     * Optionally uses GP model for expansion guidance.
     * 
     * @param node Current node in the MCTS tree
     * @param board Pointer to the board being simulated
     * @return Node* Leaf node ready for simulation
     */
    Node* findLeaf(Node* node, Board* board){
        int depth = 0;
        while(node->expanded){
            uint8_t bestMove = bestUCT(node);

            board->executeMove(bestMove, node->player);

            node = node->children[bestMove];

            depth++;
        }

        // When a board has a winner, it must be a leaf
        if(board->getWinner() != 0){
            return node;
        }

        int wallsOnBoard = 20 - board->blackWalls - board->whiteWalls;
        Quoridor_GP* model = nullptr;
        if (useModelForUCT) {
            if(depth <= 1) model = node->player ? smallWhiteModels + wallsOnBoard : smallBlackModels + wallsOnBoard;
            if(depth == 0) model = node->player ? whiteModels + wallsOnBoard : blackModels + wallsOnBoard;
        }



        node->expandNode(board, depth ? 10 : 100, model);
        uint8_t bestMove = bestUCT(node);

        board->executeMove(bestMove, node->player);

        node = node->children[bestMove];
        return node;
    }


    /**
     * Performs a single rollout simulation.
     * 
     * @param board Game board to simulate on
     * @param player true: white, false: black
     * @return bool true if white wins, false if black wins
     */
    bool rollout(Board* board, bool player){
        for(int i = 0; i < 40; i++){
            uint8_t bestMove = rolloutPolicy(board, player);
            board->executeMove(bestMove, player);

            player = !player;

            if(board->getWinner()){
                return board->getWinner() == 'w';
            }
        }

        return board->whiteCloser(player);
    }


    /**
     * Samples a move based on the GP model's prediction distribution.
     * 
     * @param board Game board
     * @param player true: white, false: black
     * @param model Pointer to GP model to use
     * @return uint8_t Sampled move
     */
    uint8_t generateMoveFromModel(Board* board, bool player, Quoridor_GP* model){
        VectorXd pred = model->predict(board->toInputVector(player));
        double r = ((double) rand() /(RAND_MAX));
        int move = 0;

        for(;move < 256; move++){
            r -= pred(move);
            if(r <= 0){
                break;
            }
        }

        return move;
    }


    /**
     * Rollout policy: Fully random (with 3 retries for wall validation).
     * 
     * @param board Game board
     * @param player true: white, false: black
     * @return uint8_t Selected move
     */
    uint8_t rolloutPolicy_fullRandom(Board* board, bool player){
        uint8_t possibleMoves[256];
        size_t moveCount = 0;
        int tries = 0;

        board->generatePossibleMovesUnchecked(player, possibleMoves, moveCount);

        while(tries < 3){
            uint8_t move = possibleMoves[rand() % moveCount];

            if(move >> 7){
                uint8_t wallPlacement = move & 0b01111111;
                if(!board->isValidWallPlacement(wallPlacement)){
                    tries++;
                    continue;
                }
            }
            return move;
        }

        // possibleMoves[0] is a pawn movement and thus definitely valid.
        return possibleMoves[0];
    }


    /**
     * Rollout policy: 50% chance for pawn movement.
     * 
     * @param board Game board
     * @param player true: white, false: black
     * @return uint8_t Selected move
     */
    uint8_t rolloutPolicy_halfProbabilityOfPawnMovement(Board* board, bool player){
        uint8_t possibleMoves[256];
        size_t moveCount = 0;
        int tries = 0;

        int pawnMoves = board->generatePossibleMovesUnchecked(player, possibleMoves, moveCount);

        bool pawnMove = possibleMoves[rand() % 2];

        if (pawnMove == 0){
            uint8_t move = possibleMoves[rand() % pawnMoves];

            return move;
        }

        // we can get a pawn move here as well, but I don't care!
        while(tries < 3){
            uint8_t move = possibleMoves[rand() % moveCount];

            if(move >> 7){
                uint8_t wallPlacement = move & 0b01111111;
                if(!board->isValidWallPlacement(wallPlacement)){
                    tries++;
                    continue;
                }
            }
            return move;
        }

        // possibleMoves[0] is a pawn movement and thus definitely valid.
        return possibleMoves[0];
    }



    /**
     * Rollout policy: Prefer best pawn movement with 75%, fallback to probable.
     * 
     * @param board Game board
     * @param player true: white, false: black
     * @return uint8_t Selected move
     */
    uint8_t rolloutPolicy_BestPawnMovement(Board* board, bool player){
        uint8_t possibleMoves[256];
        size_t moveCount = 0;

        bool pawnMove = rand() % 4;

        if (pawnMove != 0){
            return board->generateMoveOnShortestPath(player);
        }

        board->generateProbableMovesUnchecked(player, possibleMoves, moveCount);

        int tries = 0;
        // we can get a pawn move here as well, but I don't care!
        while(tries < 3){
            uint8_t move = possibleMoves[rand() % moveCount];

            if(move >> 7){
                uint8_t wallPlacement = move & 0b01111111;
                if(!board->isValidWallPlacement(wallPlacement)){
                    tries++;
                    continue;
                }
            }
            return move;
        }

        // possibleMoves[0] is a pawn movement and thus definitely valid.
        return possibleMoves[0];
    }


    /**
     * Main rollout policy function. Chooses between shortest path move, GP model, or fallback.
     * Similar to rolloutPolicy_BestPawnMovement
     * 
     * @param board Current game board
     * @param player Current player (true: white, false: black)
     * @return uint8_t Move to execute
     */
    uint8_t rolloutPolicy(Board* board, bool player){
        uint8_t possibleMoves[256];
        size_t moveCount = 0;

        int pawnMove = rand() % rolloutPolicyParameter;

        if (pawnMove != 0){
            return board->generateMoveOnShortestPath(player);
        }

        if (useModelForRollout){
            int modelMove = rand() % useModelForRollout;
            if (!modelMove) {
                int wallsOnBoard = 20 - board->whiteWalls - board->blackWalls;
                Quoridor_GP *model = player ? smallWhiteModels + wallsOnBoard : smallBlackModels + wallsOnBoard;
                return generateMoveFromModel(board, player, model);
            }
        }

        board->generateProbableMovesUnchecked(player, possibleMoves, moveCount);

        int tries = 0;
        // we can get a pawn move here as well, but I don't care!
        while(tries < 3){
            uint8_t move = possibleMoves[rand() % moveCount];

            if(move >> 7){
                uint8_t wallPlacement = move & 0b01111111;
                if(!board->isValidWallPlacement(wallPlacement)){
                    tries++;
                    continue;
                }
            }
            return move;
        }

        // possibleMoves[0] is a pawn movement and thus definitely valid.
        return possibleMoves[0];
    }


    /**
     * Backpropagates the simulation result up the tree.
     * 
     * @param node Node from which to start backpropagation
     * @param whiteWins Number of white wins to propagate
     * @param blackWins Number of black wins to propagate
     * @return void
     */
    void backpropagate(Node* node, int whiteWins, int blackWins){
        while(node){
            node->whiteWins += whiteWins;
            node->blackWins += blackWins;
            node = node->parent;
        }
    }


    /**
     * Chooses the best move from a node using UCT formula.
     * 
     * @param node Current node in the MCTS tree
     * @return uint8_t Best move determined by UCT
     */
    uint8_t bestUCT(Node* node){
        float bestValue = -1;
        uint8_t bestMove = 0;

        for (int move = 0; move < 256; move++){
            Node* child = node->children[move];
            if(!child){
                continue;
            }

            float value = child->getValue(node->player, mctsParameter);

            if(value == -1){
                return move;
            }

            if(value > bestValue){
                bestValue = value;
                bestMove = move;
            }
        }

        if(!bestMove){
            cout << "bestUCT infinite loop\n";
            bestMove = bestUCT(node);
        }

        return bestMove;
    }


    /**
     * Chooses the move that has been visited the most times.
     * 
     * @param node Current node in the MCTS tree
     * @return uint8_t Move index with the highest number of visits
     */
    uint8_t mostVisitedMove(Node* node){
        int bestValue = -1;
        uint8_t bestMove = 0;

        for (int move = 0; move < 256; move++){
            Node* child = node->children[move];
            if(!child){
                continue;
            }

            if(child->whiteWins + child->blackWins > bestValue){
                bestValue = child->whiteWins + child->blackWins;
                bestMove = move;
            }
        }
        return bestMove;
    }


    /**
     * Populates an array with the number of visits for each move.
     * 
     * @param node Root of the MCTS tree
     * @param moves Output array with 256 entries
     * @return void
     */
    void nodeVisits(Node* node, int* moves){
        for (int move = 0; move < 256; move++){
            Node* child = node->children[move];
            if(!child){
                continue;
            }

            moves[move] = child->whiteWins + child->blackWins;
        }
    }


    /**
     * Constructs an MCTS object and initializes the model data.
     * 
     * @param rollouts Number of MCTS rollouts to perform
     * @param simulationsPerRollout Number of simulations per rollout
     * @param mctsParameter UCT exploration constant
     * @param rolloutPolicyParameter Parameter influencing rollout policy behavior
     * @param modelDirectory Directory containing the GP models
     * @param useModelForUCT Whether to use a GP model in UCT selection
     * @param useModelForRollout Frequency of using GP model in rollout (0 = never)
     */
    MCTS(int rollouts = 50000, int simulationsPerRollout = 5, float mctsParameter = 0.5, int rolloutPolicyParameter = 4, string modelDirectory = "GPmodels", bool useModelForUCT = true, int useModelForRollout = 0){
        this->rollouts = rollouts;
        this->simulationsPerRollout = simulationsPerRollout;

        this->mctsParameter = mctsParameter;
        this->rolloutPolicyParameter = rolloutPolicyParameter;

        this->modelDirectory = modelDirectory;
        this->useModelForUCT = useModelForUCT;
        this->useModelForRollout = useModelForRollout;

        loadModels();
    }
};