import sys
from math import *

# Auto-generated code below aims at helping you parse
# the standard input according to the problem statement.

# In this game your are a program driving the legendary Tron light
# cycle and fighting against other programs on the game grid.
#
# The light cycle moves in straight lines and only turn in 90° angles while leaving a solid light ribbon in its wake.
# Each cycle and associated ribbon features a different color.
# Should a light cycle stop, hit a light ribbon or goes off the game grid it will be instantly deactivated.
#
# The last cycle in play wins the game. Your goal is to be the best program: once sent to the arena,
# programs will compete against each-others in battles gathering 2 to 4 cycles.
# The more battles you win, the better your rank will be.

# this class defines some useful functions pertaining to a point on a Cartesian (x-y) plane
# it has properties x and y
# and functions distance_squared(), distance(), calc_angle(), and closest()
# (I don't really know what closest does; I found it online)
class Point:
    def __init__(self, x, y):
        self.x = x
        self.y = y

    @property
    def x(self) -> int:
        return self._x

    @x.setter
    def x(self, new_x):
        self._x = round(new_x)

    @property
    def y(self) -> int:
        return self._y

    @y.setter
    def y(self, new_y):
        self._y = round(new_y)

    def distance_squared(self, point) -> float:
        """

        :type point: Point
        :return: float distance^2 between self and point, accounting for outside shell of point if it has one
        """
        dx = self.x - point.x
        dy = self.y - point.y
        if issubclass(point, Point):
            return pow(dx, 2) + pow(dy, 2)
        else:
            raise TypeError("performing distance calculation with a non-point object!")

    def distance(self, point) -> float:
        """

        :type point: Point
        :return: float distance between self and point
        """
        return sqrt(self.distance_squared(point))


# this class defines a bike
# extends Point
# adds property direction
class Bike(Point):
    def __init__(self, x: int, y: int, id):
        super().__init__(x, y)
        self.direction = False
        self.walls = []

    def frame(self):
        self.walls.append(Wall(self.x, self.y))


# this class defines a wall
# extends Point
class Wall(Point):
    def __init__(self, x: int, y: int):
        super().__init__(x, y)


# game loop
while True:
    # n: total number of players (2 to 4).
    # p: your player number (0 to 3).
    n, p = [int(i) for i in input().split()]
    for i in range(n):
        # x0: starting X coordinate of lightcycle (or -1)
        # y0: starting Y coordinate of lightcycle (or -1)
        # x1: starting X coordinate of lightcycle (can be the same as X0 if you play before this player)
        # y1: starting Y coordinate of lightcycle (can be the same as Y0 if you play before this player)
        x0, y0, x1, y1 = [int(j) for j in input().split()]

    # Write an action using print
    # To debug: print("Debug messages...", file=sys.stderr)

    # A single line with UP, DOWN, LEFT or RIGHT
    print("LEFT")
