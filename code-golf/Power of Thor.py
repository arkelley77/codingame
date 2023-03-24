w,x,y,z=[int(j) for j in input().split()]
w,n=y-w,z-x
while 1:
 input()
 s=''
 if n:s='N'if n>0 else 'S';n-=n/abs(n)
 if w:s=s+'W'if w>0 else s+'E';w-=w/abs(w)
 print(s)