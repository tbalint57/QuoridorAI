#include <stdint.h>
#include <algorithm>
#include <float.h>
#include "board.cpp"

using namespace std;

float minimax(Board* board, int depth, bool player, float alpha, float beta){

    if (depth == 0){
        return board->evaluate();
    }

    if(player){
        float maxValue = -1000.0f;
        for (uint8_t move : board->generatePossibleMoves(player)){
            board->executeMove(move, player);
            maxValue = max(maxValue, minimax(board, depth - 1, !player, alpha, beta));
            if (maxValue > beta){
                break;
            }
            board->undoMove(move, player);
            alpha = max(alpha, maxValue);
        }
        return maxValue;
    }

    if(!player){
        float minValue = 1000.0f;
        for (uint8_t move : board->generatePossibleMoves(player)){
            board->executeMove(move, player);
            minValue = min(minValue, minimax(board, depth - 1, !player, alpha, beta));
            if(minValue < alpha){
                break;
            }
            board->undoMove(move, player);
            beta = min(beta, minValue);
        }
        return minValue;
    }

    return 0;
}

uint8_t calculateBestMove(Board* board, int depth, bool player){
    uint8_t bestMove;

    float alpha = -2000.0f;
    float beta = 2000.0f;

    if (depth == 0){
        return board->evaluate();
    }

    if(player){
        float maxValue = -2000.0f;

        for (uint8_t move : board->generatePossibleMoves(player)){
            board->executeMove(move, player);
            float value = minimax(board, depth - 1, !player, alpha, beta);
            if(value > maxValue){
                bestMove = move;
                maxValue = value;
            }
            if (maxValue > beta){
                break;
            }
            board->undoMove(move, player);
            alpha = max(alpha, maxValue);
        }
    }

    if(!player){
        float minValue = 2000.0f;

        for (uint8_t move : board->generatePossibleMoves(player)){
            board->executeMove(move, player);
            float value = minimax(board, depth - 1, !player, alpha, beta);
            if(value < minValue){
                bestMove = move;
                minValue = value;
            }
            if(minValue < alpha){
                break;
            }
            board->undoMove(move, player);
            beta = min(beta, minValue);
        }
    }

    
    return bestMove;
}