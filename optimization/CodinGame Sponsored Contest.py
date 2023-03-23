import sys
import math

# Auto-generated code below aims at helping you parse
# the standard input according to the problem statement.

# commands are stay, goLeft, goRight, goUp, and goDown

# it is my understanding that the goal is to avoid my opponents

# objects
wall = '#'
path = '_'
walls = []

def debug(thing, ending=''):
    print(thing, file=sys.stderr, end=ending)
    if ending == '\n' or '\n' in thing:
        sys.stderr.flush()

def goLeft():
    print('E')

def goRight():
    print('A')

def goUp():
    print('C')

def goDown():
    print('D')

def stay():
    print('B')

height = int(input())
width = int(input())

# walls works as walls[x][y]
for i in range(width):
    walls.append([])
    for j in range(height):
        walls[i].append(' ')
number_of_characters = int(input())
print("E")
# game loop
while True:
    up = input()
    right = input()
    down = input()
    left = input()
    players = []
    for i in range(number_of_characters):
        x, y = [int(j) for j in input().split()]
        players.append([x,y])
    players.remove([x,y])
    walls[x][y] = '*'
    walls[x][y - 1] = up
    walls[x + 1][y] = right
    walls[x][y + 1] = down
    walls[x - 1][y] = left
    
    for i in range(width):
        debug('x')
    debug('\n')
    for i in range(height):
        # print a row
        debug('x')
        for j in range(width):
            # print a character
            if [j, i] in players:
                debug('0')
            else:
                debug(walls[j][i])
        debug('x\n')
    for i in range(width):
        debug('x')
        

    goLeft()
