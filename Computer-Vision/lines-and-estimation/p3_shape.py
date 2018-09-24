from sys import argv

import numpy as np
import cv2

# -----------------------------------------------------------------------------
def arg_parse():
    if len(argv) == 4:
        script, points_file, tau, out_fig_name = argv
        points_vector = read_points(points_file)
        return points_vector, tau, out_fig_name

    else:
        print("Invalid Argument(s).")
        print("USAGE: {0} <points-txt-file> <tau> <out-fig-name>")
        exit()

# -----------------------------------------------------------------------------
def read_points(filename):
    file = open(filename, "r")
    pts = []
    if file.mode == "r":
        lines = file.readlines()
        for line in lines:
            x,y = line.split()
            pts.append((float(x), float(y)))

    return pts

# =============================================================================
if __name__ == "__main__":
    points, tau, out_fig_name = arg_parse()
    print(points)
