// Life lesson of this file: One shall use a scripting language when doing...
#include <iostream>
#include <stdint.h>
#include "mcts.cpp"
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>
#include <unordered_set>
#include <cstdio>
#include <random>
#include <ctime>
#include <array>

using namespace std;

int ROLLOUTS = 10000;
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


void generateData(Board* board, string saveFileNameWhite, string saveFileNameBlack, int* branchings, bool player, unordered_set<Board, BoardHasher>* seenBoards){
    if(board->getWinner()){
        return;
    }

    if (seenBoards->find(*board) != seenBoards->end()) {
        return;
    }

    int distribution[256] = {0};
    mctsDistribution(*board, ROLLOUTS, SIMULATIONS_PER_ROLLOUT, player, distribution, ROLLOUT_PARAMETER, MCTS_PARAMETER);

    seenBoards->insert(*board);
    string saveFileName = player ? saveFileNameWhite : saveFileNameBlack;
    saveBoardPosition(board, saveFileName, distribution);

    for(int i = 1; i <= branchings[0]; i++){
        Board boardCopy = Board(*board);
        boardCopy.executeMove(nthBestMove(distribution, i), player);
        generateData(&boardCopy, saveFileNameWhite, saveFileNameBlack, branchings + 1, !player, seenBoards);
    }
}


Board createRandomPosition(uint8_t whiteWalls, uint8_t blackWalls){
    srand(time(NULL));
    Board board = Board();

    // Need to set everything to public!
    uint8_t whiteRow = rand() % 8;
    uint8_t whiteCol = rand() % 9;
    uint8_t blackRow = rand() % 8 + 1;
    uint8_t blackCol = rand() % 9;
    while(blackRow == whiteRow && whiteCol == blackCol){
        blackRow = rand() % 9;
        blackCol = rand() % 9;
    }

    board.whitePawn = whiteRow * 16 + whiteCol;
    board.blackPawn = blackRow * 16 + blackCol;

    for(int i = 0; i < 10 - whiteWalls; i++){
        uint8_t iWall = rand() % 8;
        uint8_t jWall = rand() % 8;
        bool horizontal = rand() % 2;
        while(!board.placeWall(iWall, jWall, horizontal, true)){
            iWall = rand() % 8;
            jWall = rand() % 8;
            horizontal = rand() % 2;
        }
    }

    for(int i = 0; i < 10 - blackWalls; i++){
        uint8_t iWall = rand() % 8;
        uint8_t jWall = rand() % 8;
        bool horizontal = rand() % 2;
        while(!board.placeWall(iWall, jWall, horizontal, false)){
            iWall = rand() % 8;
            jWall = rand() % 8;
            horizontal = rand() % 2;
        }
    }

    return board;
}


void createDataSetNatural() {
    const string saveFileNameWhite = "datasets/datasetWhite";
    const string saveFileNameBlack = "datasets/datasetBlack";

    // Delete the save file if it exists
    remove(saveFileNameWhite.c_str());
    remove(saveFileNameBlack.c_str());

    int branchings[40] = {2, 2, 2, 2, 1, 1, 2, 1,
                          1, 2, 1, 1, 2, 1, 1, 2,
                          1, 1, 1, 1, 1, 1, 1, 1,
                          1, 1, 1, 1, 1, 1, 1, 1,
                          1, 1, 1, 1, 1, 1, 1, 0};
    unordered_set<Board, BoardHasher> seenBoards;


    // Starting State
    Board board = Board();
    generateData(&board, saveFileNameWhite, saveFileNameBlack, branchings, true, &seenBoards);
    cout << "State #" << 1 << "/8\n";
    
    // The Sidewall Opening 
    board = Board();
    board.executeMove(24, true);
    board.executeMove(16, false);
    board.executeMove(179, true);
    generateData(&board, saveFileNameWhite, saveFileNameBlack, branchings, true, &seenBoards);
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
    generateData(&board, saveFileNameWhite, saveFileNameBlack, branchings, true, &seenBoards);
    cout << "State #" << 3 << "/8\n";

    // The Reed Opening 
    board = Board();
    board.executeMove(0b11101010, true);
    board.executeMove(16, false);
    board.executeMove(0b11101101, true);
    board.executeMove(16, false);
    generateData(&board, saveFileNameWhite, saveFileNameBlack, branchings, true, &seenBoards);
    cout << "State #" << 4 << "/8\n";
    
    // The Shatranj Opening 
    board = Board();
    board.executeMove(0b10000011, true);
    generateData(&board, saveFileNameWhite, saveFileNameBlack, branchings, true, &seenBoards);
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
    generateData(&board, saveFileNameWhite, saveFileNameBlack, branchings, true, &seenBoards);
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
    generateData(&board, saveFileNameWhite, saveFileNameBlack, branchings, true, &seenBoards);
    cout << "State #" << 7 << "/8\n";
    
    // The Quick Box Opening
    board = Board();
    board.executeMove(24, true);
    board.executeMove(0b10001100, false);
    generateData(&board, saveFileNameWhite, saveFileNameBlack, branchings, true, &seenBoards);
    cout << "State #" << 8 << "/8\n";

    // Confirm number of datapoints generated
    const size_t maxBoards = 100000;
    Board* boards = new Board[maxBoards];
    int (*distributions)[256] = new int[maxBoards][256];
    size_t size = 0;

    readInSaveFile(boards, distributions, size, saveFileNameWhite);
    cout << "Generated  " << size << " white board positions successfully.\n";

    size = 0;
    readInSaveFile(boards, distributions, size, saveFileNameBlack);
    cout << "Generated  " << size << " black board positions successfully.\n";
}


