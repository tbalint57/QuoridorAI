// part 3 application is on the website 13th of february!!!
#include <stdint.h>
#include <cstdlib>
#include <math.h>
#include <queue>
#include <thread>
#include <atomic>
#include <future>
#include "board.cpp"

/*
    Modifications to talk about, supervision 2025.01.31.:
        * backpropagate rewrite for parallel simulation
    
    TODO:
        * use threads to simulate multiple games from ending parallel
        * tweak heuristics (shortest path move)
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


    float getValue(bool player){
        float n_node = (float)(this->whiteWins + this->blackWins);

        if (!n_node){
            return -1;
        }

        float n_parent = (float)(parent ? this->parent->whiteWins + this->parent->blackWins : n_node);

        float wins = (float)(player ? whiteWins : blackWins);

        return (this->heuristicValue + wins) / n_node + MCTS_CONST * sqrt(log(n_parent) / n_node);
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


Node* findLeaf(Node* node, Board* board);
bool rollout(Board* board, bool player);
void backpropagate(Node* node, int whiteWins, int blackWins);
uint8_t rolloutPolicy(Board* board, bool player);
uint8_t bestUCT(Node* node);
uint8_t mostVisitedMove(Node* node);


uint8_t mcts(Board state, int rollouts, bool whiteTurn){
    int simulationsPerRollout = 10;
    Node *root = new Node(nullptr, whiteTurn, 0);
    Board board = Board(state);

    while(rollouts > 0){
        Node* leaf = findLeaf(root, &board);

        // Threading does not work, too much overhead for thread creation: ~100 usec, while a single rollout is roughly 50 usec
        // // =========== THREADING START ==========
        // vector<int> simulationResult(simulationsPerRollout);
        // vector<thread> threads;
        // for(int i = 0; i < simulationsPerRollout; i++){
        //     threads.emplace_back([&, i]() {
        //         Board boardCopy = board;
        //         simulationResult[i] = rollout(&boardCopy, leaf->player);
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
            simulationResult[i] = rollout(&boardCopy, leaf->player);
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

    uint8_t bestMove = mostVisitedMove(root);
    delete(root);
    return bestMove;
}


Node* findLeaf(Node* node, Board* board){
    while(node->expanded){
        uint8_t bestMove = bestUCT(node);

        board->executeMove(bestMove, node->player);
        movesExecuted.push_back(bestMove);

        node = node->children[bestMove];
    }

    // When a board has a winner, it must be a leaf
    if(board->getWinner() != 0){
        return node;
    }

    node->expandNode(board);
    uint8_t bestMove = bestUCT(node);

    board->executeMove(bestMove, node->player);
    movesExecuted.push_back(bestMove);

    node = node->children[bestMove];
    return node;
}


bool rollout(Board* board, bool player){
    for(int i = 0; i < 40; i++){
        uint8_t bestMove = rolloutPolicy(board, player);
        board->executeMove(bestMove, player);
        movesExecuted.push_back(bestMove);
        number_of_tries ++;

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


inline uint8_t rolloutPolicy_probableNextMoveWithHalfProbabilityOfPawnMovement(Board* board, bool player){
    uint8_t possibleMoves[256];
    size_t moveCount = 0;
    int tries = 0;

    int pawnMoves = board->generateProbableMovesUnchecked(player, possibleMoves, moveCount);

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


uint8_t rolloutPolicy(Board* board, bool player){
    return rolloutPolicy_probableNextMoveWithHalfProbabilityOfPawnMovement(board, player);
}


void backpropagate(Node* node, int whiteWins, int blackWins){
    while(node){
        node->whiteWins += whiteWins;
        node->blackWins += blackWins;
        node = node->parent;
    }
}


uint8_t bestUCT(Node* node){
    float bestValue = -1;
    uint8_t bestMove = 0;

    for (int move = 0; move < 256; move++){
        Node* child = node->children[move];
        if(!child){
            continue;
        }

        float value = child->getValue(node->player);

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


uint8_t mostVisitedMove(Node* node){
    int bestValue = -1;
    uint8_t bestMove = 0;

    for (int move = 0; move < 256; move++){
        Node* child = node->children[move];
        if(!child){
            continue;
        }

        // cout << (int) move << " - whiteWins: " << child->whiteWins << ",\t\tblackWins: " << child->blackWins << "\n";

        if(child->getValue(node->player) == -1){
            return move;
        }

        if(child->whiteWins + child->blackWins > bestValue){
            bestValue = child->whiteWins + child->blackWins;
            bestMove = move;
        }
    }
    return bestMove;
}


// Currently here for testing, obviously to be removed


#include <chrono> 
int main(int argc, char const *argv[]){
    srand (time(NULL));
    Board board = Board();
    bool whiteTurn = true;

    auto start = chrono::high_resolution_clock::now();
    uint8_t move = mcts(board, 10000, whiteTurn);
    cout << board.translateMove(move) << endl;
    auto end = chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    cout << "Move made in " << duration.count() << " seconds" << endl;

    // for(int i = 0; i < 100; i++){
    //     auto start = chrono::high_resolution_clock::now();
    //     uint8_t move = mcts(board, 100000, whiteTurn);
    //     cout << board.translateMove(move) << endl;
    //     auto end = chrono::high_resolution_clock::now();
    //     std::chrono::duration<double> duration = end - start;
    //     cout << "Move made in " << duration.count() << " seconds" << endl;
    //     board.executeMove(move, whiteTurn);
    //     if(board.getWinner()){
    //         cout << "The winner is: " << board.getWinner() << endl;
    //         break;
    //     }
    //     whiteTurn = !whiteTurn;
    // }
    // cout << "Simulation finished" << endl;
}