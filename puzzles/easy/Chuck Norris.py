import sys
import math

# Auto-generated code below aims at helping you parse
# the standard input according to the problem statement.

message = input().encode("ASCII")
message_bin = ""    # the binary form of the message, will be added to later
amt_list = []       # a list of lists with the inner lists being [value, # of occurrences]
occurrences = 0
encoded = ""

# parse the message for chars, then encode those chars into pure binary and add them to message_bin
for i in message:
    message_bin = message_bin + (bin(i)[2:].zfill(7))

# now we have the message in pure binary, so we can perform the encryption
# parse the binary for recurring bits; add occurrence lists to amt_list
prev_bit = message_bin[0]
for bit in message_bin:
    if bit == prev_bit:
        occurrences = occurrences + 1
    else:
        amt_list.append([prev_bit, occurrences])
        occurrences = 1
    prev_bit = bit
amt_list.append([prev_bit, occurrences])

# now we use amt_list to generate the encoded value
for occurrence in amt_list:
    if occurrence[0] == "0":
        encoded = encoded + "00 "
    else:
        encoded = encoded + "0 "

    for i in range(occurrence[1]):
        encoded = encoded + "0"
    encoded = encoded + " "
print(encoded[:-1]) # because the string ends with a space, take away the final char
