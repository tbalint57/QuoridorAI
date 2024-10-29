#include <stdint.h>
#include <algorithm>
#include "board.cpp"

using namespace std;

float minimax(Board* board_ptr, int depth, bool player){
    Board board = *board_ptr;

    if (depth = 0){
        return board.evaluate();
    }

    if(player){
        float maxValue = FLT_MIN;
        for (uint8_t move : board.generatePossibleMoves(player)){
            board.executeMove(move, player);
            maxValue = max(maxValue, minimax(&board, depth - 1, !player));
            board.undoMove(move, player);
        }
        return maxValue;
    }

    if(!player){
        float minValue = FLT_MAX;
        for (uint8_t move : board.generatePossibleMoves(player)){
            board.executeMove(move, player);
            minValue = min(minValue, minimax(&board, depth - 1, !player));
            board.undoMove(move, player);
        }
        return minValue;
    }
}