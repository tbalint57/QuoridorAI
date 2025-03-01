#include <iostream>
#include <stdint.h>
#include "mcts.cpp"
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>
using namespace std;

int nthBestMove(int* distribution, int n) {
    vector<int> moves(256);
    for (size_t i = 0; i < 256; ++i) {
        moves[i] = i;
    }
    
    // Sort moves based on distribution
    sort(moves.begin(), moves.end(), [&](int a, int b) {
        return distribution[a] > distribution[b];
    });
    
    return moves[n - 1];
}


void saveBoardPosition(Board* board, string saveFileName){
    uint8_t saveData[32];
    board->getSaveData(saveData);
    size_t size = saveData[0];
    cout << size << "\n";

    ofstream file(saveFileName, ios::out | ios::binary | ios::app);
    file.write((char*)saveData, size);
    file.close();
}


void createDataSet(Board* board, string saveFileName, int* branchings, int numberOfMoves, bool player){
    if(board->getWinner()){
        return;
    }

    saveBoardPosition(board, saveFileName);

    for(int i = 1; i <= branchings[0]; i++){
        Board boardCopy = Board(*board);
        int distribution[256];
        mctsDistribution(boardCopy, 10000, 10, player, distribution);
        boardCopy.executeMove(nthBestMove(distribution, i), player);
        createDataSet(&boardCopy, saveFileName, branchings + 1, numberOfMoves - 1, !player);
    }
}


void readInSaveFile(Board* boards, size_t& size, string saveFileName){
    std::ifstream file(saveFileName, std::ios::in | std::ios::binary);

    char ch;
    while (file.get(ch)) {
        size_t board_size = ch - 1;
        char whitePawn, blackPawn, whiteWalls, blackWalls;

        file.get(whitePawn);
        file.get(blackPawn);
        file.get(whiteWalls);
        file.get(blackWalls);

        char WallsOnBoard[20];
        size_t numberOfWallsOnBoard = board_size - 4;

        for(int i = 0; i < numberOfWallsOnBoard; i++){
            file.get(WallsOnBoard[i]);
        }
        boards[size] = Board(whitePawn, blackPawn, whiteWalls, blackWalls, WallsOnBoard, numberOfWallsOnBoard);
        size++;
    }

    std::cout << std::endl;
    file.close();
}


int main(int argc, char const* argv[]) {
    Board board = Board();
    int branchings[36] = {1, 1, 1, 1, 2, 2, 1, 1,
                          1, 2, 2, 1, 1, 1, 1, 1,
                          1, 2, 1, 1, 1, 1, 1, 1,
                          2, 1, 1, 1, 1, 1, 1, 1};

    
    createDataSet(&board, "bigData", branchings, 36, true);
}