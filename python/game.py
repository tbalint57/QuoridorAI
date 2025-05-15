from cppInterface import getPossibleMoves
from typing import Tuple

# Game state representation in python. Only stores and converts information.
class Game:
    def __init__(self, 
                 whitePawn: Tuple[int, int] = (0, 4), 
                 blackPawn: Tuple[int, int] = (8, 4), 
                 whiteWalls: int = 10, 
                 blackWalls: int = 10, 
                 wallsOnBoard: list[Tuple[bool, int, int]] = [],
                 winner:str = ""):
        
        self.whitePawn: Tuple[int, int] = whitePawn
        self.blackPawn: Tuple[int, int] = blackPawn
        self.whiteWalls: int = whiteWalls
        self.blackWalls: int = blackWalls
        self.wallsOnBoard: list[Tuple[bool, int, int]] = wallsOnBoard
        self.winner: str = winner


    # translates C++ move representation (byte) into python move representation (Tuple)
    @staticmethod
    def translateMove(moveChar: int) -> Tuple[bool, Tuple[int, int], bool]: 
        def checkBit(k):
            return (moveChar >> k) & 1 == 1

        isWallPlacement = checkBit(7)

        if isWallPlacement:
            isHorizontal = checkBit(6)
            i = (moveChar >> 3) & 7
            j = moveChar & 7

            return (isWallPlacement, (i, j), isHorizontal)
        
        i = (moveChar >> 4) & 3
        if not checkBit(3):
            i = -i
        
        j = moveChar & 3
        if not checkBit(2):
            j = -j

        return (isWallPlacement, (i, j), None)
    

    # translates python move representation (Tuple) int C++ move representation (byte)
    @staticmethod
    def compressMove(move: Tuple[bool, Tuple[int, int], bool]) -> int:
        isWall, position, isHorizontal = move
        i, j = position

        moveChar = 0

        if isWall:
            moveChar += 128
        
            if isHorizontal:
                moveChar += 64

            moveChar += 8 * i
            moveChar += j

            return moveChar
        
        if i > 0:
            moveChar += 8
        
        if j > 0:
            moveChar += 4

        moveChar += 16 * abs(i)
        moveChar += abs(j)
        return moveChar


    # executes move on board
    def executeMove(self, move: Tuple[bool, Tuple[int, int], bool], isWhitePlayer: bool) -> None:
        isWallPlacement, (i, j), isHorizontal = move
        if isWallPlacement:
            self.wallsOnBoard.append((isHorizontal, i, j))

            if isWhitePlayer:
                self.whiteWalls -= 1 
            else:
                self.blackWalls -= 1
            
            return 
        
        if isWhitePlayer:
            self.whitePawn = (self.whitePawn[0] + i, self.whitePawn[1] + j)

        else:
            self.blackPawn = (self.blackPawn[0] + i, self.blackPawn[1] + j)

        if self.whitePawn[0] == 8:
            self.winner = "white"
        
        if self.blackPawn[0] == 0:
            self.winner = "black"


    # generates possible moves (by calling C++ representation)
    def generatePossibleMoves(self, player: bool) -> list[Tuple[bool, Tuple[int, int], bool]]:
        possibleMoveChars = getPossibleMoves(self, player)
        return [self.translateMove(moveChar) for moveChar in possibleMoveChars]
    
