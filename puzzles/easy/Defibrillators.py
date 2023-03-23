# The city of Montpellier has equipped its streets with defibrillators to help save victims of cardiac arrests.
# The data corresponding to the position of all defibrillators is available online.
#
# Based on the data we provide in the tests, write a program that will allow users to find the defibrillator
# nearest to their location using their mobile phone.

import sys
from math import *


# Auto-generated code below aims at helping you parse
# the standard input according to the problem statement.
def parse_str_float(string):
    res = int(string.split(",")[0]) + (int(string.split(",")[1]) / pow(10, len(string.split(",")[1])))
    return res


lon = parse_str_float(input())
lat = parse_str_float(input())
n = int(input())
defibrillator_list = []
for i in range(n):
    defibrillator = input().split(";")
    defibrillator_list.append(defibrillator)

d = []
for defibrillator in defibrillator_list:
    x = (lon - parse_str_float(defibrillator[4])) * cos(radians((parse_str_float(defibrillator[5]) + lat) / 2))
    y = lat - parse_str_float(defibrillator[5])
    d.append(sqrt(pow(x, 2) + pow(y, 2)) * 6371)

print(defibrillator_list[d.index(min(d))][1])
