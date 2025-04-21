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

float MCTS_CONST = 0.25;

int number_of_calls = 0;

Quoridor_GP whiteModels[21];
Quoridor_GP blackModels[21];
Quoridor_GP smallWhiteModels[21];
Quoridor_GP smallBlackModels[21];

void loadModelsOnce() {
    static bool loaded = false;
    if (loaded) return;
    cout << "load in started" << endl;

    // Absolute path needed for UI integration, figuring this out took an embarassingly long time (roughly 3 hours), if anyone reads this feel free to roast.
    std::filesystem::path modelDir = std::filesystem::absolute("GPmodels");

    for (int i = 0; i < 21; i++) {
        whiteModels[i] = Quoridor_GP::load((modelDir / ("whiteModel" + to_string(i))).string());
        blackModels[i] = Quoridor_GP::load((modelDir / ("blackModel" + to_string(i))).string());
        smallWhiteModels[i] = Quoridor_GP::load((modelDir / ("whiteModelSmall" + to_string(i))).string());
        smallBlackModels[i] = Quoridor_GP::load((modelDir / ("blackModelSmall" + to_string(i))).string());
    }

    loaded = true;
    cout << "load in successful" << endl;
}

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

    VectorXd calculateHeuristicForChildren(Board* board, bool useSmallModel = true){
        int wallsOnBoard = 20 - board->whiteWalls - board->blackWalls;
        Quoridor_GP* model = player ? whiteModels + wallsOnBoard : blackModels + wallsOnBoard;
        if (useSmallModel) {
            model = player ? smallWhiteModels + wallsOnBoard : smallBlackModels + wallsOnBoard;
        }

        return model->predict(board->toInputVector(player));
    }


    void expandNode(Board* board, int heuristics = 0, int heuristicsWeight = 10){
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

        if (heuristics) {
            number_of_calls++;
            VectorXd childrenHeuristic = calculateHeuristicForChildren(board, heuristics==1);
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


    float getValue(bool player, float mctsParameter){
        float n_node = (float)(this->whiteWins + this->blackWins);

        if (n_node == 0){
            return -1;
        }

        float n_parent = (float)(parent ? this->parent->whiteWins + this->parent->blackWins : n_node);

        float wins = (float)(player ? whiteWins : blackWins);

        return (this->heuristicValue + wins) / n_node + mctsParameter * sqrt(log(n_parent) / n_node);
    }


    Node(Node* parent, bool player, float heuristicValue){
        this->parent = parent;
        this->player = player;
        this->expanded = false;
        this->whiteWins = 0;
        this->blackWins = 0;
        this->heuristicValue = heuristicValue;
    }


    ~Node() {
        if(!this->children){
            return;
        }

        for(int i = 0; i < 256; i++){
            if(this->children[i]){
                delete this->children[i];
            }
        }

        free(this->children); // Free the array of child pointers
    }
};


Node* findLeaf(Node* node, Board* board, float mctsParameter, bool useModelForUCT);
bool rollout(Board* board, bool player, int rolloutPolicyParameter, int useModelForRollout);
void backpropagate(Node* node, int whiteWins, int blackWins);
uint8_t rolloutPolicy(Board* board, bool player, int rolloutPolicyParameter, int useModelForRollout);
uint8_t bestUCT(Node* node, float mctsParameter);
uint8_t mostVisitedMove(Node* node, float mctsParameter);
Node* buildTree(Board state, int rollouts, int simulationsPerRollout, bool whiteTurn, int rolloutPolicyParameter, float mctsParameter, bool useModelForUCT, int useModelForRollout);
void nodeVisits(Node* node, int* moves);


uint8_t mctsGetBestMove(Board state, int rollouts, int simulationsPerRollout, bool whiteTurn, int rolloutPolicyParameter, float mctsParameter, bool useModelForUCT = true, int useModelForRollout = 0){
    Node *mctsTree = buildTree(state, rollouts, simulationsPerRollout, whiteTurn, rolloutPolicyParameter, mctsParameter, useModelForUCT, useModelForRollout);
    uint8_t bestMove = mostVisitedMove(mctsTree, mctsParameter);

    delete(mctsTree);
    return bestMove;
}


void mctsDistribution(Board state, int rollouts, int simulationsPerRollout, bool whiteTurn, int* distribution, int rolloutPolicyParameter, float mctsParameter, bool useModelForUCT = true, int useModelForRollout = 0){
    Node *mctsTree = buildTree(state, rollouts, simulationsPerRollout, whiteTurn, rolloutPolicyParameter, mctsParameter, useModelForUCT, useModelForRollout);
    nodeVisits(mctsTree, distribution);
    delete(mctsTree);
}


Node* buildTree(Board state, int rollouts, int simulationsPerRollout, bool whiteTurn, int rolloutPolicyParameter, float mctsParameter, bool useModelForUCT, int useModelForRollout){
    loadModelsOnce();

    srand(time(NULL));
    Node *root = new Node(nullptr, whiteTurn, 0);
    Board board = Board(state);

    while(rollouts > 0){
        Node* leaf = findLeaf(root, &board, mctsParameter, useModelForUCT);

        // Threading does not work, too much overhead for thread creation: ~100 usec, while a single rollout is roughly 50 usec
        // // =========== THREADING START ==========
        // vector<int> simulationResult(simulationsPerRollout);
        // vector<thread> threads;
        // for(int i = 0; i < simulationsPerRollout; i++){
        //     threads.emplace_back([&, i]() {
        //         Board boardCopy = board;
        //         simulationResult[i] = rollout(&boardCopy, leaf->player rolloutPolicyParameter);
        //     });
        // }

        // // Run threads
        // for (thread& th : threads) {
        //     th.join();
        // }
        // // =========== THREADING END ===========



        // =========== NORMAL START ==========
        vector<bool> simulationResult(simulationsPerRollout);
        for(int i = 0; i < simulationsPerRollout; i++){
            Board boardCopy = board;
            simulationResult[i] = rollout(&boardCopy, leaf->player, rolloutPolicyParameter, useModelForRollout);
        }
        // =========== NORMAL END ===========


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
        rollouts--;
    }

    return root;
}


Node* findLeaf(Node* node, Board* board, float mctsParameter, bool useModelForUCT){
    int depth = 0;
    while(node->expanded){
        uint8_t bestMove = bestUCT(node, mctsParameter);

        board->executeMove(bestMove, node->player);

        node = node->children[bestMove];

        depth++;
    }

    // When a board has a winner, it must be a leaf
    if(board->getWinner() != 0){
        return node;
    }

    int modelUsage = 0;
    if (useModelForUCT) {
        if(depth <= 2) modelUsage = 1;
        if(depth == 0) modelUsage = 2;
    }


    node->expandNode(board, 0, depth ? 10 : 100);
    uint8_t bestMove = bestUCT(node, mctsParameter);

    board->executeMove(bestMove, node->player);

    node = node->children[bestMove];
    return node;
}


bool rollout(Board* board, bool player, int rolloutPolicyParameter, int useModelForRollout){
    for(int i = 0; i < 40; i++){
        uint8_t bestMove = rolloutPolicy(board, player, rolloutPolicyParameter, useModelForRollout);
        board->executeMove(bestMove, player);

        player = !player;

        if(board->getWinner()){
            return board->getWinner() == 'w';
        }
    }

    return board->whiteCloser(player);
}


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


uint8_t rolloutPolicy(Board* board, bool player, int rolloutPolicyParameter, int useModelForRollout){
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


void backpropagate(Node* node, int whiteWins, int blackWins){
    while(node){
        node->whiteWins += whiteWins;
        node->blackWins += blackWins;
        node = node->parent;
    }
}


uint8_t bestUCT(Node* node, float mctsParameter){
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
        bestMove = bestUCT(node, mctsParameter);
    }

    return bestMove;
}


uint8_t mostVisitedMove(Node* node, float mctsParameter){
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


void nodeVisits(Node* node, int* moves){
    for (int move = 0; move < 256; move++){
        Node* child = node->children[move];
        if(!child){
            continue;
        }

        moves[move] = child->whiteWins + child->blackWins;
    }
}