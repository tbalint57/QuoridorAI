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


bool simulateGame(Board board, int rolloutPolicyParameter1, float mctsParameter1, int rolloutPolicyParameter2, float mctsParameter2, bool whiteMove){
    for (int i = 0; i < 40 && !board.getWinner(); i++){
        int rolloutPolicyParameter = whiteMove ? rolloutPolicyParameter1 : rolloutPolicyParameter2;
        float mctsParameter = whiteMove ? mctsParameter1 : mctsParameter2;

        uint8_t bestMove = mctsGetBestMove(board, ROLLOUTS, SIMULATIONS_PER_ROLLOUT, whiteMove, rolloutPolicyParameter, mctsParameter);
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
    cout << rolloutPolicyParameter1 << "," << mctsParameter1 << "-" << rolloutPolicyParameter2 << "," << mctsParameter2 << ": " << difference << "\n";

    return difference;
}


void initialSearch(int* rolloutPolicyParameter, float* mctsParameter){
    int cur_rpp = *rolloutPolicyParameter;
    float cur_mctsp = *mctsParameter;

    bool changed = true;
    while(changed){
        changed = false;
        int rpps[3] = {cur_rpp + 1, cur_rpp, cur_rpp - 1};
        float mctsps[3] = {cur_mctsp / 1.5, cur_mctsp, cur_mctsp * 1.5};

        int bestDifference = 2;
        int best_rpp = cur_rpp;
        float best_mctsp = cur_mctsp;

        for(int rpp : rpps){
            if(rpp < 2){
                continue;
            }

            for(float mctsp : mctsps){
                int difference = contestParameters(rpp, mctsp, cur_rpp, cur_mctsp);
                if(difference > bestDifference){
                    bestDifference = difference;
                    best_rpp = rpp;
                    best_mctsp = mctsp;
                }
            }
        }

        if(bestDifference > 0 && !(cur_rpp == best_rpp && cur_mctsp ==  best_mctsp)){
            changed = true;
            cur_rpp = best_rpp;
            cur_mctsp = best_mctsp;
        }

        cout << cur_rpp << " " << cur_mctsp << "\n";
    }

    *rolloutPolicyParameter = cur_rpp;
    *mctsParameter = cur_mctsp;
}


int main(int argc, char const* argv[]) {
    int initialRolloutPolicyParameter = 4;
    float initialMctsParameter = 1;
    initialSearch(&initialRolloutPolicyParameter, &initialMctsParameter);
    cout << initialRolloutPolicyParameter << " " << initialMctsParameter << "\n";
}