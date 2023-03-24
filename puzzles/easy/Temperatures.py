# Auto-generated code below aims at helping you parse
# the standard input according to the problem statement.

n = int(input())  # the number of temperatures to analyse
t = []
u = []
for i in input().split():
    # t: a temperature expressed as an integer ranging from -273 to 5526
    t.append(abs(int(i)))
    u.append(int(i))

try:
    t = u[u.index(min(t))]
except ValueError:
    try:
        t = u[u.index(-min(t))]
    except ValueError:
        t = 0

# Write an action using print
# To debug: print("Debug messages...", file=sys.stderr)

print(t)
