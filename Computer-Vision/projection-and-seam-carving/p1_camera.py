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

    # convert to flaots and convert rotations to radians
    R = np.array([float(i)*m.pi/180. for i in R])
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


    # pixels are square of size 'd' microns - convert to millimeters
    sx = f/(d * 1e-3)
    sy = f/(d * 1e-3)
    K = np.array([[sx,  0.,  ic],
                  [0.,  sy,  jc],
                  [0.,  0.,  1.]])

    # concatenate rotation and translation
    Rt = np.hstack((Rot.T, -Rot.T @ T.T))

    # multiply to form camera matrix
    M = K @ Rt

    return M

# -----------------------------------------------------------------------------
def project_points(points, M, ic, jc):
    visible = []
    hidden = []

    print("Projections:")
    for i,pt in enumerate(points):
        print("{0}: {1:.1f} {2:.1f} {3:.1f} ".format(i, pt[0], pt[1], pt[2]), end='')
        
        # convert to homogenous
        pt = np.append(pt, 1.0)

        # project with camera matrix and convert back to affine
        warp = M @ pt
        row = warp[1]/warp[2]
        col = warp[0]/warp[2]

        # convert back to affine
        print("=> {0:.1f} {1:.1f} ".format(row, col), end='')

        # check if inside the image (4000 x 6000 image)
        if row >= 0 and row < 4000 and col >= 0 and col < 6000:
            print("inside")
        else:
            print("outside")

        # check if point in front of or behind the camera (z positive or negative)
        if warp[2] < 0:
            hidden.append(i)
        else:
            visible.append(i)

    print("visible:", end='')
    for pt in visible:
        print(" {0}".format(pt), end='')

    print("\nhidden:", end='')
    for pt in hidden:
        print(" {0}".format(pt), end='')
    print()

# -----------------------------------------------------------------------------
def print_matrix(M):
    for row in M:
        for element in row[:len(row)-1]:
            print("{0:.2f}, ".format(element), end='')
        print("{0:.2f}".format(row[-1]))

# =============================================================================
if __name__ == "__main__":
    points, R, T, f, d, ic, jc = arg_parse()
    M = get_camera_matrix(R, T, f, d, ic, jc)
    print("Matrix M:")
    print_matrix(M)

    project_points(points, M, ic, jc)

