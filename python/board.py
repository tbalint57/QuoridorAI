from game import Game
from typing import Tuple
import time

# Board representation in Python. Not the same as the C++, this is derictly stores information of the user interface
class Board:
    def __init__(self, 
                 isHorizontalPressed: bool = False, 
                 isVerticalPressed: bool = False, 
                 gameState: Game = Game(), 
                 whiteTurn: bool = True, 
                 moves: list[Tuple[bool, Tuple[int, int], bool]] = [],
                 currentMove: int = 0):
        
        self.isHorizontalPressed: bool = isHorizontalPressed
        self.isVerticalPressed: bool = isVerticalPressed
        self.gameState: Game = gameState    # This is the C++ board equivalent
        self.whiteTurn: bool = whiteTurn
        self.moves: list[Tuple[bool, Tuple[int, int], bool]] = moves
        self.currentMove = currentMove

    
    # resets board
    def reset(self):
        self.isHorizontalPressed = False
        self.isVerticalPressed = False
        self.gameState = Game((0, 4), (8, 4), 10, 10, [], "")
        self.whiteTurn = True
        self.moves = []

    
    # vertical wall button press
    def pressVertical(self):
        self.isVerticalPressed = True
        self.isHorizontalPressed = False


    # horizontal wall button press
    def pressHorizontal(self):
        self.isHorizontalPressed = True
        self.isVerticalPressed = False


    # execute move in gamestate, set wall buttons to unpressed
    def executeMove(self, move: Tuple[bool, Tuple[int, int], bool]):
        self.gameState.executeMove(move, self.whiteTurn) 
        self.isHorizontalPressed = False
        self.isVerticalPressed = False
        self.whiteTurn = not self.whiteTurn


    # handles which wall to place down
    def getWallDirection(self):
        wallDirection = None

        if self.isHorizontalPressed:
            wallDirection = "horizontal"

        if self.isVerticalPressed:
            wallDirection = "vertical"

        return wallDirection


    def undo(self):
        pass