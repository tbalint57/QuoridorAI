#include <stdint.h>
#include <cstdlib>
#include <math.h>
#include <queue>
#include <thread>
#include <atomic>
#include <future>
#include "board.cpp"

/*
    Modifications to talk about, supervision 2025.02.13.:
        * integrated shortest path move (debugged)
    
    TODO:
        * research and implement GAUSSIAN PROCESSES!!!

*/

float MCTS_CONST = sqrt(2);

vector<uint8_t> movesExecuted = {};
int number_of_retries = 0;
int number_of_tries = 0;

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
    int depth;

    void expandNode(Board* board){
        uint8_t possibleMoves[256];
        size_t moveCount = 0;
        board->generatePossibleMoves(this->player, possibleMoves, moveCount);

        // Do not want to preallocate, as that would result in a ~100x memory usage.
        this->children = (Node**) malloc(256 * sizeof(Node*));

        // Init to nullptr, so we avoid that nasty-nasty segfault.
        for(int i = 0; i < 256; i++){
            this->children[i] = nullptr;
        }

        for(int i = 0; i < moveCount; i++){
            uint8_t child = possibleMoves[i];
            // Prioritise pawn moves a bit
            float childHeuristicValue = board->calculateHeuristicForMove(child, this->player);

            this->children[child] = new Node(this, !this->player, childHeuristicValue);
        }
        this->expanded = true;
    }


    float getValue(bool player, float mctsParameter){
        float n_node = (float)(this->whiteWins + this->blackWins);

        if (!n_node){
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
        this->depth = parent ? parent->depth + 1 : 0;
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


Node* findLeaf(Node* node, Board* board, float mctsParameter);
bool rollout(Board* board, bool player, int rolloutPolicyParameter);
void backpropagate(Node* node, int whiteWins, int blackWins);
uint8_t rolloutPolicy(Board* board, bool player, int rolloutPolicyParameter);
uint8_t bestUCT(Node* node, float mctsParameter);
uint8_t mostVisitedMove(Node* node, float mctsParameter);
Node* buildTree(Board state, int rollouts, int simulationsPerRollout, bool whiteTurn, int rolloutPolicyParameter, float mctsParameter);
void nodeVisits(Node* node, int* moves);


uint8_t mctsGetBestMove(Board state, int rollouts, int simulationsPerRollout, bool whiteTurn, int rolloutPolicyParameter, float mctsParameter){
    Node *mctsTree = buildTree(state, rollouts, simulationsPerRollout, whiteTurn, rolloutPolicyParameter, mctsParameter);
    uint8_t bestMove = mostVisitedMove(mctsTree, mctsParameter);

    delete(mctsTree);
    return bestMove;
}


void mctsDistribution(Board state, int rollouts, int simulationsPerRollout, bool whiteTurn, int* distribution, int rolloutPolicyParameter, float mctsParameter){
    Node *mctsTree = buildTree(state, rollouts, simulationsPerRollout, whiteTurn, rolloutPolicyParameter, mctsParameter);
    nodeVisits(mctsTree, distribution);
    delete(mctsTree);
}


Node* buildTree(Board state, int rollouts, int simulationsPerRollout, bool whiteTurn, int rolloutPolicyParameter, float mctsParameter){
    srand(time(NULL));
    Node *root = new Node(nullptr, whiteTurn, 0);
    Board board = Board(state);

    while(rollouts > 0){
        Node* leaf = findLeaf(root, &board, mctsParameter);

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
            simulationResult[i] = rollout(&boardCopy, leaf->player, rolloutPolicyParameter);
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
        movesExecuted = {};
        rollouts--;
    }

    return root;
}


Node* findLeaf(Node* node, Board* board, float mctsParameter){
    while(node->expanded){
        uint8_t bestMove = bestUCT(node, mctsParameter);

        board->executeMove(bestMove, node->player);
        movesExecuted.push_back(bestMove);

        node = node->children[bestMove];
    }

    // When a board has a winner, it must be a leaf
    if(board->getWinner() != 0){
        return node;
    }

    node->expandNode(board);
    uint8_t bestMove = bestUCT(node, mctsParameter);

    board->executeMove(bestMove, node->player);
    movesExecuted.push_back(bestMove);

    node = node->children[bestMove];
    return node;
}


bool rollout(Board* board, bool player, int rolloutPolicyParameter){
    for(int i = 0; i < 40; i++){
        uint8_t bestMove = rolloutPolicy(board, player, rolloutPolicyParameter);
        board->executeMove(bestMove, player);
        // movesExecuted.push_back(bestMove);
        // number_of_tries ++;

        player = !player;

        if(board->getWinner()){
            return board->getWinner() == 'w';
        }
    }

    return board->whiteCloser(player);
}


inline uint8_t rolloutPolicy_fullRandom(Board* board, bool player){
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


inline uint8_t rolloutPolicy_halfProbabilityOfPawnMovement(Board* board, bool player){
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


inline uint8_t rolloutPolicy_BestPawnMovement(Board* board, bool player){
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


uint8_t rolloutPolicy(Board* board, bool player, int rolloutPolicyParameter){
    uint8_t possibleMoves[256];
    size_t moveCount = 0;

    bool pawnMove = rand() % rolloutPolicyParameter;

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
