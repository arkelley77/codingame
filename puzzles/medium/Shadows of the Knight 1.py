# Batman will look for the hostages on a given building by jumping from one window to another using his grapnel gun.
# Batman's goal is to jump to the window where the hostages are located in order to disarm the bombs.
# Unfortunately he has a limited number of jumps before the bombs go off...

import sys
import math


# Auto-generated code below aims at helping you parse
# the standard input according to the problem statement.


class Point:
    def __init__(self, coords: tuple = (0, 0)):
        self.coords = coords
        self.x = coords[0]
        self.y = coords[1]

    @property
    def x(self):
        return self._x

    @x.setter
    def x(self, new_x: int):
        self._x = new_x

    @property
    def y(self):
        return self._y

    @y.setter
    def y(self, new_y: int):
        self._y = new_y

    @property
    def coords(self):
        return self.x, self.y

    @coords.setter
    def coords(self, new_coords: tuple):
        self.x = new_coords[0]
        self.y = new_coords[1]


# w: width of the building.
# h: height of the building.
w, h = [int(i) for i in input().split()]
n = int(input())  # maximum number of turns before game over.
x0, y0 = [int(i) for i in input().split()]

batman = Point((x0, y0))

y_below = 0
y_above = h - 1
x_right_of = 0
x_left_of = w - 1

# game loop
while True:
    bomb_dir = input()  # the direction of the bombs from batman's current location (U, UR, R, DR, D, DL, L or UL)

    # Write an action using print
    # To debug: print("Debug messages...", file=sys.stderr)
    if "U" in bomb_dir:
        y_above = batman.y
    elif "D" in bomb_dir:
        y_below = batman.y

    if "L" in bomb_dir:
        x_left_of = batman.x
    elif "R" in bomb_dir:
        x_right_of = batman.x

    if abs((x_left_of - x_right_of) / 2) < 1:
        if "L" in bomb_dir:
            batman.x = batman.x - 1
        elif "R" in bomb_dir:
            batman.x = batman.x + 1
    else:
        batman.x = int((x_left_of + x_right_of) / 2)

    if abs((y_above - y_below) / 2) < 1:
        if "U" in bomb_dir:
            batman.y = batman.y - 1
        elif "D" in bomb_dir:
            batman.y = batman.y + 1
    else:
        batman.y = int((y_above + y_below) / 2)

    # the location of the next window Batman should jump to.
    print(str(batman.x) + " " + str(batman.y))
