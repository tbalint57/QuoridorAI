#include <iostream>
#include <stdint.h>
#include "dataGeneration.cpp"
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>
#include <unordered_set>
#include <cstdio>
#include <random>
#include <ctime>
#include <array>


bool simulateGame(Board board, int rolloutPolicyParameter1, float mctsParameter1, int rolloutPolicyParameter2, float mctsParameter2, bool whiteMove){
    MCTS agent1 = MCTS(ROLLOUTS, SIMULATIONS_PER_ROLLOUT, mctsParameter1, rolloutPolicyParameter1);
    MCTS agent2 = MCTS(ROLLOUTS, SIMULATIONS_PER_ROLLOUT, mctsParameter2, rolloutPolicyParameter2);

    for (int i = 0; i < 40 && !board.getWinner(); i++){
        MCTS agent = whiteMove ? agent1 : agent2;

        uint8_t bestMove = agent.predictBestMove(board, whiteMove);
        board.executeMove(bestMove, whiteMove);

        whiteMove = !whiteMove;
    }

    if(board.getWinner() == 'w'){
        return true;
    }

    if(board.getWinner() == 'b'){
        return false;
    }

    return board.whiteCloser(whiteMove);
}


int contestParameters(int rolloutPolicyParameter1, float mctsParameter1, int rolloutPolicyParameter2, float mctsParameter2){
    int difference = 0;
    bool results[16];

    // Starting State
    Board board = Board();
    results[0] = simulateGame(board, rolloutPolicyParameter1, mctsParameter1, rolloutPolicyParameter2, mctsParameter2, true);
    results[1] = !simulateGame(board, rolloutPolicyParameter2, mctsParameter2, rolloutPolicyParameter1, mctsParameter1, true);
    
    // The Sidewall Opening 
    board = Board();
    board.executeMove(24, true);
    board.executeMove(16, false);
    board.executeMove(179, true);
    results[2] = simulateGame(board, rolloutPolicyParameter1, mctsParameter1, rolloutPolicyParameter2, mctsParameter2, false);
    results[3] = !simulateGame(board, rolloutPolicyParameter2, mctsParameter2, rolloutPolicyParameter1, mctsParameter1, false);
    
    // The Rush Opening 
    board = Board();
    board.executeMove(24, true);
    board.executeMove(16, false);
    board.executeMove(24, true);
    board.executeMove(16, false);
    board.executeMove(24, true);
    board.executeMove(16, false);
    board.executeMove(163, true);
    results[4] = simulateGame(board, rolloutPolicyParameter1, mctsParameter1, rolloutPolicyParameter2, mctsParameter2, false);
    results[5] = !simulateGame(board, rolloutPolicyParameter2, mctsParameter2, rolloutPolicyParameter1, mctsParameter1, false);

    // The Reed Opening 
    board = Board();
    board.executeMove(0b11101010, true);
    board.executeMove(16, false);
    board.executeMove(0b11101101, true);
    board.executeMove(16, false);
    results[6] = simulateGame(board, rolloutPolicyParameter1, mctsParameter1, rolloutPolicyParameter2, mctsParameter2, true);
    results[7] = !simulateGame(board, rolloutPolicyParameter2, mctsParameter2, rolloutPolicyParameter1, mctsParameter1, true);
    
    // The Shatranj Opening 
    board = Board();
    board.executeMove(0b10000011, true);
    results[8] = simulateGame(board, rolloutPolicyParameter1, mctsParameter1, rolloutPolicyParameter2, mctsParameter2, false);
    results[9] = !simulateGame(board, rolloutPolicyParameter2, mctsParameter2, rolloutPolicyParameter1, mctsParameter1, false);
    
    // The Standard Opening
    board = Board();
    board.executeMove(24, true);
    board.executeMove(16, false);
    board.executeMove(24, true);
    board.executeMove(16, false);
    board.executeMove(24, true);
    board.executeMove(16, false);
    board.executeMove(0b11010100, true);
    results[10] = simulateGame(board, rolloutPolicyParameter1, mctsParameter1, rolloutPolicyParameter2, mctsParameter2, false);
    results[11] = !simulateGame(board, rolloutPolicyParameter2, mctsParameter2, rolloutPolicyParameter1, mctsParameter1, false);
    
    // The Shiller Opening
    board = Board();
    board.executeMove(24, true);
    board.executeMove(16, false);
    board.executeMove(24, true);
    board.executeMove(16, false);
    board.executeMove(24, true);
    board.executeMove(16, false);
    board.executeMove(0b10000100, true);
    results[12] = simulateGame(board, rolloutPolicyParameter1, mctsParameter1, rolloutPolicyParameter2, mctsParameter2, false);
    results[13] = !simulateGame(board, rolloutPolicyParameter2, mctsParameter2, rolloutPolicyParameter1, mctsParameter1, false);
    

    // The Quick Box Opening
    board = Board();
    board.executeMove(24, true);
    board.executeMove(0b10001100, false);
    results[14] = simulateGame(board, rolloutPolicyParameter1, mctsParameter1, rolloutPolicyParameter2, mctsParameter2, true);
    results[15] = !simulateGame(board, rolloutPolicyParameter2, mctsParameter2, rolloutPolicyParameter1, mctsParameter1, true);
    
    for(bool result : results){
        result ? difference++ : difference--;
    }

    return difference;
}



