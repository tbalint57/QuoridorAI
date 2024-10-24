#include <stdint.h>
#include <algorithm>
#include "board.cpp"

using namespace std;

float minimax(Board board, int depth, bool player){
    if (depth = 0){
        return board.evaluate();
    }

    if(player){
        float maxValue = FLT_MIN;
        for (uint8_t move : board.generatePossibleMoves(player)){
            Board newBoard = board;
            newBoard.executeMove(move, player);
            maxValue = max(maxValue, minimax(newBoard, depth - 1, !player));
        }
        return maxValue;
    }

    if(!player){
        float minValue = FLT_MAX;
        for (uint8_t move : board.generatePossibleMoves(player)){
            Board newBoard = board;
            newBoard.executeMove(move, player);
            minValue = min(minValue, minimax(newBoard, depth - 1, !player));
        }
        return minValue;
    }
}