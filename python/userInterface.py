import pygame
import sys
from game import Game
from board import Board
from cppInterface import calculateBestMove
from button import RectButton, RoundButton, ImageButton
import os
import time
from datetime import datetime
import shutil

pygame.init()

# Define board settings
BUTTON_HEIGHT = 50
BOARD_SIZE = 9  
CELL_SIZE = 90 
SPACING = 6

# Calculate window size
WINDOW_SIZE = BOARD_SIZE * CELL_SIZE + (BOARD_SIZE - 1) * SPACING
WIDTH = WINDOW_SIZE + 3 * CELL_SIZE
HEIGHT = WINDOW_SIZE
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("Quoridor")

# Colors
BG_COLOR = (101, 55, 57)
CELL_COLOR = (59, 59, 61)
WHITE_COLOR = (214, 169, 121)
BLACK_COLOR = (73, 21, 31)
WALL_COLOR = (246, 173, 88)

# Load images
own_dir = os.path.dirname(os.path.abspath(__file__))
images_path = os.path.join(own_dir, 'images/')
images = {image[:-4]: pygame.image.load(images_path + image) for image in os.listdir(images_path)}


def cellToRealCoordinates(row, col):
    x = col * (CELL_SIZE + SPACING)
    y = row * (CELL_SIZE + SPACING)
    return x, y


def spawnImageButton(image, center, action):
    return ImageButton(center, image, action)


