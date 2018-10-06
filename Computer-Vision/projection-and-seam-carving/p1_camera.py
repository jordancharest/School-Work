from sys import argv

import numpy as np

# -----------------------------------------------------------------------------
def arg_parse():
    if len(argv) == 3:
        script, params_file, points_file = argv
        R, T, f, d, ic, jc = read_params(params_file)
        points_vector = read_points(points_file)
    else:
        print("Invalid Argument(s).")
        print("USAGE: {0} <params-file> <points-file>".format(argv[0]))
        exit()

    return points_vector, R, T, f, d, ic, jc

# -----------------------------------------------------------------------------
def read_params(filename):
    file = open(filename, "r")
    if file.mode == "r":
        lines = file.readlines()
        R = lines[0].split()
        T = lines[1].split()
        f, d, ic, jc = lines[2].split()
    else:
        print("open() failed")
        exit()

    R = [float(i) for i in R]
    T = [float(i) for i in T]

    return R, T, float(f), float(d), float(ic), float(jc)


# -----------------------------------------------------------------------------
def read_points(filename):
    points = np.loadtxt(filename)
    return points

# -----------------------------------------------------------------------------
def get_camera_matrix(R, T, f, d, ic, jc):
    C = np.ones((3,4))
    print(C)


# =============================================================================
if __name__ == "__main__":
    points_vector, R, T, f, d, ic, jc = arg_parse()
    C = get_camera_matrix(R, T, f, d, ic, jc)