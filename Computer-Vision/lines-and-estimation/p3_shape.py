from sys import argv


import numpy as np
import cv2
import matplotlib.pyplot as plt
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

    return np.array(pts)

# =============================================================================
if __name__ == "__main__":
    pts, tau, out_fig_name = arg_parse()
    x = pts[:, 0]
    y = pts[:, 1]

    # calculate some characteristics of the point set: min, max, average
    print("min: ({0:.3f}, {1:.3f})".format(np.min(x), np.min(y)))
    print("max: ({0:.3f}, {1:.3f})".format(np.max(x), np.max(y)))
    print("com: ({0:.3f}, {1:.3f})".format(np.average(x), np.average(y)))

    # find principle components to determine the vector along which the
    # data varies the least and the most (min and max axes)
    M = np.zeros((2,2), dtype=np.float64)
    for pt in pts:
        M += np.outer(pt, pt.T)

    eig_vals, eig_vectors = np.linalg.eig(M)

    print()
    print("Eigenvalues:", eig_vals)
    print("Eigenvector:", eig_vectors)

    i_max = np.argmax(eig_vals)
    i_min = np.argmin(eig_vals)
    vector_max = eig_vectors[:, i_max]
    vector_min = eig_vectors[:, i_min]
    print("Min eigenvector:", vector_min)
    print("Max eigenvector:", vector_max)
    print()

    s_min = np.sqrt(eig_vals[i_min])
    s_max = np.sqrt(eig_vals[i_max])

    print("min axis: ({0:.3f},{1:.3f}) sd {2:.3f}".format(vector_min[0], vector_min[1], s_min))
    print("max axis: ({0:.3f},{1:.3f}) sd {2:.3f}".format(vector_max[0], vector_max[1], s_max))

    # Least Squares
    # A = np.vstack([x, np.ones(len(x))]).T
    # slope, y_intercept = np.linalg.lstsq(A, y, rcond=None)[0]
    # print(slope,y_intercept)

    # plt.scatter(x, y, label='Data')
    # plt.plot(x, slope*x + y_intercept, 'r', label='Best fit line')
    # plt.legend()
    # plt.show()

    # determine closest point representation of the line
    theta = np.arccos(vector_min[0])
    theta2 = np.arcsin(vector_min[1])
    theta3 = np.arccos(.591)
    rho = 0
    print("Mine: ", theta)
    print("Check:", theta2)
    print("Ans:  ", theta3)

    #rho?
    print("closest point: rho {0:.3f} theta {1:.3f}".format(rho, theta))

    # determine the implicit form of the line
    a = np.cos(theta)
    b = np.sin(theta)
    c = -rho
    print("implicit: a {0:.3f} b {1:.3f} c {2:.3f}".format(a, b, c))

    if s_min < tau * s_max:
        print("best as line")
    else:
        print("best as ellipse")
