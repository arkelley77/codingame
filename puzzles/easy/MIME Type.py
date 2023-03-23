import re

# Auto-generated code below aims at helping you parse
# the standard input according to the problem statement.

n = int(input())    # number of elements which make up the association table.
q = int(input())    # number of file names to be analyzed.
ext = []            # extension list
mt = []             # MIME type list
ext_pattern = r'[.](\w*)'
for i in range(n):
    # ext: file extension
    # mt: MIME type.
    e, m = input().split()
    ext.append(e.lower())
    mt.append(m)
for i in range(q):
    file_name = input().lower()
    try:
        file_ext = re.findall(ext_pattern, file_name)
        file_ext = file_ext[len(file_ext) - 1]
        if len(file_ext) <= 10:
            try:
                file_mime = mt[ext.index(file_ext)]
                print(file_mime)
            except ValueError:
                print("UNKNOWN")
        else:
            print("UNKNOWN")
    except IndexError:
        print("UNKNOWN")
