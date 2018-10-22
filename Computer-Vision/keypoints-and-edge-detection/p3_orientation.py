from sys import argv

import numpy as np
import cv2


# -----------------------------------------------------------------------------
def arg_parse():
    if len(argv) == 4:
        script, sigma, img_name, points_file = argv
        pts = np.loadtxt(points_file, np.int32)
        img = cv2.imread(img_name, cv2.IMREAD_GRAYSCALE)
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
    gradient_direction = np.arctan2(y_gradient.astype(np.float64), x_gradient)

    # compute the weight function 
    # i.e. smooth the magnitude with sigma_v == 2 * sigma
    sigma_v = 2*sigma
    ksize = (int(4*sigma_v+1), int(4*sigma_v+1))
    weight = cv2.GaussianBlur(gradient_magnitude, ksize, sigma_v)

    return gradient_magnitude, gradient_direction, weight

# -----------------------------------------------------------------------------
def orientation_voting(weight, magnitude, direction, sigma):
    pass
    # separate the weight into the two orientation bins that it is closest
    # to using linear interpolation


# =============================================================================
if __name__ == "__main__":
    sigma, gray, pts = arg_parse()
    magnitude, direction, weight = calculate_gradients(gray, sigma)
    print(pts)

    for pt in pts:
        histogram = orientation_voting(weight, magnitude, direction, sigma)