from sys import argv
import math as m

import numpy as np
import cv2


# -----------------------------------------------------------------------------
def arg_parse():
    if len(argv) == 4:
        script, sigma, img_name, points_file = argv
        pts = np.loadtxt(points_file, np.int32)
        img = cv2.imread(img_name, cv2.IMREAD_COLOR)
        return float(sigma), img, pts

    else:
        print("Invalid Argument(s).")
        print("USAGE: {0} <sigma> <img> <points-file>")
        exit()

# -----------------------------------------------------------------------------
def calculate_gradients(img, sigma=1.0):
    ksize = (int(4*sigma+1), int(4*sigma+1))
    blurred = cv2.GaussianBlur(img, ksize, sigma)

    # x and y derivatives
    kx,ky = cv2.getDerivKernels(1,1,3)
    kx = np.transpose(kx/2)
    ky = ky/2
    x_gradient = cv2.filter2D(blurred, -1, kx)
    y_gradient = cv2.filter2D(blurred, -1, ky)

    gradient_magnitude = np.sqrt(x_gradient.astype(np.float64)**2 + y_gradient**2) # np.hypot()
    gradient_direction = np.arctan2(y_gradient, x_gradient)

    print("Maximum direction:",np.max(gradient_direction))
    print("Minimum direction:",np.min(gradient_direction))

    # compute the weight function 
    # i.e. smooth the magnitude with sigma_v == 2 * sigma
    sigma_v = 2*sigma
    ksize = (int(4*sigma_v+1), int(4*sigma_v+1))
    weight = cv2.GaussianBlur(gradient_magnitude, ksize, sigma_v)

    return gradient_magnitude, gradient_direction, weight

# -----------------------------------------------------------------------------
def orientation_voting(pt, weight, magnitude, direction, sigma):
    sigma_v = 2*sigma
    w = round(2*sigma_v)
    # print("2w + 1 =", 2*w+1)

    # extract the neighborhood around the point
    row, col = pt
    w_neighborhood = weight[row-w : row+w+1, col-w : col+w+1]
    d_neighborhood = direction[row-w : row+w+1, col-w : col+w+1]
    # print(w_neighborhood.shape)
    # print(w_neighborhood)
    # print(d_neighborhood.shape)
    np.set_printoptions(precision=2)
    print("Direction Neighborhood:\n",d_neighborhood)

    # convert to degrees
    deg = 180.0 / m.pi
    d_neighborhood *= deg
    print("Votes:\n", d_neighborhood)
     
    # determine the distance between inter and 5 for linear interpolation
    # if votes == 0, then the weight is divided evenly between two bins
    votes = d_neighborhood % 10
    rounded = np.round(d_neighborhood, -1)
    print("\nBefore:\n", votes)
    votes = np.abs(np.abs(votes) - 5)
    print("\nAfter:\n", votes)
    bin_weight  = 1.0 - votes/5.0
    print("\nAfter2:\n", votes)
    print("\nRounded:\n", rounded)





    # print(bin_size_rad)


# =============================================================================
if __name__ == "__main__":
    sigma, img, pts = arg_parse()
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY).astype(float)
    magnitude, direction, weight = calculate_gradients(gray, sigma)
    print(pts)

    for i, pt in enumerate(pts):
        histogram = orientation_voting(pt, weight, magnitude, direction, sigma)
        
        print("Point {0}: ({1}, {2})".format(i, pt[0], pt[1]))
        print("Histograms:")
        # for j in range(-18,18):
            # print("[{},{}]: ".format(j*10, (j+1)*10))
        break