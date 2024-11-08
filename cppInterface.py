import ctypes
from ctypes import *

lib = ctypes.CDLL('./interface.so') 

class Cell(Structure):
    _fields_ = [("i", c_int), ("j", c_int)]

class Wall(Structure):
    _fields_ = [("isHorizontal", c_bool), ("i", c_int), ("j", c_int)]

# uint8_t getMove(Cell whitePawn, Cell blackPawn, Wall* walls, size_t length, int whiteWalls, int blackWalls, bool player)
lib.calculateBestMove.argtypes = (Cell, Cell, POINTER(Wall), c_size_t, c_int, c_int, c_bool, c_int)
lib.calculateBestMove.restype = c_uint8

# uint8_t* getPossibleMoves(Cell whitePawn, Cell blackPawn, Wall* walls, size_t length, int whiteWalls, int blackWalls, bool player, size_t* outputLength)
lib.getPossibleMoves.argtypes = (Cell, Cell, POINTER(Wall), c_size_t, c_int, c_int, c_bool, POINTER(c_size_t))
lib.getPossibleMoves.restype = POINTER(c_uint8)

# void freeMemory(uint8_t* ptr)
lib.freeMemory.argtypes = (POINTER(c_uint8),)
lib.freeMemory.restype = None


def calculateBestMove(board, player, depth = 2):
    whitePawn = Cell(board.whitePawn[0], board.whitePawn[1])
    blackPawn = Cell(board.blackPawn[0], board.blackPawn[1])
    walls = (Wall * len(board.wallsOnBoard))(*board.wallsOnBoard)
    length = len(walls)

    result = lib.calculateBestMove(whitePawn, blackPawn, walls, length, board.whiteWalls, board.blackWalls, player, depth)

    return result


def getPossibleMoves(board, player):
    whitePawn = Cell(board.whitePawn[0], board.whitePawn[1])
    blackPawn = Cell(board.blackPawn[0], board.blackPawn[1])
    walls = (Wall * len(board.wallsOnBoard))(*board.wallsOnBoard)
    length = len(walls)
    outputLength = c_size_t()

    result = lib.getPossibleMoves(whitePawn, blackPawn, walls, length, board.whiteWalls, board.blackWalls, player, ctypes.byref(outputLength))

    moves = [result[i] for i in range(outputLength.value)]

    lib.freeMemory(result)

    return moves
