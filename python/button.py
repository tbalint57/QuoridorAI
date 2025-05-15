import pygame
import math
# Just a bunch of buttons for the UI

CELL_COLOR = (59, 59, 61)

# Circular button for placing walls
class RoundButton():
    def __init__(self, center, radius, move):
        self.clicked = False

        self.isWall = True
        self.move = move

        self.center = center
        self.radius = radius
    
    def draw(self, screen, colour=(230, 200, 200), border=(255,255,255)):
        pos = pygame.mouse.get_pos()

        pygame.draw.circle(screen, border, self.center, self.radius + 1)
        pygame.draw.circle(screen, colour, self.center, self.radius)

        if self.isMouseOverButton(pos):
            pygame.draw.circle(screen, (border[0] // 2, border[1] // 2, border[2] // 2), self.center, self.radius + 1)
            pygame.draw.circle(screen, (colour[0] // 2, colour[1] // 2, colour[2] // 2), self.center, self.radius)

            if pygame.mouse.get_pressed()[0] == 1 and not self.clicked:
                self.clicked = True

    def isMouseOverButton(self, mouse_pos):
        return math.dist(mouse_pos, self.center) <= self.radius
    

# Rectengular Buttons for pawn movement and wall selection
class RectButton():
    def __init__(self, left, top, width, height, move, colour = CELL_COLOR, clicked = False):
        self.clicked = clicked
        self.move = move
        self.isWall = False
        
        self.rect = pygame.Rect(left, top, width, height)
        self.colour = colour
    
    def draw(self, screen):
        pos = pygame.mouse.get_pos()

        pygame.draw.rect(screen, (self.colour[0] * 2, self.colour[1] * 2, self.colour[2] * 2), self.rect)

        if self.isMouseOverButton(pos) or self.clicked:
            pygame.draw.rect(screen, (self.colour[0] * 4, self.colour[1] * 4, self.colour[2] * 4), self.rect)
            if pygame.mouse.get_pressed()[0] == 1:
                self.clicked = True

    def isMouseOverButton(self, mouse_pos):
        return self.rect.collidepoint(mouse_pos)
    

# Image button for menus 
class ImageButton():
    def __init__(self, center, image, action):
        self.clicked = False

        width = image.get_width()
        height = image.get_height()

        self.image = image
        self.rect = self.image.get_rect()
        self.rect.center = center

        self.action = action
    
    def draw(self, screen):
        screen.blit(self.image, (self.rect.x, self.rect.y))
        pos = pygame.mouse.get_pos()

        if self.isMouseOverButton(pos):
            if pygame.mouse.get_pressed()[0] == 1 and not self.clicked:
                self.clicked = True

        return self.clicked

    def isMouseOverButton(self, mouse_pos):
        return self.rect.collidepoint(mouse_pos)