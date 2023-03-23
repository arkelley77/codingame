import sys
import math

# Auto-generated code below aims at helping you parse
# the standard input according to the problem statement.

n = int(input())
horse_list = []
for i in range(n):
    horse_list.append(int(input()))

horse_list = sorted(horse_list)

diff_list = []
for i in range(n - 1):
    diff_list.append(horse_list[i + 1] - horse_list[i])

# Write an action using print
# To debug: print("Debug messages...", file=sys.stderr)

print(min(diff_list))
