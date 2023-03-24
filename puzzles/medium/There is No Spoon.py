# The game is played on a rectangular grid with a given size. Some cells contain power nodes.
# The rest of the cells are empty.
#
# The goal is to find, when they exist, the horizontal and vertical neighbors of each node.

import sys
import math

# Don't let the machines win. You are humanity's last hope...

class Node:
    def __init__(self, x, y):
        self.x = x
        self.y = y
        self.neighbors_x = []
        self.neighbors_y = []

    def add_neighbor(self, node):
        if node.x == self.x and self.y < node.y:
            self.neighbors_x.append(node)
        elif node.y == self.y and self.x < node.x:
            self.neighbors_y.append(node)

    def get_info(self):
        return str(self.x) + " " + str(self.y)


width = int(input())  # the number of cells on the X axis
height = int(input())  # the number of cells on the Y axis
nodeListXY = []
for i in range(width):
    nodeListXY[i] = [False]
nodeListYX = []
for i in range(height):
    nodeListYX[i] = [False]
# parse the input, converting strings to Nodes
for y in range(height):
    n = 0
    row = input()  # width characters, each either 0 or .
    for x in range(width):
        n = row.index(".", n)
        nodeListYX[y][x] = Node(n, row)

# find the horizontal neighbors first



# Write an action using print
# To debug: print("Debug messages...", file=sys.stderr)


# Three coordinates: a node, its right neighbor, its bottom neighbor
print(len(nodeListYX[0]))