import pygame
import sys
from board import Board
from cppInterface import getMove
import time

pygame.init()

initialGameState = {"whitePawn": (3, 4), "blackPawn": (8, 4), "walls": [{"position": (1, 2), "isHorizontal": True}, {"position": (4, 2), "isHorizontal": True}, {"position": (7, 1), "isHorizontal": False},]}

# Define board settings
BOARD_SIZE = 9  
CELL_SIZE = 90 
SPACING = 5

# Calculate window size
WINDOW_SIZE = BOARD_SIZE * CELL_SIZE + (BOARD_SIZE - 1) * SPACING
screen = pygame.display.set_mode((WINDOW_SIZE, WINDOW_SIZE))
pygame.display.set_caption("Quoridor")

# Colors
BG_COLOR = (101, 55, 57)
CELL_COLOR = (59, 59, 61)
WHITE_COLOR = (214, 169, 121)
BLACK_COLOR = (73, 21, 31)
WALL_COLOR = (246, 173, 88)


def cellToRealCoordinates(row, col):
    x = col * (CELL_SIZE + SPACING)
    y = row * (CELL_SIZE + SPACING)
    return x, y


def drawBoard(gameState):
    screen.fill(BG_COLOR)
    for row in range(BOARD_SIZE):
        for col in range(BOARD_SIZE):
            x = col * (CELL_SIZE + SPACING)
            y = row * (CELL_SIZE + SPACING)

            pygame.draw.rect(screen, CELL_COLOR, (x, y, CELL_SIZE, CELL_SIZE))
    
    drawPawn(gameState.whitePawn, WHITE_COLOR)
    drawPawn(gameState.blackPawn, BLACK_COLOR)
    drawWalls(gameState.wallsOnBoard)


def drawPawn(position, color):
    row, col = position
    row = 8 - row
    x, y = cellToRealCoordinates(row, col)
    
    center_x = x + CELL_SIZE // 2
    center_y = y + CELL_SIZE // 2
    
    pygame.draw.circle(screen, color, (center_x, center_y), CELL_SIZE // 3)


def drawWalls(walls):
    for wall in walls:
        drawWall(wall)


def drawWall(wall, color = WALL_COLOR):
    isHorizontal, row, col = wall
    row = 8 - row
    x, y = cellToRealCoordinates(row, col)

    if isHorizontal:
        pygame.draw.rect(screen, color, (x - SPACING, y - SPACING, 2 * CELL_SIZE + SPACING, SPACING))
    if not isHorizontal:
        pygame.draw.rect(screen, color, (x + CELL_SIZE, y - CELL_SIZE - SPACING, SPACING, 2 * CELL_SIZE + SPACING))


game = Board()
whiteTurn = True
gameRecord = open("record.txt", "w")
index = 2

# Main game loop
running = True
while running:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
    
    # Draw the board
    drawBoard(game)
    # Update display
    pygame.display.flip()

    move = getMove(game, whiteTurn)

    game.executeMove(Board.translateMove(move), whiteTurn)
    whiteTurn = not whiteTurn
    gameRecord.write("#" + str(index // 2) + "\t" + str(move) + ":\t" + str(game.whitePawn) + "\t" + str(game.blackPawn) + "\n")
    index += 1
    time.sleep(1)

    

# Quit Pygame
pygame.quit()
sys.exit()