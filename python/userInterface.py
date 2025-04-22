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

# Modifications during holiday to talk about:
#     * code cleanup
#     * saving and veiwing past matches finally works

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

    def drawText(text, position, color=(255, 255, 255)):
        font = pygame.font.Font(None, 36)  # Set font and size
        textSurface = font.render(text, True, color)
        screen.blit(textSurface, position)

    screen.fill(BG_COLOR)

    for row in range(BOARD_SIZE):
        for col in range(BOARD_SIZE):
            x = col * (CELL_SIZE + SPACING)
            y = row * (CELL_SIZE + SPACING)

            pygame.draw.rect(screen, CELL_COLOR, (x, y, CELL_SIZE, CELL_SIZE))

    drawPawn(gameState.whitePawn, WHITE_COLOR)
    drawPawn(gameState.blackPawn, BLACK_COLOR)
    drawWalls(gameState.wallsOnBoard)

    # Display wall counts
    whiteWallsText = f"White Walls: {gameState.whiteWalls}"
    blackWallsText = f"Black Walls: {gameState.blackWalls}"
    whiteWallsPos = (screen.get_width() - 200, screen.get_height() - 50)
    blackWallsPos = (screen.get_width() - 200, 20)

    drawText(whiteWallsText, whiteWallsPos)
    drawText(blackWallsText, blackWallsPos)


def getMoveAI(board):
    moveChar = calculateBestMove(board.gameState, board.whiteTurn, 50_000)
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
        

def getMoveSpectate(board: Board):
    while True:
        for event in pygame.event.get():
            if event.type == pygame.KEYDOWN:
                move = board.moves[board.currentMove]
                board.currentMove += 1
                return move
            
            if event.type == pygame.QUIT:
                running = False
                print("quit called")
                quit()


def selectGameSave():
    font = pygame.font.Font(None, 36)
    save_path = os.path.join(own_dir, 'savedGames/')

    def get_save_files():
        return [f for f in os.listdir(save_path) if os.path.isfile(os.path.join(save_path, f))]

    def render_save_files(save_files, selected_index, start_index, visible_count = 15):
        screen.fill(BG_COLOR)
        title_text = font.render("Select a Save File", True, WHITE_COLOR)
        screen.blit(title_text, (WIDTH // 2 - title_text.get_width() // 2, 20))
        
        # Render only the visible portion of the save files
        for i in range(visible_count):
            if start_index + i < len(save_files):
                save_file = save_files[start_index + i]
                color = BLACK_COLOR if start_index + i == selected_index else WHITE_COLOR
                text = font.render(save_file, True, color)
                screen.blit(text, (50, 100 + i * 40))
        
        pygame.display.flip()

    def selector():
        clock = pygame.time.Clock()
        running = True
        save_files = get_save_files()
        selected_index = 0
        start_index = 0
        visible_count = 18

        while running:
            render_save_files(save_files, selected_index, start_index, visible_count)

            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    running = False

                elif event.type == pygame.KEYDOWN:
                    if event.key == pygame.K_UP:
                        if selected_index > 0:
                            selected_index -= 1
                            if selected_index < start_index:
                                start_index -= 1

                    elif event.key == pygame.K_DOWN:
                        if selected_index < len(save_files) - 1:
                            selected_index += 1
                            # Adjust start_index if needed
                            if selected_index >= start_index + visible_count:
                                start_index += 1

                    elif event.key == pygame.K_RETURN:
                        print(f"Selected save file: {os.path.join(save_path, save_files[selected_index])}")
                        return os.path.join(save_path, save_files[selected_index])
                    
                    elif event.key == pygame.K_ESCAPE:
                        running = False

            clock.tick(30)

    return selector()


def play(moveFunctions):
    time.sleep(1)
    print("play")

    board = Board()
    board.reset()

    if moveFunctions == None:
        gameSave = selectGameSave()
        print(f"gameSave: {gameSave}")
        with open(gameSave, 'r') as saveFile:
            for line in saveFile: 
                board.moves.append(Game.translateMove(int(line.strip())))
            

        moveFunctions = (getMoveSpectate, getMoveSpectate)

    gameRecord = open("record.txt", "w")
    running = True

    whiteMoveFunc, blackMoveFunc = moveFunctions

    while running:
        drawBoard(board.gameState)
        pygame.display.flip()

        if board.whiteTurn:
            move = whiteMoveFunc(board)

        else:
            move = blackMoveFunc(board)

        board.executeMove(move)
        running = board.gameState.winner == ""

        print(str(Game.compressMove(move)))
        gameRecord.write(str(Game.compressMove(move)) + "\n")
        drawBoard(board.gameState)
        pygame.display.flip()
    
    gameRecord.close()
    endOfGamePage(board.gameState.winner)


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
    timestamp = now.strftime("%Y-%m-%d_%H-%M-%S")
    save_dir = os.path.join(own_dir, "savedGames")
    
    saveFile = os.path.join(save_dir, f"game_{timestamp}")
    shutil.copy("record.txt", saveFile)

    homePage()


homePage()

