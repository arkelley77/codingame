# The goal for your program is to safely land the "Mars Lander" shuttle,
# the landing ship which contains the Opportunity rover. Mars Lander is guided by a program,
# and right now the failure rate for landing on the NASA simulator is unacceptable.
#
# This puzzle is the second level of the "Mars Lander" trilogy.
# The controls are the same as the previous level but you must now control the angle in order to succeed.

import sys
import math

MAX_HS = 20
MAX_VS = 30

# Auto-generated code below aims at helping you parse
# the standard input according to the problem statement.
angle = math.degrees(math.acos(3.711 / 4.0))  # angle necessary to hover and accelerate horizontally

surface_n = int(input())  # the number of points used to draw the surface of Mars.
land_x_prev = -1
land_y_prev = -1
set_prev = True
for i in range(surface_n):
    # land_x: X coordinate of a surface point. (0 to 6999)
    # land_y: Y coordinate of a surface point.
    # By linking all the points together in a sequential fashion, you form the surface of Mars.
    land_x, land_y = [int(j) for j in input().split()]
    if land_y == land_y_prev:
        if abs(land_x - land_x_prev) >= 1000:
            landing_x = (land_x + land_x_prev) / 2
            landing_y = land_y
            landing_width = abs(land_x - land_x_prev)
            set_prev = True
        else:
            set_prev = False


def is_lander_over_target(ship_x: float):  # returns bool of whether or not lander is over the target
    return abs(ship_x - landing_x) <= (landing_width / 2)


def calc_angle_slow(v_x: float, v_y: float):
    speed = math.sqrt(pow(v_x, 2) + pow(v_y, 2))
    return math.degrees(math.asin(v_x / speed))


def calc_thrust_to_fly(v_y: float, v_goal):
    if v_y >= v_goal:
        return 3
    else:
        return 4


# game loop
while True:
    # h_speed: the horizontal speed (in m/s), can be negative.
    # v_speed: the vertical speed (in m/s), can be negative.
    # fuel: the quantity of remaining fuel in liters.
    # rotate: the rotation angle in degrees (-90 to 90).
    # power: the thrust power (0 to 4).
    x, y, h_speed, v_speed, fuel, rotate, power = [int(i) for i in input().split()]

    # Write an action using print
    # To debug: print("Debug messages...", file=sys.stderr)

    if is_lander_over_target(x):
        # over target; kill HS and begin descent
        if h_speed > 0:
            # going right
            rotation = angle
            thrust = 4
        elif h_speed < 0:
            # going left
            rotation = -angle
            thrust = 4
        else:
            # perfect
            rotation = 0
            thrust = calc_thrust_to_fly(v_speed, -MAX_VS)
    elif x - landing_width > (landing_width / 2):
        # too far right
        if h_speed < -(4 * MAX_HS):
            # too fast
            rotation = -angle
            thrust = 4
        elif h_speed > -(2 * MAX_HS):
            # too slow
            rotation = angle
            thrust = 4
        else:
            # okay speed
            rotation = 0
            thrust = calc_thrust_to_fly(v_speed, 0)
    else:
        # too far left
        if h_speed > (4 * MAX_HS):
            # too fast
            rotation = angle
            thrust = 4
        elif h_speed < (2 * MAX_HS):
            # too slow
            rotation = -angle
            thrust = 4
        else:
            # okay speed
            rotation = 0
            thrust = calc_thrust_to_fly(v_speed, 0)

    # rotate power. rotate is the desired rotation angle. power is the desired thrust power.
    print(str(thrust) + " " + str(rotation))
