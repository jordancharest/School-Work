from sys import argv
from math import sin, cos

import numpy as np
import cv2
# import matplotlib.pyplot as plt
# -----------------------------------------------------------------------------
def arg_parse():
    if len(argv) == 4:
        script, points_file, tau, out_fig_name = argv
        points_vector = read_points(points_file)
        return points_vector, float(tau), out_fig_name

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

    return np.array(pts)

# -----------------------------------------------------------------------------
def print_characteristics(com, x, y):
    print("min: ({0:.3f},{1:.3f})".format(np.min(x), np.min(y)))
    print("max: ({0:.3f},{1:.3f})".format(np.max(x), np.max(y)))
    print("com: ({0:.3f},{1:.3f})".format(com[0], com[1]))

# -----------------------------------------------------------------------------
def find_min_and_max_axes(pts):    
    # find principle components to determine the vector along which the
    # data varies the least and the most (min and max axes)
    # the sum of outer products is the same as the matrix multiplication of pts.T * pts
    M = np.dot(pts.T, pts)
    eig_vals, eig_vectors = np.linalg.eig(M)

    # normalize for the number of points we have
    eig_vals /= len(pts)

    # display min and max axes
    i_max = np.argmax(eig_vals)
    i_min = np.argmin(eig_vals)
    vec_max = eig_vectors[:, i_max]
    vec_min = eig_vectors[:, i_min]
    if vec_min[0] < 0:
        vec_min = -vec_min
    if vec_max[0] < 0:
        vec_max = -vec_max

    # standard deviations are the sqrt of the eignevalues
    s_min = np.sqrt(eig_vals[i_min])
    s_max = np.sqrt(eig_vals[i_max])

    print("min axis: ({0:.3f},{1:.3f}), sd {2:.3f}".format(vec_min[0], vec_min[1], s_min))
    print("max axis: ({0:.3f},{1:.3f}), sd {2:.3f}".format(vec_max[0], vec_max[1], s_max))

    return vec_min, s_min, vec_max, s_max


# =============================================================================
if __name__ == "__main__":
    original_pts, tau, out_fig_name = arg_parse()
    pts = np.copy(original_pts)
    x = pts[:, 0]
    y = pts[:, 1]

    # calculate some characteristics of the point set: min, max, average
    com = np.array((np.average(x), np.average(y)))
    print_characteristics(com, x, y)

    # center the data around the center of mass, then determine axes of 
    # smallest and largest variance
    pts -= com
    vector_min, s_min, vector_max, s_max = find_min_and_max_axes(pts)

    # determine closest point representation of the line
    # since we centered the data around the center of mass, we know that
    # that point is on the line
    theta = np.arccos(vector_min[0])
    rho = com[0] * np.cos(theta) + com[1] * np.sin(theta)
    print("closest point: rho {0:.3f}, theta {1:.3f}".format(rho, theta))

    # determine the implicit (standard) form of the line
    a = np.cos(theta)
    b = np.sin(theta)
    c = -rho
    print("implicit: a {0:.3f}, b {1:.3f}, c {2:.3f}".format(a, b, c))

    # determine shape of best fit
    if s_min < tau * s_max:
        print("best as line")
    else:
        print("best as ellipse")

    # plot and save
    # x0 = original_pts[:,0]
    # y0 = original_pts[:,1]
    # plt.scatter(x0, y0, color='b', label='Data')
    # plt.scatter(com[0], com[1], color='r', label='CoM')
    # plt.plot(x0, (rho - x0*cos(theta))/sin(theta), color='k', label='Best Fit')
    # plt.legend()
    # plt.savefig(out_fig_name)
    # plt.show()