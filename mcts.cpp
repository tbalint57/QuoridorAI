// part 3 application is on the website 13th of february!!!
#include <stdint.h>
#include <cstdlib>
#include <math.h>
#include <queue>
#include "board.cpp"

float MCTS_CONST = 1;

vector<uint8_t> movesExecuted = {};
int number_of_retries = 0;
int number_of_tries = 0;

class Node
{
public:
    Node* children[256] = {nullptr};
    Node* parent;
    bool player;
    bool expanded;
    int whiteWins;
    int blackWins;
    int depth;

    void expandNode(Board* board){
        uint8_t possibleMoves[256];
        size_t moveCount = 0;
        board->generatePossibleMoves(this->player, possibleMoves, moveCount);
        for(int i = 0; i < moveCount; i++){
            uint8_t child = possibleMoves[i];
            this->children[child] = new Node(this, !this->player);
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

        return wins / n_node + MCTS_CONST * sqrt(log(n_parent) / n_node);
    }


    Node(Node* parent, bool player){
        this->parent = parent;
        this->player = player;
        this->expanded = false;
        this->whiteWins = 0;
        this->blackWins = 0;
        this->depth = parent ? parent->depth + 1 : 0;
    }


    ~Node() {
        for(int i = 0; i < 256; i++){
            if(this->children[i]){
                delete this->children[i];
            }
        }
    }

};


Node* findLeaf(Node* node, Board* board);
bool rollout(Board* board, bool player);
void backpropagate(Node* node, bool result);
uint8_t rolloutPolicy(Board* board, bool player);
uint8_t bestUCT(Node* node);
uint8_t mostVisitedMove(Node* node);


uint8_t mcts(Board state, int rollouts, bool whiteTurn){
    Node *root = new Node(nullptr, whiteTurn);
    Board board = Board(state);

    while(rollouts > 0){
        Node* leaf = findLeaf(root, &board);
        bool simulationResult = rollout(&board, leaf->player);
        backpropagate(leaf, simulationResult);

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



uint8_t rolloutPolicy(Board* board, bool player){
    return rolloutPolicy_fullRandom(board, player);
}

void backpropagate(Node* node, bool result){
    while(node){
        result ? node->whiteWins++ : node->blackWins++;
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

        if(child->getValue(node->player) == -1){
            return move;
        }

        if(child->getValue(node->player) > bestValue){
            bestValue = child->getValue(node->player);
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

#include <chrono> 

int main(int argc, char const *argv[]){
    srand (time(NULL));

    Board board = Board();
    bool whiteTurn = true;


    auto start = chrono::high_resolution_clock::now();

    uint8_t move = mcts(board, 50000, whiteTurn);
    cout << board.translateMove(move) << endl;

    auto end = chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    cout << "Move made in " << duration.count() << " seconds" << endl;

    // for(int i = 0; i < 100; i++){
    //     auto start = chrono::high_resolution_clock::now();

    //     uint8_t move = mcts(board, 10000, whiteTurn);
    //     cout << board.translateMove(move) << endl;

    //     auto end = chrono::high_resolution_clock::now();
    //     std::chrono::duration<double> duration = end - start;
    //     cout << "Move made in " << duration.count() << " seconds" << endl;

    //     board.executeMove(move, whiteTurn);
    //     if(board.getWinner()){
    //         break;
    //     }
    //     whiteTurn = !whiteTurn;
    // }
}

// lookup gcc profiler!!!