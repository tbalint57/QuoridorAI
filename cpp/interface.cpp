#include <iostream>
#include <stdint.h>
#include <vector>
#include <cstring>
#include <cstdlib>
#include "mcts.cpp"
using namespace std;

// For future reference, this is how to compile:
// g++ -shared -Wl,-soname,interface -o cpp/interface.so -fPIC cpp/interface.cpp -Ilibs/eigen-3.4.0


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


    uint8_t calculateBestMove(Cell whitePawn, Cell blackPawn, Wall* walls, size_t length, int whiteWalls, int blackWalls, bool player, int rollouts){
        vector<pair<bool, pair<int, int>>> wallsVector = {};
        for (size_t i = 0; i < length; i++){
            Wall wall = walls[i];
            wallsVector.push_back({wall.isHorizontal, {wall.i, wall.j}});
        }

        Board game = Board({whitePawn.i, whitePawn.j}, {blackPawn.i, blackPawn.j}, wallsVector, whiteWalls, blackWalls);
        MCTS agent = MCTS(rollouts, 5, 0.5, 4, "cpp/GPmodels", true, 0);
        return agent.predictBestMove(game, player);
    }


    uint8_t* getPossibleMoves(Cell whitePawn, Cell blackPawn, Wall* walls, size_t length, int whiteWalls, int blackWalls, bool player, size_t* outputLength){
        vector<pair<bool, pair<int, int>>> wallsVector = {};
        for (size_t i = 0; i < length; i++){
            Wall wall = walls[i];
            wallsVector.push_back({wall.isHorizontal, {wall.i, wall.j}});
        }

        Board game = Board({whitePawn.i, whitePawn.j}, {blackPawn.i, blackPawn.j}, wallsVector, whiteWalls, blackWalls);
        uint8_t possibleMoves[256];
        size_t moveCount = 0;
        game.generatePossibleMoves(player, possibleMoves, moveCount);
        
        *outputLength = moveCount;

        uint8_t* output = (uint8_t*)malloc(*outputLength * sizeof(uint8_t));
        memcpy(output, possibleMoves, *outputLength);

        return output;
    }


    void freeMemory(uint8_t* ptr) {
        free(ptr);
    }
}


