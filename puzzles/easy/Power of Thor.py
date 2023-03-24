import sys
import math


def distance2_formula(point_1: tuple, point_2: tuple):
    return pow(point_1[0] - point_2[0], 2) + pow(point_1[1] - point_2[1], 2)


def distance_formula(point_1: tuple, point_2: tuple):
    return math.sqrt(distance2_formula(point_1, point_2))


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


# Auto-generated code below aims at helping you parse
# the standard input according to the problem statement.
# ---
# Hint: You can use the debug stream to print initialTX and initialTY, if Thor seems not follow your orders.

# light_x: the X position of the light of power
# light_y: the Y position of the light of power
# initial_tx: Thor's starting X position
# initial_ty: Thor's starting Y position
light_x, light_y, initial_tx, initial_ty = [int(i) for i in input().split()]
thor = Point(coords=(initial_tx, initial_ty))
goal = Point(coords=(light_x, light_y))


def direction(target: Point):
    go_north = False
    go_south = False
    go_east = False
    go_west = False
    if thor.y - target.y > 0:
        go_north = True
    if target.y - thor.y > 0:
        go_south = True
    if target.x - thor.x > 0:
        go_east = True
    if thor.x - target.x > 0:
        go_west = True

    if go_north:
        if go_east:
            thor.coords = (thor.x + 1, thor.y - 1)
            return "NE"
        elif go_west:
            thor.coords = (thor.x - 1, thor.y - 1)
            return "NW"
        else:
            thor.coords = (thor.x, thor.y - 1)
            return "N"
    elif go_south:
        if go_east:
            thor.coords = (thor.x + 1, thor.y + 1)
            return "SE"
        elif go_west:
            thor.coords = (thor.x - 1, thor.y + 1)
            return "SW"
        else:
            thor.coords = (thor.x, thor.y + 1)
            return "S"
    elif go_east:
        thor.coords = (thor.x + 1, thor.y)
        return "E"
    elif go_west:
        thor.coords = (thor.x - 1, thor.y)
        return "W"


def debug(thing):
    print(thing, file=sys.stderr)


# game loop
while True:
    remaining_turns = int(input())  # The remaining amount of turns Thor can move. Do not remove this line.

    # A single line providing the move to be made: N NE E SE S SW W or NW
    print(direction(goal))
