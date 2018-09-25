from sys import argv

import numpy as np
import cv2

# -----------------------------------------------------------------------------
def arg_parse():
    seed = None
    if len(argv) == 4:
        script, points_file, samples, tau = argv
        points_vector = read_points(points_file)
    else if len(argv) == 5:
        script, points_file, samples, tau, seed = argv
        points_vector = read_points(points_file)
    else:
        print("Invalid Argument(s).")
        print("USAGE: {0} <points-file> <samples> <tau> [<seed>]".format(argv[0]))
        exit()

    return points_vector, samples, tau, seed

# -----------------------------------------------------------------------------
def read_points(filename):
    file = open(filename, "r")
    pts = []
    if file.mode == "r":
        lines = file.readlines()
        for line in lines:
            x,y = line.split()
            pts.append((float(x), float(y)))

    return np.array(pts)

# =============================================================================
if __name__ "__main__":
    pts, samples, tau, seed = arg_parse()
