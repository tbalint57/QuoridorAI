#include <iostream>
#include <stdint.h>
#include "board.cpp"

using namespace std;

int main(int argc, char const *argv[])
{
    Board game = Board();

    (game.movePawn(-1, 0, true));
    (game.placeWall(7, 4, true, true));

    (game.movePawn(-1, 0, false));
    (game.movePawn(0, 1, false));

    (game.placeWall(7, 4, true, true));
    (game.placeWall(7, 3, true, true));
    (game.placeWall(7, 5, true, true));
    (game.placeWall(7, 4, false, true));
    (game.placeWall(7, 5, false, true));

    (game.movePawn(0, -1, false));

    (game.placeWall(7, 5, false, true));
    (game.movePawn(1, 0, true));

    (game.movePawn(0, -1, false));

    (game.movePawn(1, 0, true));

    (game.movePawn(-1, 0, false));
    
    (game.movePawn(1, 0, true));

    (game.movePawn(-1, 0, false));
    
    (game.movePawn(1, 0, true));

    (game.movePawn(-1, 0, false));
    
    (game.movePawn(1, 0, true));

    (game.movePawn(0, 1, false));
    (game.movePawn(0, 2, false));

    (game.placeWall(4, 3, false, true));


    for(auto move : game.generatePossibleMoves(false)){
        cout << game.translateMove(move) << " ";
    }


    cout << (game.movePawn(0, -1, false)) << "\n";
    cout << (game.movePawn(0, -2, false)) << "\n";
    cout << (game.movePawn(1, -1, false)) << "\n";



    
    // (game.movePawn(2, 0, true)) << "Legal pawn move not accepted, hop over #17";

    // (game.movePawn(1, -1, false)) << "Legal pawn move not accepted, hop up-right #18";

    // (game.movePawn(1, 1, true)) << "Illegal pawn move, hop right-over wall #19";
    // (game.movePawn(0, 2, true)) << "Illegal pawn move, hop over wall #19";
    // (game.movePawn(1, -1, true)) << "Legal pawn move not accepted, hop right-down #19";




    // newBoard.bfs(newBoard.whitePawn, true);
    // cout << "\n";

    // newBoard.executeMove(0b11001011, true);
    // newBoard.bfs(newBoard.whitePawn, true);
    // cout << "\n";
    
    // newBoard.executeMove(0b11001101, true);
    // newBoard.bfs(newBoard.whitePawn, true);
    // cout << "\n";
    // newBoard.bfs(newBoard.blackPawn, false);
    // cout << "\n";
}
