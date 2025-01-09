from game import Game
from typing import Tuple, List
import time

class Board:
    def __init__(self, 
                 isHorizontalPressed: bool = False, 
                 isVerticalPressed: bool = False, 
                 gameState: Game = Game(), 
                 whiteTurn: bool = True, 
                 moves: List[Tuple[bool, Tuple[int, int], bool]] = []):
        
        self.isHorizontalPressed: bool = isHorizontalPressed
        self.isVerticalPressed: bool = isVerticalPressed
        self.gameState: Game = gameState
        self.whiteTurn: bool = whiteTurn
        self.moves: List[Tuple[bool, Tuple[int, int], bool]] = moves

    
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


    def executeMove(self, move: Tuple[bool, Tuple[int, int], bool], toAppend:bool=True):
        self.gameState.executeMove(move, self.whiteTurn) 
        self.isHorizontalPressed = False
        self.isVerticalPressed = False
        self.whiteTurn = not self.whiteTurn
        if toAppend:
            self.moves.append((move, self.whiteTurn))


    def getWallDirection(self):
        wallDirection = None

        if self.isHorizontalPressed:
            wallDirection = "horizontal"

        if self.isVerticalPressed:
            wallDirection = "vertical"

        return wallDirection


    def undo(self):
        pass