from sys import argv
import math as m

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

    R = np.array([float(i) for i in R])
    T = np.array([[float(i) for i in T]])

    return R, T, float(f), float(d), float(ic), float(jc)


# -----------------------------------------------------------------------------
def read_points(filename):
    points = np.loadtxt(filename)
    return points

# -----------------------------------------------------------------------------
def get_camera_matrix(R, T, f, d, ic, jc):

    # rotations in each direction
    Rx = np.array([[1.,           0.,            0.],
                   [0.,  m.cos(R[0]),  -m.sin(R[0])],
                   [0.,  m.sin(R[0]),   m.cos(R[0])]])

    Ry = np.array([[ m.cos(R[1]),  0.,  m.sin(R[1])],
                   [          0.,  1.,           0.],
                   [-m.sin(R[1]),  0.,  m.cos(R[1])]])

    Rz = np.array([[m.cos(R[2]),  -m.sin(R[2]),  0.],
                   [m.sin(R[2]),   m.cos(R[2]),  0.],
                   [         0.,            0.,  1.]])

    # rotation matrix
    Rot = Rx @ Ry @ Rz


    # pixels are square of size 'd'
    sx = f/d
    sy = f/d
    K = np.array([[sx,  0.,  ic],
                  [0.,  sy,  jc],
                  [0.,  0.,  1.]])





    print(Rot)
    print(T)
    Rt = np.hstack((Rot, T.T))
    print(Rt)




    print(Rx)
    print(Ry)
    print(Rz)


    return K @ Rt


# =============================================================================
if __name__ == "__main__":
    points_vector, R, T, f, d, ic, jc = arg_parse()
    M = get_camera_matrix(R, T, f, d, ic, jc)
    print("Matrix M:")
    print(M)