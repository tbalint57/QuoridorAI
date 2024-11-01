#include <stdint.h>
#include <algorithm>
#include <float.h>
#include "board.cpp"

using namespace std;

float minimax(Board* board, int depth, bool player){

    if (depth == 0){
        return board->evaluate();
    }

    if(player){
        float maxValue = -1000.0f;
        for (uint8_t move : board->generatePossibleMoves(player)){
            board->executeMove(move, player);
            maxValue = max(maxValue, minimax(board, depth - 1, !player));
            board->undoMove(move, player);
        }
        return maxValue;
    }

    if(!player){
        float minValue = 1000.0f;
        for (uint8_t move : board->generatePossibleMoves(player)){
            board->executeMove(move, player);
            minValue = min(minValue, minimax(board, depth - 1, !player));
            board->undoMove(move, player);
        }
        return minValue;
    }

    return 0;
}

uint8_t calculateBestMove(Board* board, int depth, bool player){
    uint8_t bestMove;

    if (depth == 0){
        return board->evaluate();
    }

    if(player){
        float maxValue = -1000.0f;

        for (uint8_t move : board->generatePossibleMoves(player)){
            board->executeMove(move, player);
            float value = minimax(board, depth - 1, !player);
            if(value > maxValue){
                bestMove = move;
                maxValue = value;
            }
            board->undoMove(move, player);
        }
    }

    if(!player){
        float minValue = 1000.0f;

        for (uint8_t move : board->generatePossibleMoves(player)){
            board->executeMove(move, player);
            float value = minimax(board, depth - 1, !player);
            if(value < minValue){
                bestMove = move;
                minValue = value;
            }
            board->undoMove(move, player);
        }
    }

    
    return bestMove;
}