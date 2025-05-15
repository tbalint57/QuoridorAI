#include <iostream>
#include <stdint.h>
#include "dataGeneration.cpp"
#include <string>
#include <fstream>
#include <cstdio>

using namespace std::chrono;


/**
 * Simulate game between two agents
 */
bool simulateGame(Board board, MCTS agent1, MCTS agent2, bool whiteMove){
    for (int i = 0; i < 200 && !board.getWinner(); i++){
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

    cout << "Simulation not finished" << endl;

    return board.whiteCloser(whiteMove);
}


/**
 * Contest two agents
 */
int contestModels(MCTS agent1, MCTS agent2){
    bool results[16];

    // Starting State
    Board board = Board();
    results[0] = simulateGame(board, agent1, agent2, true);
    results[1] = !simulateGame(board, agent2, agent1, true);
    cout << "1/8" << endl;
    
    // The Sidewall Opening 
    board = Board();
    board.executeMove(24, true);
    board.executeMove(16, false);
    board.executeMove(179, true);
    results[2] = simulateGame(board, agent1, agent2, false);
    results[3] = !simulateGame(board, agent2, agent1, false);
    cout << "2/8" << endl;
    
    // The Rush Opening 
    board = Board();
    board.executeMove(24, true);
    board.executeMove(16, false);
    board.executeMove(24, true);
    board.executeMove(16, false);
    board.executeMove(24, true);
    board.executeMove(16, false);
    board.executeMove(163, true);
    results[4] = simulateGame(board, agent1, agent2, false);
    results[5] = !simulateGame(board, agent2, agent1, false);
    cout << "3/8" << endl;

    // The Reed Opening 
    board = Board();
    board.executeMove(0b11101010, true);
    board.executeMove(16, false);
    board.executeMove(0b11101101, true);
    board.executeMove(16, false);
    results[6] = simulateGame(board, agent1, agent2, true);
    results[7] = !simulateGame(board, agent2, agent1, true);
    cout << "4/8" << endl;
    
    // The Shatranj Opening 
    board = Board();
    board.executeMove(0b10000011, true);
    results[8] = simulateGame(board, agent1, agent2, false);
    results[9] = !simulateGame(board, agent2, agent1, false);
    cout << "5/8" << endl;
    
    // The Standard Opening
    board = Board();
    board.executeMove(24, true);
    board.executeMove(16, false);
    board.executeMove(24, true);
    board.executeMove(16, false);
    board.executeMove(24, true);
    board.executeMove(16, false);
    board.executeMove(0b11010100, true);
    results[10] = simulateGame(board, agent1, agent2, false);
    results[11] = !simulateGame(board, agent2, agent1, false);
    cout << "6/8" << endl;
    
    // The Shiller Opening
    board = Board();
    board.executeMove(24, true);
    board.executeMove(16, false);
    board.executeMove(24, true);
    board.executeMove(16, false);
    board.executeMove(24, true);
    board.executeMove(16, false);
    board.executeMove(0b10000100, true);
    results[12] = simulateGame(board, agent1, agent2, false);
    results[13] = !simulateGame(board, agent2, agent1, false);
    cout << "7/8" << endl;
    

    // The Quick Box Opening
    board = Board();
    board.executeMove(24, true);
    board.executeMove(0b10001100, false);
    results[14] = simulateGame(board, agent1, agent2, true);
    results[15] = !simulateGame(board, agent2, agent1, true);
    cout << "8/8" << endl;
    
    int firstModelWins = 0;
    for(bool result : results){
        if (result){
            firstModelWins++;
        }
    }

    return firstModelWins;
}


/**
 * Test architectures under constant computational time
 */
void testArchitecturesByTime(){
    string saveFileName = "testResults/arch_time";
    remove(saveFileName.c_str());
    ofstream saveFile(saveFileName);

    int baseModelSizes[5] = {1200, 2300, 5800, 11500, 22500};
    int heurModelSizes[5] = {1200, 2400, 5900, 12000, 24000};
    int GPModelSizes[5] = {0, 1200, 4500, 10000, 22000};

    for(int i = 0; i < 5; i++){
        MCTS agent_base = MCTS(baseModelSizes[i], 3, 0.25, 4, "GPmodels", false, 0);
        MCTS agent_heur = MCTS(heurModelSizes[i], 3, 0.25, 4, "GPmodels", false, 2);
        MCTS agent_GP = MCTS(GPModelSizes[i], 3, 0.25, 4, "GPmodels", true, 2);

        int heur_base = 0;
        int GP_base = 0;
        int GP_heur = 0;

        heur_base += contestModels(agent_heur, agent_base);
        heur_base += contestModels(agent_heur, agent_base);
        heur_base += contestModels(agent_heur, agent_base);

        if (i){
            GP_base += contestModels(agent_GP, agent_base);
            GP_base += contestModels(agent_GP, agent_base);
            GP_base += contestModels(agent_GP, agent_base);
            
            GP_heur += contestModels(agent_GP, agent_heur);
            GP_heur += contestModels(agent_GP, agent_heur);
            GP_heur += contestModels(agent_GP, agent_heur);
        }

        saveFile << GP_base << ", " << GP_heur << ", " << heur_base << endl;
    }
}


/**
 * Test architectures under constant number of rollouts
 */
void testArchitecturesBySize(){
    string saveFileName = "testResults/arch_size";
    remove(saveFileName.c_str());
    ofstream saveFile(saveFileName);
    
    int modelSizes[5] = {500, 1000, 2000, 5000, 10000};

    for(int i = 0; i < 5; i++){
        MCTS agent_base = MCTS(modelSizes[i], 3, 0.25, 4, "GPmodels", false, 0);
        MCTS agent_heur = MCTS(modelSizes[i], 3, 0.25, 4, "GPmodels", false, 2);
        MCTS agent_GP = MCTS(modelSizes[i], 3, 0.25, 4, "GPmodels", true, 2);

        int heur_base = 0;
        int GP_base = 0;
        int GP_heur = 0;

        heur_base += contestModels(agent_heur, agent_base);
        heur_base += contestModels(agent_heur, agent_base);
        heur_base += contestModels(agent_heur, agent_base);

        if (i){
            GP_base += contestModels(agent_GP, agent_base);
            GP_base += contestModels(agent_GP, agent_base);
            GP_base += contestModels(agent_GP, agent_base);
            
            GP_heur += contestModels(agent_GP, agent_heur);
            GP_heur += contestModels(agent_GP, agent_heur);
            GP_heur += contestModels(agent_GP, agent_heur);
        }

        saveFile << GP_base << ", " << GP_heur << ", " << heur_base << endl;
    }
}


/**
 * Test how size affects performance
 */
void testSize(){
    string saveFileName = "testResults/size_result";
    remove(saveFileName.c_str());
    ofstream saveFile(saveFileName);
    
    int rollouts[5] = {1000, 2000, 5000, 10000, 25000};

    MCTS agents[5];

    for(int i = 0; i < 5; i++){
        agents[i] = MCTS(rollouts[i], 3, 0.25, 4, "GPmodels", true, 2);
    }

    for(int i = 0; i < 5; i++){
        saveFile << i << ":\t";

        for(int j = i + 1; j < 5; j++){
            int result = 0;

            // repeat test 3 times to make it less noisy
            result += contestModels(agents[i], agents[j]);
            result += contestModels(agents[i], agents[j]);
            result += contestModels(agents[i], agents[j]);

            saveFile << result << ",\t";
        }
        saveFile << endl;
    }
}


/**
 * Test how simulations per rollouts affect performance
 */
void testSimulationsPerRolloutBySimulations(){
    string saveFileName = "testResults/SPR_result";
    remove(saveFileName.c_str());
    ofstream saveFile(saveFileName);
    
    int simulations = 12000;
    int simulationsPerRollout[5] = {1, 2, 3, 5, 10};

    MCTS agents[5];

    for(int i = 0; i < 5; i++){
        int rollouts = simulations / simulationsPerRollout[i];
        agents[i] = MCTS(rollouts, simulationsPerRollout[i], 0.25, 4, "GPmodels", true, 2);
    }

    for(int i = 0; i < 5; i++){
        saveFile << i << ":\t";

        for(int j = i + 1; j < 5; j++){
            int result = 0;

            // repeat test 3 times to make it less noisy
            result += contestModels(agents[i], agents[j]);
            result += contestModels(agents[i], agents[j]);
            result += contestModels(agents[i], agents[j]);

            saveFile << result << ",\t";
        }
        saveFile << endl;
    }
}


/**
 * Test how mctsParameter affects performance
 */
void testMCTSParam(){
    string saveFileName = "testResults/MCTSparam_result";
    remove(saveFileName.c_str());
    ofstream saveFile(saveFileName);
    
    double MCTSParams[13] = {0.05, 0.1, 0.5, 0.75, 1, 1.25, 1.5, 1.75, 2, 2.5, 3, 4, 5};

    MCTS challenger = MCTS(10000, 3, 0.25, 4, "GPmodels", true, 2);
    MCTS agents[13];

    for(int i = 0; i < 13; i++){
        agents[i] = MCTS(10000, 3, MCTSParams[i], 4, "GPmodels", true, 2);
    }

    for(int i = 0; i < 13; i++){
        int result = 0;

        // repeat test 3 times to make it less noisy
        result += contestModels(agents[i], challenger);
        result += contestModels(agents[i], challenger);
        result += contestModels(agents[i], challenger);

        saveFile << result << endl;
    }
}


/**
 * Test how rolloutPolicyParam affects performance
 */
void testRolloutPolicyParam(){
    string saveFileName = "testResults/RolloutPolicyParam_result";
    remove(saveFileName.c_str());
    ofstream saveFile(saveFileName);
    
    int rolloutPolicyParams[6] = {2, 3, 5, 6, 7, 8};

    MCTS challenger = MCTS(1000, 3, 0.25, 4, "GPmodels", true, 2);
    MCTS agents[6];

    for(int i = 0; i < 6; i++){
        agents[i] = MCTS(1000, 3, 0.25, rolloutPolicyParams[i], "GPmodels", true, 2);
    }

    for(int i = 0; i < 6; i++){
        int result = 0;

        // repeat test 3 times to make it less noisy
        result += contestModels(agents[i], challenger);
        result += contestModels(agents[i], challenger);
        result += contestModels(agents[i], challenger);

        saveFile << result << endl;
    }
}


/**
 * Calculate decision time for model
 */
double caculateDecisionTime(MCTS agent, int simulations = 10){
    Board board = Board();
    auto start = high_resolution_clock::now();

    for(int i = 0; i < simulations; i++){
        agent.predictBestMove(board, true);
    }

    auto end = high_resolution_clock::now();
    duration<double> elapsed = end - start;
    return elapsed.count() / simulations;
}

/**
 * Test decision time for different model sizes
 */
void testDecisionTime(){
    string saveFileName = "testResults/decisionTime";
    remove(saveFileName.c_str());
    ofstream saveFile(saveFileName);
    
    // Same rollouts as Lee's agents
    int rollouts[4] = {2500, 7500, 20000, 60000};
    for(int i = 0; i < 4; i++){
        agent = MCTS(rollouts[i], 1, 0.25, 4, "GPmodels", true, 2);
        double decisionTime = caculateDecisionTime(agent);
        saveFile << decisionTime << endl;
    }
}


int main(int argc, char const* argv[]) {
    auto start = high_resolution_clock::now();
    auto end = high_resolution_clock::now();
    duration<double> elapsed = end - start;
    
    testMCTSParam();
    testRolloutPolicyParam();

    testSize();
    testSimulationsPerRolloutBySimulations();

    testArchitecturesByTime();
    testArchitecturesBySize();

    testDecisionTime();
    end = high_resolution_clock::now();
    elapsed = end - start;
    cout << "Elapsed time: " << elapsed.count() / 3600 << " hours" << endl;

}


