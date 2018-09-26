from sys import argv
from math import sin, cos

import numpy as np
import cv2
import matplotlib.pyplot as plt


# -----------------------------------------------------------------------------
def arg_parse():
    seed = None
    if len(argv) == 4:
        script, points_file, samples, tau = argv
        points_vector = read_points(points_file)
    elif len(argv) == 5:
        script, points_file, samples, tau, seed = argv
        seed = int(seed)
        points_vector = read_points(points_file)
    else:
        print("Invalid Argument(s).")
        print("USAGE: {0} <points-file> <samples> <tau> [<seed>]".format(argv[0]))
        exit()

    return points_vector, int(samples), float(tau), seed

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
def fit_line(pt1, pt2):
    theta = np.arctan((pt1[0]-pt2[0]) / (pt2[1] - pt1[1]))
    rho = pt1[0] * np.cos(theta) + pt1[1] * np.sin(theta)

    return rho, theta


# =============================================================================
if __name__  == "__main__":
    pts, samples, tau, seed = arg_parse()
    x = pts[:, 0]
    y = pts[:, 1]

    if seed:
        np.random.seed(seed)

    N = len(pts)
    k_max = 0
    k = 0
    rho_model = 0
    theta_model = 0
    best_i, best_j = 0,1

    # generate n line samples and find which one is best
    for num in range(samples):
        i,j = np.random.randint(0, N, 2)

        # don't attempt to fit a line to a single point
        if i == j:
            continue

        # calculate rho and theta for the line between these two points
        rho, theta = fit_line(pts[i], pts[j])

        # calculate the distance between each point and the line
        error = (x*cos(theta) + y*sin(theta) - rho)**2
        # print(error)

        # only keep the ones that are less than the tolerance
        error = error[error < tau**2]
        k = len(error)

        # check if we have a new best fit
        if k > k_max:
            # save this fit
            best_i, best_j = i,j
            k_max = k
            rho_model = rho
            theta_model = theta

            # output statistics about this fit
            print("Sample {0}:".format(num))
            print("indices ({0},{1})".format(i, j))
            if rho < 0:
                print("line ({0:.3f},{1:.3f},{2:.3f})".format(-cos(theta), -sin(theta), rho))
            else:
                print("line ({0:.3f},{1:.3f},{2:.3f})".format(cos(theta), sin(theta), -rho))
            print("inliers", k)
            print()

            # plt.scatter(x, y)
            # plt.scatter(pts[best_i, 0], pts[best_i, 1], color='r')
            # plt.scatter(pts[best_j, 0], pts[best_j, 1], color='r')
            # plt.plot(x, (rho_model - x*cos(theta_model))/sin(theta_model))
            # plt.show()

    # calculate inlier and outlier average distances
    distances = abs(x*cos(theta_model) + y*sin(theta_model) - rho_model)
    error = distances**2
    inliers = distances[error < tau**2]
    outliers = distances[error >= tau**2]

    print("avg inlier dist {0:.3f}".format(np.average(inliers)))
    print("avg outlier dist {0:.3f}".format(np.average(outliers)))

    # display
    # plt.scatter(x, y)
    # plt.scatter(pts[best_i, 0], pts[best_i, 1], color='r')
    # plt.scatter(pts[best_j, 0], pts[best_j, 1], color='r')
    # plt.plot(x, (rho_model - x*cos(theta_model))/sin(theta_model))
    # plt.show()