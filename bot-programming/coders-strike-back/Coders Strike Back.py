from sys import stderr
from math import sqrt, cos, sin, atan2, radians, degrees
from random import randint, random, sample
from time import perf_counter as time

"""
Rules from Coders Strike Back Gold:

 The players each control a team of two pods during a race.
 As soon as a pod completes the race, that pod's team is declared the winner.
 The circuit of the race is made up of checkpoints.
 To complete one lap, your vehicle (pod) must pass through each one in order and back through the start.
 The first player to reach the start on the final lap wins.

 The game is played on a map 16000 units wide and 9000 units high. The coordinate X=0, Y=0 is the top left pixel.

 The checkpoints work as follows:

     The checkpoints are circular, with a radius of 600 units.
     Checkpoints are numbered from 0 to N where 0 is the start and N-1 is the last checkpoint.
     The disposition of the checkpoints is selected randomly for each race.

 The pods work as follows:

     To pass a checkpoint, the center of a pod must be inside the radius of the checkpoint.
     To move a pod, you must print a target destination point followed by a thrust value.
       Details of the protocol can be found further down.
     The thrust value of a pod is its acceleration and must be between 0 and 100.
     The pod will pivot to face the destination point by a maximum of 18 degrees per turn
       and will then accelerate in that direction.
     You can use 1 acceleration boost in the race, you only need to replace the thrust value by the BOOST keyword.
     You may activate a pod's shields with the SHIELD command instead of accelerating.
       This will give the pod much more weight if it collides with another.
       However, the pod will not be able to accelerate for the next 3 turns.
     The pods have a circular force-field around their center, with a radius of 400 units,
       which activates in case of collisions with other pods.
     The pods may move normally outside the game area.
     If none of your pods make it to their next checkpoint in under 100 turns,
       you are eliminated and lose the game. Only one pod need to complete the race.
"""

"""
Advanced Rules from Coders Strike Back Gold:
On each turn the pods movements are computed this way:

    Rotation: the pod rotates to face the target point, with a maximum of 18 degrees (except for the 1rst round).
    Acceleration: the pod's facing vector is multiplied by the given thrust value. 
     The result is added to the current speed vector.
    Movement: The speed vector is added to the position of the pod. 
     If a collision would occur at this point, the pods rebound off each other.
    Friction: the current speed vector of each pod is multiplied by 0.85
    The speed's values are truncated and the position's values are rounded to the nearest integer.

Collisions are elastic. The minimum impulse of a collision is 120.
A boost is in fact an acceleration of 650. The number of boost available is common between pods. 
    If no boost is available, the maximum thrust is used.
A shield multiplies the Pod mass by 10.
The provided angle is absolute. 0° means facing EAST while 90° means facing SOUTH.
"""

"""
I/O Format Info from Coders Strike Back Gold:
Game Input
    Initialization input
        Line 1: laps : the number of laps to complete the race.
        Line 2: checkpointCount : the number of checkpoints in the circuit.
        Next checkpointCount lines : 2 integers checkpointX , checkpointY for the coordinates of checkpoint.
    Input for one game turn
        First 2 lines: Your two pods.
        Next 2 lines: The opponent's pods.
        Each pod is represented by 6 integers:
            x & y for the position. 
            vx & vy for the speed vector. 
            angle for the rotation angle in degrees. 
            nextCheckPointId for the number of the next checkpoint the pod must go through.
    Output for one game turn
        Two lines: 2 integers for the target coordinates of your pod followed by thrust , 
        the acceleration to give your pod, or by SHIELD to activate the shields, or by BOOST for an acceleration burst. 
        One line per pod.
Constraints
    0 ≤ thrust ≤ 100
    2 ≤ checkpointCount ≤ 8
    Response time first turn ≤ 1000ms
    Response time per turn ≤ 75ms
"""

"""
This code runs natively in radians, if you are confused how to use them just remember 2pi radians = 360 degrees
Also, because (0, 0) is in the top-left corner, all angles are clockwise from +x when placed on the field but
  when placed on a co-ordinate plane (unit circle) they are COUNTER-clockwise from +x,
  so the program ends up inverting the y-axis in its trig calculations.

For more information on trigonometric equations go to https://en.wikipedia.org/wiki/Trigonometric_functions
For more information on radians go to https://en.wikipedia.org/wiki/Radian
"""

MAX_TURN = radians(18.0)

AI_TIME_LIMIT = 0.075
AI_TIME_TOLERANCE = 0.005

BOUNDS = [[-MAX_TURN, MAX_TURN], [-2, 100], [-MAX_TURN, MAX_TURN], [-2, 100]]
BITS_PER_VAR = 7
NUM_VARS = len(BOUNDS)
POPULATION_SIZE = 8
R_CROSS = 0.95
R_MUT = 0.2
K = 2

# coefficients for evaluating the score of the state of a pod
DIST_TO_CP_COEFFICIENT = -2e-1  # ranges in the five thousands so bring down to the thousands
ANGLE_TO_CP_COEFFICIENT = 0  # only a very short-term impact
ANGLE_POD_CP1_CP2_COEFFICIENT = 0  # range of angle is 0 to pi and we want as straight a shot as possible
VELOCITY_ALIGN_COEFFICIENT = 0  # how closely the velocity vector aligns with the next checkpoint
LAPS_COEFFICIENT = 0  # we also calculate fractional laps so this includes checkpoints
BOOSTS_COEFFICIENT = 0  # coefficient on how many boosts remain, can be 0 or 1
SHIELD_FRAMES_COEFFICIENT = 0  # ranges from 0 to 3, fewer is better


# coefficients for evaluating the score of the state of a team
TIMEOUT_COEFFICIENT = 0  # goes from 0 to 100 so convert to thousands
BEST_POD_COEFFICIENT = 1
WORST_POD_COEFFICIENT = 1


def debug(*args, **kwargs):
    kwargs['file'] = stderr
    print(*args, **kwargs)
    stderr.flush()


t = time()
debug('started')


