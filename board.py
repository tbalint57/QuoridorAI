from game import Game
from typing import Tuple
import time

class Board:
    def __init__(self, 
                 isHorizontalPressed: bool = False, 
                 isVerticalPressed: bool = False, 
                 gameState: Game = Game(), 
                 whiteTurn: bool = True, 
                 moves: Tuple[Tuple[bool, Tuple[int, int], bool], bool] = []):
        
        self.isHorizontalPressed: bool = isHorizontalPressed
        self.isVerticalPressed: bool = isVerticalPressed
        self.gameState: Game = gameState
        self.whiteTurn: bool = whiteTurn
        self.moves: Tuple[Tuple[bool, Tuple[int, int], bool], bool] = moves

    
    def reset(self):
        
        self.isHorizontalPressed = False
        self.isVerticalPressed = False
        self.gameState = Game((0, 4), (8, 4), 10, 10, [], "")
        self.whiteTurn = True
        self.moves = []



    
    def pressVertical(self):
        self.isVerticalPressed = True
        self.isHorizontalPressed = False


    def pressHorizontal(self):
        self.isHorizontalPressed = True
        self.isVerticalPressed = False


    def executeMove(self, move: Tuple[bool, Tuple[int, int], bool]):
        self.gameState.executeMove(move, self.whiteTurn) 
        self.moves.append((move, self.whiteTurn))
        self.isHorizontalPressed = False
        self.isVerticalPressed = False
        self.whiteTurn = not self.whiteTurn


    def getWallDirection(self):
        wallDirection = None

        if self.isHorizontalPressed:
            wallDirection = "horizontal"

        if self.isVerticalPressed:
            wallDirection = "vertical"

        return wallDirection


    def undo(self):
        pass