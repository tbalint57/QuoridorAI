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

int ROLLOUTS = 100;
int SIMULATIONS_PER_ROLLOUT = 3;
int ROLLOUT_PARAMETER = 4;
float MCTS_PARAMETER = 0.5;


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


void createDataSet(Board* board, string saveFileName, int* branchings, bool player, unordered_set<Board, BoardHasher>* seenBoards){
    if(board->getWinner()){
        return;
    }

    if (seenBoards->find(*board) != seenBoards->end()) {
        return;
    }

    int distribution[256] = {0};
    mctsDistribution(*board, ROLLOUTS, SIMULATIONS_PER_ROLLOUT, player, distribution, ROLLOUT_PARAMETER, MCTS_PARAMETER);

    seenBoards->insert(*board);
    saveBoardPosition(board, saveFileName, distribution);

    for(int i = 1; i <= branchings[0]; i++){
        Board boardCopy = Board(*board);
        boardCopy.executeMove(nthBestMove(distribution, i), player);
        createDataSet(&boardCopy, saveFileName, branchings + 1, !player, seenBoards);
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

    // Delete the save file if it exists
    remove(saveFileName.c_str());

    // Create and save data
    int branchings[36] = {2, 2, 2, 2, 1, 1, 2, 1,
                          1, 2, 1, 1, 2, 1, 1, 2,
                          1, 1, 1, 1, 1, 1, 1, 1,
                          1, 1, 1, 1, 1, 1, 1, 1,
                          1, 1, 1, 0};  // 36 total

    std::unordered_set<Board, BoardHasher> seenBoards;


    // Starting State
    Board board = Board();
    createDataSet(&board, saveFileName, branchings, true, &seenBoards);
    cout << "State #" << 1 << "/8\n";
    
    // The Sidewall Opening 
    board = Board();
    board.executeMove(24, true);
    board.executeMove(16, false);
    board.executeMove(179, true);
    createDataSet(&board, saveFileName, branchings, false, &seenBoards);
    cout << "State #" << 2 << "/8\n";
    
    // The Rush Opening 
    board = Board();
    board.executeMove(24, true);
    board.executeMove(16, false);
    board.executeMove(24, true);
    board.executeMove(16, false);
    board.executeMove(24, true);
    board.executeMove(16, false);
    board.executeMove(163, true);
    createDataSet(&board, saveFileName, branchings, false, &seenBoards);
    cout << "State #" << 3 << "/8\n";

    // The Reed Opening 
    board = Board();
    board.executeMove(0b11101010, true);
    board.executeMove(16, false);
    board.executeMove(0b11101101, true);
    board.executeMove(16, false);
    createDataSet(&board, saveFileName, branchings, true, &seenBoards);
    cout << "State #" << 4 << "/8\n";
    
    // The Shatranj Opening 
    board = Board();
    board.executeMove(0b10000011, true);
    createDataSet(&board, saveFileName, branchings, false, &seenBoards);
    cout << "State #" << 5 << "/8\n";
    
    // The Standard Opening
    board = Board();
    board.executeMove(24, true);
    board.executeMove(16, false);
    board.executeMove(24, true);
    board.executeMove(16, false);
    board.executeMove(24, true);
    board.executeMove(16, false);
    board.executeMove(0b11010100, true);
    createDataSet(&board, saveFileName, branchings, false, &seenBoards);
    cout << "State #" << 6 << "/8\n";
    
    // The Shiller Opening
    board = Board();
    board.executeMove(24, true);
    board.executeMove(16, false);
    board.executeMove(24, true);
    board.executeMove(16, false);
    board.executeMove(24, true);
    board.executeMove(16, false);
    board.executeMove(0b10000100, true);
    createDataSet(&board, saveFileName, branchings, false, &seenBoards);
    cout << "State #" << 7 << "/8\n";
    
    // The Quick Box Opening
    board = Board();
    board.executeMove(24, true);
    board.executeMove(0b10001100, false);
    createDataSet(&board, saveFileName, branchings, true, &seenBoards);
    cout << "State #" << 8 << "/8\n";

     // Step 3: Read from the save file
    const size_t maxBoards = 100000; // adjust depending on how big the data can get
    Board* boards = new Board[maxBoards];
    int (*distributions)[256] = new int[maxBoards][256];
    size_t size = 0;

    readInSaveFile(boards, distributions, size, saveFileName);

    // Step 4: Print out the results
    std::cout << "Read " << size << " board positions.\n";
}