Numeric = (float, int)
VectorLikeList = (tuple, list)


class Vector:
    __slots__ = ['_x', '_y', '_z', '_r', '_theta']

    def __init__(self, *args):
        """
        Vector can be initialized in many ways:
        Vector(): placeholder vector with magnitude 0
        Vector(Circle): position of the circle
        Vector(Vector): clones the vector
        Vector(VectorLikeList(x, y, (z)))
        Vector(x, y)
        Vector(VectorLikeList(x, y, (z)), magnitude): if magnitude of vector is already known, saves time
        Vector(x, y, z)
        Vector(x, y, z, magnitude)
        """
        magnitude_not_given = True
        if len(args) == 0:
            self._x, self._y, self._z = 0, 0, 0
            self._r = 0.0
            self._theta = 0.0
            magnitude_not_given = False
        elif len(args) == 1:
            args = args[0]
            if isinstance(args, Circle):
                args = args.position
                self._x, self._y, self._z = args.coordinates
                self._r = abs(args)
                self._theta = args.theta
                magnitude_not_given = False
            elif isinstance(args, Vector):
                self._x, self._y, self._z = args.coordinates
                self._r = abs(args)
                self._theta = args.theta
                magnitude_not_given = False
            elif isinstance(args, VectorLikeList):
                if len(args) == 2:
                    self._x, self._y = args
                    self._z = 0
                elif len(args) == 3:
                    self._x, self._y, self._z = args
            else:
                raise TypeError("Can't initialize Vector with only 1 argument of type", type(args))
        elif len(args) == 2:
            if all(isinstance(i, Numeric) for i in args):
                self._x, self._y = args
                self._z = 0
            elif isinstance(args[0], VectorLikeList):
                self._r = args[1]
                args = args[0]
                if len(args) == 2:
                    self._x, self._y = args
                    self._z = 0
                    self._theta = atan2(self._y, self._x)
                elif len(args) == 3:
                    self._x, self._y, self._z = args
                    self._theta = atan2(self._y, self._x)
                else:
                    raise TypeError("Can't use VectorLikeList of length", len(args))
                magnitude_not_given = False
            else:
                raise TypeError("Can't initialize Vector with 2 non-numeric arguments")
        elif len(args) == 3:
            if all(isinstance(i, Numeric) for i in args):
                self._x, self._y, self._z = args
            else:
                raise TypeError("Can't initialize Vector with 3 non-numeric arguments")
        elif len(args) == 4:
            if all(isinstance(i, Numeric) for i in args):
                self._x, self._y, self._z, self._r = args
                magnitude_not_given = False
            else:
                raise TypeError("Can't initialize Vector with 4 non-numeric arguments")
        else:
            raise TypeError("Vector expected at most 4 arguments, got", len(args))

        if magnitude_not_given:
            self._r = sqrt(self._x ** 2 + self._y ** 2 + self._z ** 2)
            self._theta = atan2(self.y, self.x)

        pass

    @property
    def x(self):
        return self._x

    @x.setter
    def x(self, x):
        if isinstance(x, Numeric):
            self._x = x
            self.refresh_magnitude()
        else:
            raise TypeError("Vector.x expected Numeric object, got", type(x))
        pass

    @property
    def y(self):
        return self._y

    @y.setter
    def y(self, y):
        if isinstance(y, Numeric):
            self._y = y
            self.refresh_magnitude()
        else:
            raise TypeError("Vector.y expected Numeric object, got", type(y))
        pass

    @property
    def z(self):
        return self._z

    @z.setter
    def z(self, z):
        if isinstance(z, Numeric):
            self._z = z
            self.refresh_magnitude()
        else:
            raise TypeError("Vector.z expected Numeric object, got", type(z))
        pass

    @property
    def r_2(self):
        """
        most programs optimize by using the squared magnitude and only square rooting when necessary, but this class
        stores the magnitude on creation and modification, so it's actually a tad slower to get the squared
        magnitude than the actual magnitude
        """
        return self._r ** 2

    @property
    def theta(self) -> float:
        """
        returns the angle of the vector in radians ccw from +x in a Cartesian plane
        """
        return self._theta

    @property
    def coordinates(self) -> tuple:
        return self.x, self.y, self.z

    @coordinates.setter
    def coordinates(self, new_coordinates):
        self.__init__(new_coordinates)

    def __neg__(self):
        """
        negates the vector's coordinates
        """
        x = -self._x
        y = -self._y
        z = -self._z
        return Vector(x, y, z)

    def __abs__(self):
        """
        returns magnitude of vector
        """
        return self._r

    def __add__(self, other):
        if isinstance(other, Vector):
            x = self._x + other.x
            y = self._y + other.y
            z = self._z + other.z
        elif isinstance(other, VectorLikeList):
            if len(other) == 3:
                if all(isinstance(i, Numeric) for i in other):
                    x = self._x + other[0]
                    y = self._y + other[1]
                    z = self._z + other[2]
                else:
                    raise TypeError("Can't perform '+' operation on Vector and", type(other), "of unsupported types")
            else:
                raise ValueError("Can't perform '+' operation on Vector and", type(other), "of length", len(other))
        else:
            raise TypeError("Can't perform '+' operation on Vector and object of type", type(other))
        return Vector(x, y, z)

    def __iadd__(self, other):
        if isinstance(other, Vector):
            x = self._x + other.x
            y = self._y + other.y
            z = self._z + other.z
        elif isinstance(other, VectorLikeList):
            if len(other) == 3:
                if all(isinstance(i, Numeric) for i in other):
                    x = self._x + other[0]
                    y = self._y + other[1]
                    z = self._z + other[2]
                else:
                    raise TypeError("Can't perform '+=' operation on Vector and", type(other), "of unsupported types")
            else:
                raise ValueError("Can't perform '+=' operation on Vector and", type(other), "of length", len(other))
        else:
            raise TypeError("Can't perform '+=' operation on Vector and object of type", type(other))
        self.coordinates = x, y, z
        return self

    def __sub__(self, other):
        if isinstance(other, Vector):
            x = self._x - other.x
            y = self._y - other.y
            z = self._z - other.z
        elif isinstance(other, VectorLikeList):
            if len(other) == 3:
                if all(isinstance(i, Numeric) for i in other):
                    x = self._x - other[0]
                    y = self._y - other[1]
                    z = self._z - other[2]
                else:
                    raise TypeError("Can't perform '-' operation on Vector and", type(other), "of unsupported types")
            else:
                raise ValueError("Can't perform '-' operation on Vector and", type(other), "of length", len(other))
        else:
            raise TypeError("Can't perform '-' operation on Vector and object of type", type(other))
        return Vector(x, y, z)

    def __isub__(self, other):
        if isinstance(other, Vector):
            x = self._x - other.x
            y = self._y - other.y
            z = self._z - other.z
        elif isinstance(other, VectorLikeList):
            if len(other) == 3:
                if all(isinstance(i, Numeric) for i in other):
                    x = self._x - other[0]
                    y = self._y - other[1]
                    z = self._z - other[2]
                else:
                    raise TypeError("Can't perform '-' operation on Vector and", type(other), "of unsupported types")
            else:
                raise ValueError("Can't perform '-' operation on Vector and", type(other), "of length", len(other))
        else:
            raise TypeError("Can't perform '-' operation on Vector and object of type", type(other))
        self.coordinates = x, y, z
        return self

    def __mul__(self, other):
        """
        if other is numeric, performs scalar multiplication
        if other is Vector or Vector-like, returns dot product
        """
        if isinstance(other, Numeric):
            x = self._x * other
            y = self._y * other
            z = self._z * other
            return Vector(x, y, z)
        elif isinstance(other, Vector):
            x = self._x * other.x
            y = self._y * other.y
            z = self._z * other.z
            return x + y + z
        elif isinstance(other, VectorLikeList):
            if len(other) == 3:
                if all(isinstance(i, Numeric) for i in other):
                    x = self._x * other[0]
                    y = self._y * other[1]
                    z = self._z * other[2]
                    return x + y + z
                else:
                    raise TypeError("Can't find Vector dot product with tuple of unsupported types")
            else:
                raise ValueError("Can't find Vector dot product with", type(other), "of length", len(other))
        else:
            raise TypeError("Can't perform '*' operation on Vector and object of type", type(other))
        pass

    def __imul__(self, other):
        """
            if other is numeric, performs scalar multiplication
            if other is Vector or Vector-like, returns dot product
        """
        if isinstance(other, Numeric):
            x = self._x * other
            y = self._y * other
            z = self._z * other
            self.coordinates = x, y, z
        else:
            raise TypeError("Can't perform '*=' operation on Vector and object of type", type(other))
        return self

    def __truediv__(self, other):
        """
        if other is numeric, performs scalar division
        if other is Vector or Vector-like, returns cross product
        """
        if isinstance(other, Numeric):
            if other > 0:
                x = self._x / other
                y = self._y / other
                z = self._z / other
                return Vector(x, y, z)
            elif other == 0:
                raise ZeroDivisionError("Can't perform scalar division by zero")
            else:
                raise ValueError("Can't perform scalar division by a negative number")
        elif isinstance(other, Vector):
            x = (self._y * other.z) - (self._z * other.y)
            y = (self._z * other.x) - (self._x * other.z)
            z = (self._x * other.y) - (self._y * other.x)
            return Vector(x, y, z)
        elif isinstance(other, VectorLikeList):
            x = (self._y * other[2]) - (self._z * other[1])
            y = (self._z * other[0]) - (self._x * other[2])
            z = (self._x * other[1]) - (self._y * other[0])
            return Vector(x, y, z)
        else:
            raise TypeError("Can't perform '/' operation on Vector and object of type", type(other))
        pass

    def __idiv__(self, other):
        """
        performs scalar division
        """
        if isinstance(other, Numeric):
            if other > 0:
                x = self._x / other
                y = self._y / other
                z = self._z / other
                self.coordinates = x, y, z
            elif other == 0:
                raise ZeroDivisionError("Can't perform '/=' operation on Vector and 0")
            else:
                raise ValueError("Can't perform scalar division on Vector and negative number")
        else:
            raise TypeError("Can't perform '/=' operation on Vector and object of type", type(other))
        return self

    def __floordiv__(self, other):
        """
        returns scalar projection of self on other
        """
        if isinstance(other, (Vector, VectorLikeList)):
            return (self * other) / abs(other) if abs(other) != 0 else 0
        else:
            raise TypeError("Can't perform scalar projection of Vector on object of type", type(other))
        pass

    def __mod__(self, other):
        """
        returns vector projection of self on other
        """
        if isinstance(other, (Vector, VectorLikeList)):
            return (other * (self // other)) / abs(other)

    def __str__(self):
        return "Vector with coordinates ({}, {}, {}) and magnitude {}".format(self._x, self._y, self._z, self._r)

    def get_rotated(self, angle, other):
        if isinstance(other, Vector):
            pass
        elif isinstance(other, VectorLikeList):
            other = Vector(other)
        else:
            raise TypeError("Can't rotate Vector about object of type", type(other))
        a_parallel_b = self % other  # component of self parallel to other
        a_orthogonal_b = self - a_parallel_b  # component of self orthogonal to other
        if abs(a_orthogonal_b) == 0:
            # self == other
            return self

        orthogonal_both = other / a_orthogonal_b
        """
        other
        ^
        |
        |
        |
        |
        |
        ^self // other
        |
        |
        |      other / (self - (self // other))
        |     /-----__
        |    /        ^---__
        |   /               \
        |  /                 \
        | /                   |
        |/                    |
         ----------------------self - (self // other)

        """

        if abs(orthogonal_both) == 0:
            return self

        a = a_orthogonal_b * cos(angle) / abs(a_orthogonal_b)
        b = orthogonal_both * sin(angle) / abs(orthogonal_both)
        a_orthogonal_b_rotated = (a + b) * abs(a_orthogonal_b)
        return a_parallel_b + a_orthogonal_b_rotated

    def get_distance_2(self, other):
        v = self - other
        return v.r_2

    def get_distance(self, other):
        return abs(self - other)

    def get_angle_to(self, other):
        return self.theta - other.theta

    def refresh_magnitude(self):
        r_2 = self.x ** 2 + self.y ** 2 + self.z ** 2
        self._r = sqrt(r_2)
        self._theta = atan2(self.y, self.x)
        pass

    def rotate(self, angle, other):
        v = self.get_rotated(angle, other)
        self.coordinates = v.coordinates
        pass

    pass


class UnitVector(Vector):
    def __init__(self, angle: float):
        super().__init__((cos(angle), sin(angle)), 1.0)
        pass

    def __str__(self):
        return "Unit vector with angle {} degrees".format(degrees(self.theta))

    pass


#   Unit Vectors
UNIT_V_I = Vector(1, 0, 0)
UNIT_V_J = Vector(0, 1, 0)
UNIT_V_K = Vector(0, 0, 1)

X_AXIS = UNIT_V_I
Y_AXIS = UNIT_V_J
Z_AXIS = UNIT_V_K


class Chromosome:
    def __init__(self):
        self.bits = []
        self.n_bits = 0
        self.bits_per_var = 0
        self.var_bounds = []
        self.n_vars = 0
        self.is_coded = False

    @classmethod
    def randomize(cls, n_bits: int):
        z = cls()
        z.bits = [randint(0, 1) for _ in range(n_bits)]
        z.n_bits = n_bits
        return z

    @classmethod
    def randomize_coded(cls, var_bounds: list, num_vars: int, bits_per_var: int):
        """
        Currently supports encoding for int, uint, float, ufloat

        :param var_bounds: list of variable boundary values, example: [[-12, 0], [-1600, 1599]]
        :param num_vars: the number of variables
        :param bits_per_var: the number of bits used to represent each variable
        """
        z = Chromosome.randomize(len(var_bounds) * bits_per_var)
        z.var_bounds = var_bounds
        z.n_vars = num_vars
        z.bits_per_var = bits_per_var
        z.is_coded = True
        return z

    @classmethod
    def copy(cls, chromosome):
        z = cls()
        z.bits = [i for i in chromosome.bits]
        z.n_bits = chromosome.n_bits
        z.bits_per_var = chromosome.bits_per_var
        z.var_bounds = chromosome.var_bounds
        z.n_vars = chromosome.n_vars
        z.is_coded = chromosome.is_coded
        return z

    def decode(self) -> list:
        if self.is_coded:
            l_vars = []
            bits_not_parsed = self.bits
            i = 0
            largest_possible_value = 2**self.bits_per_var
            while bits_not_parsed:
                this_var_bits = bits_not_parsed[:self.bits_per_var]
                low, high = self.var_bounds[i]

                chars = ''.join([str(i) for i in this_var_bits])
                integer = int(chars, 2)

                adjusted_value = low + ((integer * (high - low)) / largest_possible_value)

                l_vars.append(adjusted_value)
                bits_not_parsed = bits_not_parsed[self.bits_per_var:]
                i += 1

            return l_vars
        else:
            return self.bits

    def clone(self):
        return Chromosome.copy(self)

    pass


class TeamMove:

    def __init__(self, chromosome):
        self.chromosome = chromosome.clone()

    @property
    def p1_angle(self) -> Vector:
        return self.decode()[0]

    @property
    def p2_angle(self) -> Vector:
        return self.decode()[2]

    @property
    def p1_int_thrust(self) -> int:
        return self.decode()[1]

    @property
    def p2_int_thrust(self) -> int:
        return self.decode()[3]

    @property
    def p1_out_thrust(self) -> int or str:
        t = self.p1_int_thrust
        if t == -2:
            t = 'SHIELD'
        elif t == -1:
            t = 'BOOST'

        return t

    @property
    def p2_out_thrust(self) -> int or str:
        t = self.p2_int_thrust
        if t == -2:
            t = 'SHIELD'
        elif t == -1:
            t = 'BOOST'
        return t

    def decode(self) -> list:
        a1, thrust1, a2, thrust2 = self.chromosome.decode()
        thrust1 = round(thrust1)
        thrust2 = round(thrust2)
        return [a1, thrust1, a2, thrust2]

    def get_printout(self, team) -> str:
        p1 = team.pod_1
        p1_dest = p1.position + UnitVector(self.p1_angle + p1.facing.theta) * 2000
        p2 = team.pod_2
        p2_dest = p2.position + UnitVector(self.p2_angle + p2.facing.theta) * 2000
        return '{} {} {}\n{} {} {}'.format(round(p1_dest.x), round(p1_dest.y), self.p1_out_thrust,
                                           round(p2_dest.x), round(p2_dest.y), self.p2_out_thrust)

    pass


class GeneticOptimizer:
    def __init__(self, eval_func, n_bits: int, time_limit: float, time_tolerance: float, n_pop: int,
                 r_cross: float, r_mut: float, k: int = 3, maximize=True):
        self._objective = eval_func
        self.n_bits = n_bits
        self.time_limit = time_limit
        self.time_tolerance = time_tolerance
        self.population = []
        self.n_pop = n_pop
        self.r_cross = r_cross
        self.r_mut = r_mut
        self.k = k
        self.scores = []
        self.score_coeff = 1.0 if maximize else -1.0

    def objective(self, c: Chromosome):
        return self.score_coeff * self._objective(*c.decode())

    def generate_population(self):
        self.population = [Chromosome.randomize(self.n_bits) for _ in range(self.n_pop)]

    def select_parents(self) -> list:
        parents = []
        for _ in range(self.n_pop):
            sam = sample(self.population, k=self.k)
            i_sam = [self.population.index(i) for i in sam]
            scores = [self.scores[i] for i in i_sam]
            max_score = max(scores)
            i_max_score = scores.index(max_score)
            parents.append(sam[i_max_score])

        return parents

    def crossover(self, p1: Chromosome, p2: Chromosome) -> list:
        # children are clones of parents by default
        c1, c2 = p1.clone(), p2.clone()

        # check if there is a crossover event
        if random() < self.r_cross:
            # figure out where to split it, make sure not to split at end (does nothing)
            split = randint(1, self.n_bits - 2)
            # do the split
            c1.bits = p1.bits[:split] + p2.bits[split:]
            c2.bits = p2.bits[:split] + p1.bits[split:]

        return [c1, c2]

    def mutate(self, c: Chromosome):
        # mutations are clones by default
        c = c.clone()

        # check if there is a mutation event
        if random() < self.r_mut:
            bit_flip = randint(0, self.n_bits - 1)
            c.bits[bit_flip] = 1 - c.bits[bit_flip]

        return c

    def reproduce(self, parents: list) -> list:
        children = []
        # get pairs of parents
        for i in range(0, self.n_pop, 2):
            try:
                p1, p2 = parents[i], parents[i + 1]
            except IndexError:
                p1, p2 = parents[i], parents[0]
            # crossover
            c = self.crossover(p1, p2)
            # then mutate
            c = [self.mutate(i) for i in c]
            children += c

        return children

    def optimize(self, best_score: float = None, tolerance: float = 1e-6) -> list:
        if best_score is not None:
            best_score *= self.score_coeff
        # generate initial population, completely random
        self.generate_population()
        # evaluate candidates
        self.scores = [self.objective(i) for i in self.population]
        max_score = max(self.scores)
        i_max = self.scores.index(max_score)
        # evaluate the first member, keep track of best
        best, best_eval = self.population[i_max].clone(), max_score
        # run generations
        gen = 0
        while time()-t < self.time_limit - self.time_tolerance:
            gen += 1
            # choose parents
            parents = self.select_parents()
            # reproduce
            self.population = self.reproduce(parents)

            # evaluate candidates
            self.scores = [self.objective(i) for i in self.population]
            # check for best solution
            max_score = max(self.scores)
            i_max = self.scores.index(max_score)

            if max_score > best_eval:
                best, best_eval = self.population[i_max].clone(), max_score
                if best_score is not None:
                    if best_score - best_eval < tolerance:
                        return [best, self.score_coeff * best_eval]
        debug('genetic optimization completed at time', time()-t, 'after', gen, 'generations')
        debug('best move score:', best_eval)
        return [best, self.score_coeff * best_eval]

    pass


class GeneticContinuousOptimizer(GeneticOptimizer):
    def __init__(self, eval_func, input_bounds: list, bits_per_var: int, time_limit: float, time_tol: float, n_pop: int,
                 r_cross: float, r_mut: float, k: int = 3, maximize=True):
        self.n_inputs = len(input_bounds)
        super().__init__(eval_func, self.n_inputs * bits_per_var, time_limit, time_tol, n_pop, r_cross, r_mut,
                         k=k, maximize=maximize)
        self._objective = eval_func
        self.input_bounds = input_bounds
        self.bits_per_var = bits_per_var

    def objective(self, c: Chromosome):
        return self.score_coeff * self._objective(TeamMove(c))

    def generate_population(self):
        self.population = [Chromosome.randomize_coded(self.input_bounds, self.n_inputs, self.bits_per_var)
                           for _ in range(self.n_pop)]
        pass

    def optimize(self, best_score: float = None, tolerance: float = 1e-6) -> list:
        if best_score is not None:
            best_score *= self.score_coeff
        # generate initial population, completely random
        self.generate_population()
        # evaluate candidates
        self.scores = [self.objective(i) for i in self.population]
        best_eval = max(self.scores)
        i_max = self.scores.index(best_eval)
        # evaluate the first member, keep track of best
        best = self.population[i_max].clone()
        # run generations
        gen = 0
        while time()-t < self.time_limit - self.time_tolerance:
            gen += 1
            # choose parents
            parents = self.select_parents()
            # reproduce
            self.population = self.reproduce(parents)

            # evaluate candidates
            self.scores = [self.objective(i) for i in self.population]
            # check for best solution
            max_score = max(self.scores)
            i_max = self.scores.index(max_score)

            if max_score > best_eval:
                best, best_eval = self.population[i_max].clone(), max_score
                debug('new best move with score', best_eval)
                if best_score is not None:
                    if best_score - best_eval < tolerance:
                        return [best, self.score_coeff * best_eval]
        debug('genetic optimization completed at time', time()-t, 'after', gen, 'generations')
        debug('best move score:', best_eval)
        return [best, self.score_coeff * best_eval]

    pass


class Circle:
    __slots__ = ['_position', 'radius']

    def __init__(self, *args, **kwargs):
        self._position = Vector(*args)
        self.radius = 500
        if 'radius' in kwargs:
            self.radius = kwargs['radius']
        pass

    @property
    def x(self):
        return self.position.x

    @x.setter
    def x(self, x):
        self._position.x = x
        pass

    @property
    def y(self):
        return self.position.y

    @y.setter
    def y(self, y):
        self._position.y = y
        pass

    @property
    def z(self):
        return self.position.z

    @z.setter
    def z(self, z):
        self._position.z = z
        pass

    @property
    def coordinates(self):
        return self.position.coordinates

    @coordinates.setter
    def coordinates(self, new_coordinates):
        self._position.coordinates = new_coordinates
        pass

    @property
    def position(self):
        return self._position

    @position.setter
    def position(self, thing):
        self._position = Vector(thing)
        pass

    def check_collision(self, other):
        if isinstance(other, Circle):
            difference = self.position - other.position
            return abs(difference) <= self.radius + other.radius
        elif isinstance(other, Vector) or isinstance(other, VectorLikeList):
            difference = self.position - other
            return abs(difference) <= self.radius
        else:
            raise TypeError("Can't check collision between Circle and object of type", type(other))
        pass

    def bounce(self, other):
        pass

    pass


class MovableObject(Circle):
    __slots__ = ['_position', 'radius', 'velocity', '_facing', 'mass']

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        self.velocity = Vector(0, 0)
        if 'velocity' in kwargs:
            self.velocity = kwargs['velocity'] if kwargs['velocity'] is Vector else Vector(kwargs['velocity'])
        if 'vx' in kwargs:
            self.velocity.x = kwargs['vx']
        if 'vy' in kwargs:
            self.velocity.y = kwargs['vy']
        if 'vz' in kwargs:
            self.velocity.z = kwargs['vz']

        self.facing = 0
        if 'facing' in kwargs:
            self.facing = kwargs['facing']

        self.mass = 1
        if 'mass' in kwargs:
            if kwargs['mass'] is Numeric:
                self.mass = kwargs['mass']
            else:
                raise TypeError("Can't have a mass of type", type(kwargs['mass']))

        pass

    @property
    def facing(self):
        return self._facing

    @facing.setter
    def facing(self, thing):
        if isinstance(thing, Vector):
            if abs(thing) == 0 or abs(thing) == 1:
                self._facing = thing
            else:
                self._facing = UnitVector(thing.theta)
        elif isinstance(thing, VectorLikeList):
            self._facing = Vector(thing)
            self._facing = UnitVector(self._facing.theta)
        elif isinstance(thing, Circle):
            self._facing = thing.position - self.position
            self._facing = UnitVector(self._facing.theta)
        elif isinstance(thing, Numeric):
            self._facing = UnitVector(thing)
        else:
            raise TypeError("Can't make MovableObject facing vector be object of type", type(thing))
        pass

    def move(self, time: float):
        d_pos = self.velocity * time
        self.position += d_pos
        pass

    def bounce(self, other, min_impulse=120.0):
        if type(other) == Circle:
            return
        elif isinstance(other, MovableObject):
            m1 = self.mass
            m2 = other.mass
            mass_coefficient = (m1 + m2) / (m1 * m2)

            d_pos = self.position - other.position

            dist_squared = 6.4e5

            d_v = self.velocity - other.velocity

            product = d_pos.x * d_v.x + d_pos.y * d_v.y

            force_vector = (d_pos * product) / (dist_squared * mass_coefficient)

            self.velocity -= force_vector / m1
            other.velocity += force_vector / m2

            impulse = abs(force_vector)
            if impulse < min_impulse:
                force_vector *= min_impulse / impulse

            self.velocity -= force_vector / m1
            other.velocity += force_vector / m2
        else:
            raise TypeError("Can't bounce with object of type", type(other))

        pass

    def time_until_collision(self, other):
        # evaluates for 1 entire frame and figures out how long it will be until two objects collide
        if isinstance(other, MovableObject):
            d_v = self.velocity - other.velocity
        elif isinstance(other, Circle):
            d_v = self.velocity
        else:
            raise TypeError("Can't find time until a MovableObject collides with an object of type", type(other))

        if d_v == 0:
            return 2.0
        elif self.check_collision(other):
            return 0.0
        else:
            # shift to a coordinate system with other at the center
            d_pos = self.position - other.position

            sum_radii = self.radius + other.radius

            normal_vector_from_d_v = Vector(-d_v.y, d_v.x)
            if abs(normal_vector_from_d_v) == 0:
                return 2.0
            closest_point_rel = (d_pos % normal_vector_from_d_v)
            # negate so it's relative to other not to the closest point

            d_to_travel = closest_point_rel - d_pos
            if abs(closest_point_rel) > sum_radii:
                return 2.0
            elif abs(closest_point_rel) == sum_radii:
                return abs(d_to_travel) / abs(d_v)
            else:
                collision_point_compensation = sqrt(sum_radii ** 2 - abs(closest_point_rel) ** 2)
                d_to_travel -= (d_v * collision_point_compensation / abs(d_v))
                if abs(d_to_travel) <= abs(d_v):
                    return abs(d_to_travel) / abs(d_v)
                else:
                    return 2.0
        pass

    pass


class Checkpoint(Circle):
    def __init__(self, *args, **kwargs):
        if 'radius' in kwargs:
            kwargs['radius'] = 100
        super().__init__(*args, **kwargs)
        pass

    pass


class Pod(MovableObject):
    __slots__ = ['_position', 'radius', 'velocity', '_facing', 'mass', 'map', 'next_cp_id', 'team',
                 'laps', 'boosts_remaining', 'shield_frames_remaining']

    def __init__(self, game_map, team):
        super().__init__(radius=400)

        self.team = team

        self.map = game_map
        self.next_cp_id = 1
        self.laps = 0
        self.boosts_remaining = 1
        self.shield_frames_remaining = 0
        pass

    @property
    def next_checkpoint(self) -> Checkpoint:
        return self.map.checkpoints[self.next_cp_id]

    @property
    def second_to_next_checkpoint(self) -> Checkpoint:
        i = self.next_cp_id + 1
        if i >= self.map.num_checkpoints:
            i -= self.map.num_checkpoints
        return self.map.checkpoints[i]

    @property
    def next_cp_vector(self):
        return self.next_checkpoint.position - self.position

    def __str__(self):
        return 'Pod with position {} and velocity {}, facing angle {} degrees'.format(self.position.coordinates,
                                                                                      self.velocity.coordinates,
                                                                                      degrees(self.facing.theta))

    def bounce(self, other, min_impulse=120):
        if isinstance(other, Checkpoint):
            if other == self.next_checkpoint:
                self.next_cp_id += 1
                self.team.timeout = 100
                if self.next_cp_id >= self.map.num_checkpoints:
                    self.laps += 1
                    self.next_cp_id = 1
        else:
            super().bounce(other, min_impulse=min_impulse)
        pass

    def turn(self, angle):
        if abs(self.facing) == 0:
            self.facing = UnitVector(angle)
        elif angle > MAX_TURN:
            self.facing = UnitVector(self.facing.theta + MAX_TURN)
        elif angle < -MAX_TURN:
            self.facing = UnitVector(self.facing.theta - MAX_TURN)
        else:
            self.facing = UnitVector(self.facing.theta + angle)
        pass

    def turn_to(self, other):
        if isinstance(other, Vector):
            d_pos = other - self.position
        elif isinstance(other, Circle):
            d_pos = other.position - self.position
        elif isinstance(other, VectorLikeList):
            d_pos = -(self.position - other)
        else:
            raise TypeError("Can't turn toward object of type", type(other))

        d_theta = d_pos.theta - self.facing.theta
        self.turn(d_theta)
        pass

    def apply_thrust(self, thrust):
        if thrust == 'SHIELD':
            thrust = 0
            self.mass = 10
            self.shield_frames_remaining = 3

        if self.shield_frames_remaining < 0:
            if thrust == 'BOOST':
                thrust = 650
            else:
                thrust = int(thrust)
            self.velocity += self.facing * thrust
        pass

    def calc_thrust(self):
        dist_thrust = abs(self.next_cp_vector) / 10
        diff_angle = abs(self.next_cp_vector.theta - self.facing.theta)
        if diff_angle > radians(180.0):
            diff_angle = radians(360.0) - diff_angle
        angle_thrust = 1 - diff_angle / radians(180.0)
        thrust = dist_thrust * angle_thrust
        return thrust

    def apply_move(self, *args):
        if len(args) == 1:
            args = args
        if isinstance(args, str):
            args = args.split()
        if len(args) == 3:
            self.turn_to(Vector(int(args[0]), int(args[1])))
            self.apply_thrust(args[2])
        pass

    def finish_frame(self):
        self.velocity *= 0.85
        self.velocity.x = int(self.velocity.x)
        self.velocity.y = int(self.velocity.y)
        self.position.x = round(self.position.x)
        self.position.y = round(self.position.y)
        while self.next_cp_id >= self.map.num_checkpoints:
            self.next_cp_id -= self.map.num_checkpoints
            self.laps += 1
        self.mass = 1
        if self.shield_frames_remaining > 0:
            self.shield_frames_remaining -= 1
        pass

    def clone(self, team):
        a = Pod(self.map, team)
        a.position = self.position
        a.velocity = self.velocity
        a.facing = self.facing
        a.mass = self.mass
        a.laps = self.laps
        a.next_cp_id = self.next_cp_id
        a.boosts_remaining = self.boosts_remaining
        a.shield_frames_remaining = self.shield_frames_remaining
        return a

    def score(self):
        angle_to_checkpoint = self.next_cp_vector.theta - self.facing.theta
        angle_self_cp1_cp2 = abs((self.second_to_next_checkpoint.position - self.next_checkpoint.position).theta
                                 - (-self.next_cp_vector).theta)
        if angle_self_cp1_cp2 > radians(180.0):
            angle_self_cp1_cp2 = radians(360.0) - angle_self_cp1_cp2
        score = (DIST_TO_CP_COEFFICIENT * abs(self.next_cp_vector)
                 + ANGLE_TO_CP_COEFFICIENT * abs(angle_to_checkpoint)
                 + ANGLE_POD_CP1_CP2_COEFFICIENT * abs(angle_self_cp1_cp2)
                 + VELOCITY_ALIGN_COEFFICIENT * abs(self.next_cp_vector.theta - self.velocity.theta)
                 + LAPS_COEFFICIENT * (self.laps + ((self.next_cp_id - 1) / self.map.num_checkpoints))
                 + BOOSTS_COEFFICIENT * self.boosts_remaining
                 + SHIELD_FRAMES_COEFFICIENT * self.shield_frames_remaining
                 )
        return score

    pass


class Team:
    __slots__ = ['pod_1', 'pod_2', 'timeout', 'map']

    def __init__(self, game_map):
        self.pod_1 = Pod(game_map, self)
        self.pod_2 = Pod(game_map, self)
        self.map = game_map
        self.timeout = 100
        pass

    @property
    def pods(self):
        return [self.pod_1, self.pod_2]

    def score(self):
        s1 = self.pod_1.score()
        s2 = self.pod_2.score()
        if s2 > s1:
            return BEST_POD_COEFFICIENT * s2 + WORST_POD_COEFFICIENT * s1 + TIMEOUT_COEFFICIENT * self.timeout
        else:
            return BEST_POD_COEFFICIENT * s1 + WORST_POD_COEFFICIENT * s2 + TIMEOUT_COEFFICIENT * self.timeout

    def fetch(self):
        prev_1_next_cp_id = self.pod_1.next_cp_id
        self.pod_1.x, self.pod_1.y, \
            self.pod_1.velocity.x, self.pod_1.velocity.y, \
            facing, self.pod_1.next_cp_id = [int(i) for i in input().split()]
        if facing == -1:
            self.pod_1.facing = Vector(0, 0)
        else:
            facing = radians(facing)
            self.pod_1.facing = facing
        if self.pod_1.next_cp_id < prev_1_next_cp_id:
            self.pod_1.laps += 1

        self.pod_2.x, self.pod_2.y, \
            self.pod_2.velocity.x, self.pod_2.velocity.y, \
            facing, self.pod_2.next_cp_id = [int(i) for i in input().split()]
        if facing == -1:
            self.pod_2.facing = Vector(0, 0)
        else:
            facing = radians(facing)
            self.pod_2.facing = facing

        self.timeout -= 1
        pass

    def finish_frame(self):
        [i.finish_frame() for i in self.pods]

    def clone(self):
        t = Team(self.map)
        t.pod_1 = self.pod_1.clone(self)
        t.pod_2 = self.pod_2.clone(self)
        t.timeout = self.timeout
        return t

    def dumb_moves(self):
        dumb_moves = [self.pod_1.next_cp_vector - self.pod_1.velocity, self.pod_1.calc_thrust(),
                      self.pod_2.next_cp_vector - self.pod_2.velocity, self.pod_2.calc_thrust()]
        dumb_moves = '{} {} {}\n{} {} {}'.format(dumb_moves[0].x, dumb_moves[0].y, dumb_moves[1],
                                                 dumb_moves[2].x, dumb_moves[2].y, dumb_moves[3])
        return dumb_moves

    pass


class Map:
    __slots__ = ['num_laps', 'num_checkpoints', 'checkpoints']

    def __init__(self):
        self.num_laps = 0
        self.num_checkpoints = 0
        self.checkpoints = []
        self.fetch()
        pass

    def fetch(self):
        self.num_laps = int(input())
        self.num_checkpoints = int(input())
        for i in range(self.num_checkpoints):
            x, y = [int(i) for i in input().split()]
            self.checkpoints.append(Checkpoint(x, y))
        pass

    pass


class Game:
    __slots__ = ['map', 'friendly_team', 'enemy_team', 'pods_list', 'circles_list']

    def __init__(self, game_map):
        self.map = game_map
        self.friendly_team = Team(self.map)
        self.enemy_team = Team(self.map)
        self.pods_list = self.friendly_team.pods + self.enemy_team.pods
        self.circles_list = self.pods_list + self.map.checkpoints
        pass

    def fetch(self):
        self.friendly_team.fetch()
        self.enemy_team.fetch()
        pass

    def do_frame(self, friendly_moves: str, enemy_moves: str):
        friendly_moves = friendly_moves.split('\n')
        enemy_moves = enemy_moves.split('\n')
        self.friendly_team.pod_1.apply_move(friendly_moves[0])
        self.friendly_team.pod_2.apply_move(friendly_moves[1])
        self.enemy_team.pod_1.apply_move(enemy_moves[0])
        self.enemy_team.pod_2.apply_move(enemy_moves[1])

        self.collisions_in_frame()
        self.finish_frame()
        pass

    def collisions_in_frame(self):
        t = 0.0
        while t < 1.0:
            collisions = []
            pods_list_2 = [i for i in self.pods_list]
            for i in range(len(self.pods_list)):
                pod_1 = pods_list_2.pop(0)
                for pod_2 in pods_list_2:
                    t2 = pod_1.time_until_collision(pod_2)
                    if abs(t2) < 1e-10:
                        pod_1.bounce(pod_2)
                    elif t2 <= 1.0 - t:
                        collisions.append([pod_1, pod_2, t2])
                for checkpoint in self.map.checkpoints:
                    t2 = pod_1.time_until_collision(checkpoint)
                    if abs(t2) < 1e-10:
                        pod_1.bounce(checkpoint)
                    elif t2 <= 1.0 - t:
                        collisions.append([pod_1, checkpoint, t2])
            soonest = [self.pods_list[0], self.pods_list[1], 100]
            for i in collisions:
                if i[2] < soonest[2]:
                    soonest = i
            if soonest[2] == 100:
                [pod.move(1.0 - t) for pod in self.pods_list]
                break
            else:
                [pod.move(soonest[2]) for pod in self.pods_list]
                t += soonest[2]
        pass

    def finish_frame(self):
        self.friendly_team.finish_frame()
        self.enemy_team.finish_frame()
        pass

    def evaluate_position(self):
        return self.friendly_team.score() - self.enemy_team.score()

    pass


class Emulator(Game):
    def __init__(self, game_to_emulate: Game):
        super().__init__(game_to_emulate.map)
        self.friendly_team = game_to_emulate.friendly_team.clone()
        self.enemy_team = game_to_emulate.enemy_team.clone()
        self.pods_list = self.friendly_team.pods + self.enemy_team.pods
        self.circles_list = self.pods_list + self.map.checkpoints
        pass

    def evaluate_move(self, friendly_team_move: TeamMove):
        f_move = friendly_team_move.get_printout(self.friendly_team)
        e_move = '{} {} {}\n{} {} {}'.format(self.enemy_team.pod_1.next_checkpoint.x - self.enemy_team.pod_1.velocity.x,
                                             self.enemy_team.pod_1.next_checkpoint.y - self.enemy_team.pod_1.velocity.y,
                                             self.enemy_team.pod_1.calc_thrust(),
                                             self.enemy_team.pod_2.next_checkpoint.x - self.enemy_team.pod_2.velocity.x,
                                             self.enemy_team.pod_2.next_checkpoint.y - self.enemy_team.pod_2.velocity.y,
                                             self.enemy_team.pod_2.calc_thrust())
        self.do_frame(f_move, e_move)
        return self.evaluate_position()

    pass


class MoveFinder(GeneticContinuousOptimizer):
    def __init__(self, map, game, emulator):
        self.map = map
        self.game = game
        self.emulator = emulator
        super().__init__(self.emulator.evaluate_move, BOUNDS, BITS_PER_VAR, AI_TIME_LIMIT, AI_TIME_TOLERANCE,
                         POPULATION_SIZE, R_CROSS, R_MUT)

    def optimize(self, best_score: float = None, tolerance: float = 1e-6) -> TeamMove:
        debug('optimizing moves starting at time', time()-t)
        z = super().optimize()
        return TeamMove(z[0])

    pass


m = Map()
debug('game map input at time', time()-t)
game = Game(m)
debug('game initialized at time', time()-t)
game.fetch()
print('{} {} 100'.format(game.friendly_team.pod_1.next_checkpoint.x, game.friendly_team.pod_1.next_checkpoint.y))
print('{} {} 100'.format(game.friendly_team.pod_2.next_checkpoint.x, game.friendly_team.pod_2.next_checkpoint.y))

while True:
    game.fetch()
    debug('received live game data at time', time()-t)

    emulator = Emulator(game)
    debug('made emulator at time', time()-t)

    move_finder = MoveFinder(m, game, emulator)
    debug('made move finder at time', time()-t)
    friendly_pod_positions = [game.friendly_team.pod_1.position,
                              game.friendly_team.pod_2.position]
    print(move_finder.optimize().get_printout(game.friendly_team))
    debug('printed moves at time', time()-t)
    t = time()
