import ctypes

class Board:
    def __init__(self, whitePawn = (0, 4), blackPawn = (7, 4), whiteWalls = 10, blackWalls = 10, wallsOnBoard = []):
        self.whitePawn = whitePawn
        self.blackPawn = blackPawn
        self.whiteWalls = whiteWalls
        self.blackWalls = blackWalls
        self.wallsOnBoard = wallsOnBoard

    def executeMove(self, isWallPlacement, i, j, isWhitePlayer, isHorizontal = True):
        if isWallPlacement:
            self.wallsOnBoard.append((isHorizontal, i, j))

            if isWhitePlayer:
                self.whiteWalls -= 1 
            else:
                self.blackWalls -= 1
            
            return 
        
        if isWhitePlayer:
            self.whitePawn = (i, j)

        else:
            self.blackPawn = (i, j)

    def generatePossibleMoves(self):
        possibleMoves = []