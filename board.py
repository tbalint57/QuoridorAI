from cppInterface import getPossibleMoves, getMove

class Board:
    def __init__(self, whitePawn = (0, 4), blackPawn = (8, 4), whiteWalls = 10, blackWalls = 10, wallsOnBoard = []):
        self.whitePawn = whitePawn
        self.blackPawn = blackPawn
        self.whiteWalls = whiteWalls
        self.blackWalls = blackWalls
        self.wallsOnBoard = wallsOnBoard


    @staticmethod
    def translateMove(moveChar):
        def checkBit(k):
            return (moveChar >> k) & 1 == 1

        isWallPlacement = checkBit(7)

        if isWallPlacement:
            isHorizontal = checkBit(6)
            i = (moveChar >> 3) & 7
            j = moveChar & 7

            return (isWallPlacement, i, j, isHorizontal)
        
        i = (moveChar >> 4) & 3
        if not checkBit(3):
            i = -i
        
        j = moveChar & 3
        if not checkBit(2):
            j = -j

        return (isWallPlacement, i, j, True)


    def executeMove(self, move, isWhitePlayer):
        isWallPlacement, i, j, isHorizontal = move
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


    def generatePossibleMoves(self, player):
        possibleMoveChars = getPossibleMoves(self, player)
        return [self.translateMove(moveChar) for moveChar in possibleMoveChars]
    