void mctsHyperParameterSearch(int& rolloutPolicyParameter, float& mctsParameter) {
    // Step 1: Initialize 64 random parameter pairs
    vector<pair<int, float>> population;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> rpp_dist(3, 10);
    uniform_real_distribution<float> mctsp_dist(0.1f, 2.0f);

    while (population.size() < 64) {
        int rpp = rpp_dist(gen);
        float mctsp = mctsp_dist(gen);
        population.emplace_back(rpp, mctsp);
    }

    // Step 2: Tournament rounds (single elimination)
    while (population.size() > 1) {
        vector<pair<int, float>> winners;
        shuffle(population.begin(), population.end(), gen);

        for (size_t i = 0; i < population.size(); i += 2) {
            auto [rpp1, mctsp1] = population[i];
            auto [rpp2, mctsp2] = population[i + 1];

            int result = contestParameters(rpp1, mctsp1, rpp2, mctsp2);

            if (result > 0) {
                winners.emplace_back(rpp1, mctsp1);
            } else {
                winners.emplace_back(rpp2, mctsp2);
            }
        }

        population = winners;
    }

    // Step 3: Return final winner
    rolloutPolicyParameter = population[0].first;
    mctsParameter = population[0].second;
}


void createDataset() {
    // Only use before first iteration, do not recreate the dataset.
    srand(time(NULL));
    createDataSetNatural();

    padDataSetArtificially(true, "datasets/datasetWhite");
    padDataSetArtificially(false, "datasets/datasetBlack");

    segmentDataset("datasets/datasetWhite", 1000, 100);
    segmentDataset("datasets/datasetBlack", 1000, 100);
}


void relabelDatasetFile(const string& filename, bool player) {
    const size_t maxBoards = 2000;
    Board* boards = new Board[maxBoards];
    int (*distributions)[256] = new int[maxBoards][256];
    size_t size = 0;

    MCTS agent = MCTS(ROLLOUTS, SIMULATIONS_PER_ROLLOUT, MCTS_PARAMETER, ROLLOUT_PARAMETER);

    readInSaveFile(boards, distributions, size, filename);

    ofstream out(filename, ios::binary | ios::trunc); // overwrite file

    for (size_t i = 0; i < size; i++) {
        int newDistribution[256] = {0};
        agent.predictDistribution(boards[i], player, newDistribution);

        uint8_t saveData[32];
        boards[i].getSaveData(saveData);
        size_t dataSize = saveData[0];

        out.write((char*)saveData, dataSize);
        out.write((char*)newDistribution, 256 * sizeof(int));
    }

    out.close();
    delete[] boards;
    delete[] distributions;
}


void relabelDatasets() {
    for (int w = 0; w <= 20; w++) {
        relabelDatasetFile("datasets/datasetWhite" + to_string(w) + ".train", true);
        relabelDatasetFile("datasets/datasetWhite" + to_string(w) + ".val", true);

        relabelDatasetFile("datasets/datasetBlack" + to_string(w) + ".train", false);
        relabelDatasetFile("datasets/datasetBlack" + to_string(w) + ".val", false);
    }
}


int main(int argc, char const* argv[]) {
    ROLLOUTS = 10000;
    SIMULATIONS_PER_ROLLOUT = 3;

    using namespace std::chrono;

    auto start = high_resolution_clock::now();
    auto end = high_resolution_clock::now();
    duration<double> elapsed = end - start;

    mctsHyperParameterSearch(ROLLOUT_PARAMETER, MCTS_PARAMETER);
    end = high_resolution_clock::now();
    elapsed = end - start;
    std::cout << "MCTS hyperparameter search finished, execution time: " << elapsed.count() / 3600 << " hours\n";
    cout << "Best params: " << ROLLOUT_PARAMETER << " " << MCTS_PARAMETER << endl;

    relabelDatasets();
    end = high_resolution_clock::now();
    elapsed = end - start;
    std::cout << "Relabeling dataset finished, execution time: " << elapsed.count() / 3600 << " hours\n";

    pre_train_models();
    end = high_resolution_clock::now();
    elapsed = end - start;
    std::cout << "Training finished, execution time: " << elapsed.count() / 3600 << " hours\n";

}


