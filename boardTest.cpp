#include "board.cpp"
#include <gtest/gtest.h>

int GetMeaningOfLife() {return 42;}

TEST(QuoridorBoard, TestMoves) {
    Board game = Board();
    ASSERT_FALSE(game.movePawn(-1, 0, true)) << "Illegal move, pawn moved outside of board #1";
    ASSERT_TRUE(game.placeWall(7, 4, true, true)) << "Legal wall placement not accepted #1";

    ASSERT_FALSE(game.movePawn(-1, 0, false)) << "Illegal move, pawn moved through wall #2";
    ASSERT_TRUE(game.movePawn(0, 1, false)) << "Legal pawn move not accepted #2";

    ASSERT_FALSE(game.placeWall(7, 4, true, true)) << "Illegal wall placement, place taken #3";
    ASSERT_FALSE(game.placeWall(7, 3, true, true)) << "Illegal wall placement, place taken #3";
    ASSERT_FALSE(game.placeWall(7, 5, true, true)) << "Illegal wall placement, place taken #3";
    ASSERT_FALSE(game.placeWall(7, 4, false, true)) << "Illegal wall placement, place taken #3";
    ASSERT_TRUE(game.placeWall(7, 5, false, true)) << "Legal wall placement not accepted #3";

    ASSERT_TRUE(game.movePawn(0, -1, false)) << "Legal pawn move not accepted #4";

    ASSERT_FALSE(game.placeWall(7, 5, false, true)) << "Illegal wall placement, complete wall-off #5";
    ASSERT_TRUE(game.movePawn(1, 0, true)) << "Legal pawn move not accepted #5";

    ASSERT_TRUE(game.movePawn(0, -1, false)) << "Legal pawn move not accepted #6";

    ASSERT_TRUE(game.movePawn(1, 0, true)) << "Legal pawn move not accepted #7";

    ASSERT_TRUE(game.movePawn(-1, 0, false)) << "Legal pawn move not accepted #8";
    
    ASSERT_TRUE(game.movePawn(1, 0, true)) << "Legal pawn move not accepted #9";

    ASSERT_TRUE(game.movePawn(-1, 0, false)) << "Legal pawn move not accepted #10";
    
    ASSERT_TRUE(game.movePawn(1, 0, true)) << "Legal pawn move not accepted #11";

    ASSERT_TRUE(game.movePawn(-1, 0, false)) << "Legal pawn move not accepted #12";
    
    ASSERT_TRUE(game.movePawn(1, 0, true)) << "Legal pawn move not accepted #13";

    ASSERT_FALSE(game.movePawn(0, 1, false)) << "Illegal pawn move, cell taken #14";
    ASSERT_TRUE(game.movePawn(0, 2, false)) << "Legal pawn move not accepted, hop over #14";

    ASSERT_TRUE(game.placeWall(4, 3, false, true)) << "Legal wall placement not accepted #15";

    ASSERT_FALSE(game.movePawn(0, -1, false)) << "Illegal pawn move, cell taken #16";
    ASSERT_FALSE(game.movePawn(0, -2, false)) << "Illegal pawn move, hop over wall #16";
    ASSERT_TRUE(game.movePawn(1, -1, false)) << "Legal pawn move not accepted, hop left-up #16";
    
    ASSERT_TRUE(game.movePawn(2, 0, true)) << "Legal pawn move not accepted, hop over #17";

    ASSERT_TRUE(game.movePawn(1, 1, false)) << "Legal pawn move not accepted, hop up-right #18";

    ASSERT_FALSE(game.movePawn(1, 1, true)) << "Illegal pawn move, hop right-over wall #19";
    ASSERT_FALSE(game.movePawn(0, 2, true)) << "Illegal pawn move, hop over wall #19";
    ASSERT_TRUE(game.movePawn(-1, 1, true)) << "Legal pawn move not accepted, hop right-down #19";
    
    ASSERT_FALSE(game.movePawn(-1, 0, false)) << "Illegal pawn move, cell taken #20";
    ASSERT_TRUE(game.movePawn(-2, 0, false)) << "Legal pawn move not accepted, hop down #20";
}