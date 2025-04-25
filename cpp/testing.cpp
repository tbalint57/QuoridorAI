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


int contestModels(MCTS agent1, MCTS agent2){
    bool results[2];

    // Starting State
    Board board = Board();
    results[0] = simulateGame(board, agent1, agent2, true);
    results[1] = !simulateGame(board, agent2, agent1, true);
    // cout << "1/8" << endl;
    
    // // The Sidewall Opening 
    // board = Board();
    // board.executeMove(24, true);
    // board.executeMove(16, false);
    // board.executeMove(179, true);
    // results[2] = simulateGame(board, agent1, agent2, false);
    // results[3] = !simulateGame(board, agent2, agent1, false);
    // cout << "2/8" << endl;
    
    // // The Rush Opening 
    // board = Board();
    // board.executeMove(24, true);
    // board.executeMove(16, false);
    // board.executeMove(24, true);
    // board.executeMove(16, false);
    // board.executeMove(24, true);
    // board.executeMove(16, false);
    // board.executeMove(163, true);
    // results[4] = simulateGame(board, agent1, agent2, false);
    // results[5] = !simulateGame(board, agent2, agent1, false);
    // cout << "3/8" << endl;

    // // The Reed Opening 
    // board = Board();
    // board.executeMove(0b11101010, true);
    // board.executeMove(16, false);
    // board.executeMove(0b11101101, true);
    // board.executeMove(16, false);
    // results[6] = simulateGame(board, agent1, agent2, true);
    // results[7] = !simulateGame(board, agent2, agent1, true);
    // cout << "4/8" << endl;
    
    // // The Shatranj Opening 
    // board = Board();
    // board.executeMove(0b10000011, true);
    // results[8] = simulateGame(board, agent1, agent2, false);
    // results[9] = !simulateGame(board, agent2, agent1, false);
    // cout << "5/8" << endl;
    
    // // The Standard Opening
    // board = Board();
    // board.executeMove(24, true);
    // board.executeMove(16, false);
    // board.executeMove(24, true);
    // board.executeMove(16, false);
    // board.executeMove(24, true);
    // board.executeMove(16, false);
    // board.executeMove(0b11010100, true);
    // results[10] = simulateGame(board, agent1, agent2, false);
    // results[11] = !simulateGame(board, agent2, agent1, false);
    // cout << "6/8" << endl;
    
    // // The Shiller Opening
    // board = Board();
    // board.executeMove(24, true);
    // board.executeMove(16, false);
    // board.executeMove(24, true);
    // board.executeMove(16, false);
    // board.executeMove(24, true);
    // board.executeMove(16, false);
    // board.executeMove(0b10000100, true);
    // results[12] = simulateGame(board, agent1, agent2, false);
    // results[13] = !simulateGame(board, agent2, agent1, false);
    // cout << "7/8" << endl;
    

    // // The Quick Box Opening
    // board = Board();
    // board.executeMove(24, true);
    // board.executeMove(0b10001100, false);
    // results[14] = simulateGame(board, agent1, agent2, true);
    // results[15] = !simulateGame(board, agent2, agent1, true);
    // cout << "8/8" << endl;
    
    int difference = 0;
    for(bool result : results){
        result ? difference++ : difference--;
    }

    return difference;
}


int main(int argc, char const* argv[]) {
    ROLLOUTS = 1000;
    SIMULATIONS_PER_ROLLOUT = 1;

    MCTS agent1 = MCTS(25000, 10, 0.5, 4, "GPmodels", true, 2);
    MCTS agent2 = MCTS(ROLLOUTS, SIMULATIONS_PER_ROLLOUT, 0.5, 4);

    Board board = Board();

    using namespace std::chrono;

    auto start = high_resolution_clock::now();
    auto end = high_resolution_clock::now();
    duration<double> elapsed = end - start;

    for(int i = 0; i < 10; i++){
        agent1.predictBestMove(board, true);
    }

    end = high_resolution_clock::now();
    elapsed = end - start;
    // std::cout << "Agent1 - Agent2 performance difference: "  << endl;
    cout << "Elapsed time: " << elapsed.count() / 10 << " secs" << endl;

}


