#include <iostream>
#include <stdint.h>
#include <vector>
#include <cstring>
#include <cstdlib>
#include "minimax.cpp"
using namespace std;


extern "C" {
    struct Cell {
        int i;
        int j;
    };


    struct Wall {
        bool isHorizontal;
        int i;
        int j;
    };


    uint8_t calculateBestMove(Cell whitePawn, Cell blackPawn, Wall* walls, size_t length, int whiteWalls, int blackWalls, bool player, int depth){
        vector<pair<bool, pair<int, int>>> wallsVector = {};
        for (size_t i = 0; i < length; i++){
            Wall wall = walls[i];
            wallsVector.push_back({wall.isHorizontal, {wall.i, wall.j}});
        }

        Board game = Board({whitePawn.i, whitePawn.j}, {blackPawn.i, blackPawn.j}, wallsVector, whiteWalls, blackWalls);
        return calculateBestMove(&game, depth, player);
    }


    uint8_t* getPossibleMoves(Cell whitePawn, Cell blackPawn, Wall* walls, size_t length, int whiteWalls, int blackWalls, bool player, size_t* outputLength){
        vector<pair<bool, pair<int, int>>> wallsVector = {};
        for (size_t i = 0; i < length; i++){
            Wall wall = walls[i];
            wallsVector.push_back({wall.isHorizontal, {wall.i, wall.j}});
        }

        Board game = Board({whitePawn.i, whitePawn.j}, {blackPawn.i, blackPawn.j}, wallsVector, whiteWalls, blackWalls);
        vector<uint8_t> possibleMoves = game.generatePossibleMoves(player);
        
        *outputLength = possibleMoves.size();

        uint8_t* output = (uint8_t*)malloc(*outputLength * sizeof(uint8_t));
        memcpy(output, possibleMoves.data(), *outputLength);

        return output;
    }


    void freeMemory(uint8_t* ptr) {
        free(ptr);
    }
}