void padDataSetArtificially(bool player, string saveFileName) {
    unordered_set<Board, BoardHasher> seenBoards;

    const size_t maxBoards = 100000;
    Board* boards = new Board[maxBoards];
    int (*distributions)[256] = new int[maxBoards][256];
    size_t size = 0;

    readInSaveFile(boards, distributions, size, saveFileName);

    int positions[21] = {0};

    for(int i = 0; i < size; i++){
        seenBoards.insert(boards[i]);
        positions[20 - boards[i].whiteWalls - boards[i].blackWalls]++;
    }
    

    for(int i = 0; i < 21; i++){
        while(positions[i] < 1100){
            int whiteWalls = 0;
            int blackWalls = 0;

            for(int j = 0; j < 20 - i; j++){
                if(rand() % 2){
                    whiteWalls++;
                }
                else{
                    blackWalls++;
                }
            }
            
            if(whiteWalls > 10){
                blackWalls += whiteWalls - 10;
                whiteWalls = 10;
            }
            
            if(blackWalls > 10){
                whiteWalls += blackWalls - 10;
                blackWalls = 10;
            }

            Board board = createRandomPosition(whiteWalls, blackWalls);
            while(seenBoards.find(board) != seenBoards.end()){
                board = createRandomPosition(whiteWalls, blackWalls);
            }
            seenBoards.insert(board);

            int distributionW[256] = {0};
            mctsDistribution(board, ROLLOUTS, SIMULATIONS_PER_ROLLOUT, player, distributionW, ROLLOUT_PARAMETER, MCTS_PARAMETER);
            saveBoardPosition(&board, saveFileName, distributionW);
            positions[i]++;
        }
    }
}


void segmentDataset(string saveFileName, int trainLength, int validationLength) {
    const size_t maxBoards = 100000;
    Board* boards = new Board[maxBoards];
    int (*distributions)[256] = new int[maxBoards][256];
    size_t size = 0;

    readInSaveFile(boards, distributions, size, saveFileName);

    int sizes[21] = {0};
    vector<pair<Board, array<int, 256>>> byWalls[21];

    for (int i = 0; i < size; i++) {
        int wallsUsed = 20 - boards[i].whiteWalls - boards[i].blackWalls;
        array<int, 256> distCopy;
        copy(distributions[i], distributions[i] + 256, distCopy.begin());
        byWalls[wallsUsed].emplace_back(boards[i], distCopy);
    }

    random_device rd;
    mt19937 gen(rd());

    for (int w = 0; w <= 20; w++) {
        vector<pair<Board, array<int, 256>>>& data = byWalls[w];

        shuffle(data.begin(), data.end(), gen);

        string trainFile = saveFileName + to_string(w) + ".train";
        string valFile = saveFileName + to_string(w) + ".val";

        ofstream trainOut(trainFile, ios::binary);
        ofstream valOut(valFile, ios::binary);

        for (int i = 0; i < trainLength; i++) {
            uint8_t saveData[32];
            data[i].first.getSaveData(saveData);
            size_t dataSize = saveData[0];
            trainOut.write((char*)saveData, dataSize);
            trainOut.write((char*)data[i].second.data(), 256 * sizeof(int));
        }

        for (int i = trainLength; i < trainLength + validationLength; i++) {
            uint8_t saveData[32];
            data[i].first.getSaveData(saveData);
            size_t dataSize = saveData[0];
            valOut.write((char*)saveData, dataSize);
            valOut.write((char*)data[i].second.data(), 256 * sizeof(int));
        }

        trainOut.close();
        valOut.close();
    }

    delete[] boards;
    delete[] distributions;
}


// int main() {
//     srand(time(NULL));
//     createDataSetNatural();

//     padDataSetArtificially(true, "datasets/datasetWhite");
//     padDataSetArtificially(false, "datasets/datasetBlack");

//     segmentDataset("datasets/datasetWhite", 1000, 100);
//     segmentDataset("datasets/datasetBlack", 1000, 100);
// }