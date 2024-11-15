#include <stdint.h>
#include <cstdlib>
#include <math.h>
#include "board.cpp"

class Node
{
public:
    Node* children[256] = {nullptr};
    Node* parent;
    bool player;
    bool expanded;
    int wins;
    int losses;
    float value;

    void expandNode(Board* board){
        vector<uint8_t> children = board->generatePossibleMoves(this->player);
        for(uint8_t child : children){
            this->children[child] = new Node(this, !this->player);
        }
        this->expanded = true;
    }


    void updateValue(){
        int n_parent, n_node = this->wins + this->losses;
        n_parent = (parent ? this->parent->wins + this->parent->losses : n_node);

        this->value = this->wins / n_node + sqrt(2) * sqrt(log(n_parent) / n_node);
    }


    Node(Node* parent, bool player){
        this->parent = parent;
        this->player = player;
        this->expanded = false;
        this->wins = 0;
        this->losses = 0;
        this->value = -1;
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


uint8_t mcts(int rollouts){
    Node *root = new Node(nullptr, true);
    Board board = Board();
    while(rollouts > 0){
        Node* leaf = findLeaf(root, &board);
        bool simulationResult = rollout(&board, leaf->player);
        backpropagate(leaf, simulationResult);

        board.reset();

        rollouts--;
    }

    return bestUCT(root);
}


Node* findLeaf(Node* node, Board* board){
    while(node->expanded){
        uint8_t bestMove = bestUCT(node);

        board->executeMove(bestMove, node->player);
        node = node->children[bestMove];
    }

    if(board->getWinner() != 0){
        return node;
    }

    node->expandNode(board);
    uint8_t bestMove = bestUCT(node);

    board->executeMove(bestMove, node->player);
    node = node->children[bestMove];
    return node;
}


bool rollout(Board* board, bool player){
    for(int i = 0; i < 90; i++){
        uint8_t bestMove = rolloutPolicy(board, player);
        board->executeMove(bestMove, player);
        player =! player;

        if(board->getWinner()){
            return board->getWinner() == 'w';
        }
    }

    return board->whiteCloser(player);
}


void exceptionCatcher(){
    cout << "alma";
}

uint8_t rolloutPolicy(Board* board, bool player){
    vector<uint8_t> possibleMoves = board->generatePossibleMoves(player);
    return possibleMoves[rand() % possibleMoves.size()];
}


void backpropagate(Node* node, bool result){
    while(node){
        result ? node->wins++ : node->losses++;
        node->updateValue();
        node = node->parent;
    }
}


uint8_t bestUCT(Node* node){
    float bestValue = (node->player ? -1000.0f : 1000.0f);
    uint8_t bestMove = 127;

    for (int move = 0; move < 256; move++){
        Node* child = node->children[move];
        if(!child){
            continue;
        }

        if(child->value == -1){
            return move;
        }

        bool betterValue = (node->player && child->value > bestValue) || (!node->player && child->value < bestValue);
        if(betterValue){
            bestValue = child->value;
            bestMove = move;
        }
    }

    return bestMove;
}


int main(int argc, char const *argv[]){
    cout << (int) mcts(20);
}