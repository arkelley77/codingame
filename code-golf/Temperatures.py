input();print(min(sorted([int(j) for j in input().split()])[::-1],key=abs,default=0))