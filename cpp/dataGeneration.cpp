#include <iostream>
#include <stdint.h>
#include "mcts.cpp"
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>
#include <unordered_set>
#include <cstdio>
using namespace std;

int ROLLOUTS = 10000;
int SIMULATIONS_PER_ROLLOUT = 3;


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


void saveBoardPosition(Board* board, string saveFileName, int* distribution){
    uint8_t saveData[32];
    board->getSaveData(saveData);
    size_t size = saveData[0];

    ofstream file(saveFileName, ios::out | ios::binary | ios::app);
    file.write((char*)saveData, size);
    file.write((char*)distribution, 256 * sizeof(int));
    file.close();
}


void createDataSet(Board* board, string saveFileName, int* branchings, int numberOfMoves, bool player, unordered_set<Board, BoardHasher>* seenBoards){
    if(board->getWinner()){
        return;
    }

    if (seenBoards->find(*board) != seenBoards->end()) {
        return;
    }

    int distribution[256] = {0};
    mctsDistribution(*board, ROLLOUTS, SIMULATIONS_PER_ROLLOUT, player, distribution);

    seenBoards->insert(*board);
    saveBoardPosition(board, saveFileName, distribution);

    for(int i = 1; i <= branchings[0]; i++){
        Board boardCopy = Board(*board);
        boardCopy.executeMove(nthBestMove(distribution, i), player);
        createDataSet(&boardCopy, saveFileName, branchings + 1, numberOfMoves - 1, !player, seenBoards);
    }
}


void readInSaveFile(Board* boards, int distributions[][256], size_t& size, string saveFileName){
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

        file.read((char*)distributions[size], 256 * sizeof(int));

        size++;
    }

    file.close();
}


int main(int argc, char const* argv[]) {
    const std::string saveFileName = "bigData";

    // Step 1: Delete the save file if it exists
    remove(saveFileName.c_str());

    // Step 2: Create and save data
    Board board = Board();
    int branchings[36] = {1, 1, 1, 1, 1, 1, 1, 1,
                          1, 1, 1, 1, 1, 1, 1, 1,
                          1, 1, 1, 1, 1, 1, 1, 1,
                          1, 1, 1, 1, 1, 1, 1, 1,
                          1, 1, 1, 0};  // 36 total

    std::unordered_set<Board, BoardHasher> seenBoards;
    createDataSet(&board, saveFileName, branchings, 36, true, &seenBoards);

    // Step 3: Read from the save file
    const size_t maxBoards = 10000; // adjust depending on how big the data can get
    Board* boards = new Board[maxBoards];
    int (*distributions)[256] = new int[maxBoards][256];
    size_t size = 0;

    readInSaveFile(boards, distributions, size, saveFileName);

    // Step 4: Print out the results
    std::cout << "Read " << size << " board positions:\n";
    for (size_t i = 0; i < size; ++i) {
        std::cout << "Board #" << i << ":\n";
        boards[i].printState(); // assuming your Board class has a printState() function

        int sum = 0;
        for (int j = 0; j < 256; ++j) {
            sum += distributions[i][j];
            if (distributions[i][j] > ROLLOUTS * SIMULATIONS_PER_ROLLOUT / 50) {
                std::cout << "  Move " << j << ": " << distributions[i][j] << "\n";
            }
        }
        std::cout << "Distribution: " << sum;
        std::cout << "---------------------------\n";
    }

    // Clean up
    delete[] boards;
    delete[] distributions;

    return 0;
}