def drawBoard(gameState):
    def drawPawn(position, color=(230, 200, 200)):
        row, col = position
        row = 8 - row
        x, y = cellToRealCoordinates(row, col)
        
        center_x = x + CELL_SIZE // 2
        center_y = y + CELL_SIZE // 2
        
        pygame.draw.circle(screen, color, (center_x, center_y), CELL_SIZE // 3)

    def drawWalls(walls):

        def drawWall(wall, color = WALL_COLOR):
            isHorizontal, row, col = wall
            row = 8 - row
            x, y = cellToRealCoordinates(row, col)

            if isHorizontal:
                pygame.draw.rect(screen, color, (x, y - SPACING, 2 * CELL_SIZE + SPACING, SPACING))
            if not isHorizontal:
                pygame.draw.rect(screen, color, (x + CELL_SIZE, y - CELL_SIZE - SPACING, SPACING, 2 * CELL_SIZE + SPACING))
                
        for wall in walls:
            drawWall(wall)

    screen.fill(BG_COLOR)
    for row in range(BOARD_SIZE):
        for col in range(BOARD_SIZE):
            x = col * (CELL_SIZE + SPACING)
            y = row * (CELL_SIZE + SPACING)

            pygame.draw.rect(screen, CELL_COLOR, (x, y, CELL_SIZE, CELL_SIZE))
    
    drawPawn(gameState.whitePawn, WHITE_COLOR)
    drawPawn(gameState.blackPawn, BLACK_COLOR)
    drawWalls(gameState.wallsOnBoard)


def getMoveAI(board):
    moveChar = calculateBestMove(board.gameState, board.whiteTurn)
    return Game.translateMove(moveChar)


def getMovePlayer(board):
    def highlightPossibleMoves(gameState, player, wall = None):

        def spawnPawnMove(position, move):
            col, row = position
            col = 8 - col

            x = col * (CELL_SIZE + SPACING)
            y = row * (CELL_SIZE + SPACING)

            return RectButton(y, x, CELL_SIZE, CELL_SIZE, move)

        def spawnWallPlacment(position, move):
            row, col = position
            row = 8 - row
            col += 1
            x, y = cellToRealCoordinates(row, col)
            button = RoundButton((x - SPACING / 2, y - SPACING / 2), SPACING, move)
            return button
        
        xPawn, yPawn = gameState.whitePawn if player else gameState.blackPawn
        possibleMoves = []
        for move in gameState.generatePossibleMoves(player):
            isWall, position, isHorizontal = move

            if not isWall:
                xMove, yMove = position
                possibleMoves.append(spawnPawnMove((xPawn + xMove, yPawn + yMove), move))
            
            if isWall and wall == "horizontal" and isHorizontal:
                possibleMoves.append(spawnWallPlacment(position, move))
            
            if isWall and wall == "vertical" and not isHorizontal:
                possibleMoves.append(spawnWallPlacment(position, move))

        return possibleMoves

    def spawnHorizontalWallButton(clicked):
        return RectButton(CELL_SIZE * 9.5 + SPACING * 8, 3 * CELL_SIZE, 2 * CELL_SIZE + SPACING, SPACING, (-1, -1), (WALL_COLOR[0] / 4, WALL_COLOR[1] / 4, WALL_COLOR[2] / 4), clicked)

    def spawnVerticalWallButton(clicked):
        return RectButton(CELL_SIZE * 10.5 + SPACING * 8, 4 * CELL_SIZE, SPACING, 2 * CELL_SIZE + SPACING, (-1, -1), (WALL_COLOR[0] / 4, WALL_COLOR[1] / 4, WALL_COLOR[2] / 4), clicked)
    
    while True:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
                print("quit called")
                quit()
                
        horizontalWallButton = None
        verticalWallButton = None

        drawBoard(board.gameState)
        
        if (board.whiteTurn and board.gameState.whiteWalls > 0) or (not board.whiteTurn and board.gameState.blackWalls > 0):
            horizontalWallButton = spawnHorizontalWallButton(board.isHorizontalPressed)
            verticalWallButton = spawnVerticalWallButton(board.isVerticalPressed)
            horizontalWallButton.draw(screen)
            verticalWallButton.draw(screen)

        possibleMoves = highlightPossibleMoves(board.gameState, board.whiteTurn, board.getWallDirection())
        
        if horizontalWallButton is not None and horizontalWallButton.clicked and not board.isHorizontalPressed:
            board.pressHorizontal()
            
        elif verticalWallButton is not None and verticalWallButton.clicked and not board.isVerticalPressed:
            board.pressVertical()

        for moveButton in possibleMoves:
            moveButton.draw(screen)
            if moveButton.clicked:
                return moveButton.move

        pygame.display.flip()


def play(moveFunctions):
    time.sleep(1)
    print("play")

    if moveFunctions == None:
        spectateGame()

    board = Board()
    board.reset()
    gameRecord = open("record.txt", "w")
    running = True

    whiteMoveFunc, blackMoveFunc = moveFunctions

    while running:
        drawBoard(board.gameState)

        if board.whiteTurn:
            move = whiteMoveFunc(board)

        else:
            move = blackMoveFunc(board)

        board.executeMove(move)
        running = board.gameState.winner == ""

        gameRecord.write(str(Game.compressMove(move)))
        drawBoard(board.gameState)
        pygame.display.flip()
    
    endOfGamePage(board.gameState.winner)
        

def spectateGame():
    pass


def homePage():
    time.sleep(1)
    print("homePage")
    time.sleep(0.5)
    font = pygame.font.Font(None, 74)
    text = font.render(f"Please choose the game mode!", True, (255, 255, 255))
    text_rect = text.get_rect(center=(WIDTH // 2, HEIGHT // 5))

    running = True
    while running:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False

        screen.fill(BG_COLOR)
        screen.blit(text, text_rect)

        buttons = []
        buttons.append(spawnImageButton(images["buttonAIvAI"], (WIDTH // 2, HEIGHT // 3), (getMoveAI, getMoveAI)))
        buttons.append(spawnImageButton(images["buttonAIvP"], (WIDTH // 2, HEIGHT // 3 + BUTTON_HEIGHT * 2), (getMoveAI, getMovePlayer)))
        buttons.append(spawnImageButton(images["buttonPvAI"], (WIDTH // 2, HEIGHT // 3 + BUTTON_HEIGHT * 4), (getMovePlayer, getMoveAI)))
        buttons.append(spawnImageButton(images["buttonPvP"], (WIDTH // 2, HEIGHT // 3 + BUTTON_HEIGHT * 6), (getMovePlayer, getMovePlayer)))
        buttons.append(spawnImageButton(images["buttonSpectate"], (WIDTH // 2, HEIGHT // 3 + BUTTON_HEIGHT * 8), None))
        buttons.append(spawnImageButton(images["buttonExit"], (WIDTH // 2, HEIGHT // 3 + BUTTON_HEIGHT * 10), 10))

        for button in buttons:
            button.draw(screen)
        
        for button in buttons:
            button.draw(screen)
            if button.clicked:
                running = False
                play(button.action)

        pygame.display.flip()


def quit():
    pygame.quit()
    sys.exit()


def colourSelection():
    pass


def endOfGamePage(winner):
    time.sleep(1)
    print("endOfGamePage")
    font = pygame.font.Font(None, 74)
    text = font.render(f"The winner is {winner}!", True, (255, 255, 255))
    text_rect = text.get_rect(center=(WIDTH // 2, HEIGHT // 5))

    running = True
    while running:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False

        screen.fill(BG_COLOR)
        screen.blit(text, text_rect)

        buttons = []
        buttons.append(spawnImageButton(images["buttonHome"], (WIDTH // 2, HEIGHT // 3), homePage))
        buttons.append(spawnImageButton(images["buttonSave"], (WIDTH // 2, HEIGHT // 3 + BUTTON_HEIGHT * 2), saveGame))
        buttons.append(spawnImageButton(images["buttonExit"], (WIDTH // 2, HEIGHT // 3 + BUTTON_HEIGHT * 4), quit))

        for button in buttons:
            button.draw(screen)
            if button.clicked:
                running = False
                button.action()

        pygame.display.flip()


def saveGame():
    now = datetime.now()
    saveFile = "savedGames/game_" + str(now)
    shutil.copy("record.txt", saveFile)

    homePage()



homePage()

