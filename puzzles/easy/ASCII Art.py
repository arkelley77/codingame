import sys
import math

# Auto-generated code below aims at helping you parse
# the standard input according to the problem statement.

l = int(input())
h = int(input())
t = input()
row = []
letters = []
letters_ref = ["a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u",
               "v", "w", "x", "y", "z"]
letters_ref_caps = ["A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T",
                    "U", "V", "W", "X", "Y", "Z"]
for i in range(h):
    row.append(input())
    letters.append([])

# puts letters so it is multi-array with letters[row][letter#]
for a in range(h):
    for i in range(0, len(row[a]), l):
        letters[a].append(row[a][i:i + l])

a = ""

for i in range(h):
    for j in t:
        try:
            a = a + (letters[i][letters_ref.index(j)])
        except ValueError:
            try:
                a = a + (letters[i][letters_ref_caps.index(j)])
            except ValueError:
                a = a + letters[i][26]
    a = a + "\n"
print(a)
