from sys import argv
import math as m

import numpy as np
import cv2

np.set_printoptions(precision=2)

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

    return gradient_magnitude, gradient_direction #, weight

# -----------------------------------------------------------------------------
def orientation_voting2(pt, weight, magnitude, direction, sigma):
    sigma_v = 2*sigma
    w = round(2*sigma_v)
    # print("2w + 1 =", 2*w+1)

    # extract the neighborhood around the point
    row, col = pt
    w_neighborhood = weight[row-w : row+w+1, col-w : col+w+1]
    print("Weight:\n", w_neighborhood)

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
    histogram, delimiters = np.histogram(d_neighborhood.flatten(), bins=range(-180,190,10))
    print("Histogram:\n",histogram, "\nDelimiters:\n", delimiters)
     
    # determine the distance between votes and 5 for linear interpolation
    # if votes == 0, then the weight is divided evenly between two bins
    votes = d_neighborhood % 10
    rounded = np.round(d_neighborhood, -1)
    print("\nBefore:\n", votes)
    votes = np.abs(np.abs(votes) - 5)
    print("\nAfter:\n", votes)
    bin_weight  = 1.0 - votes/5.0
    print("\nBin weight:\n", votes)
    print("\nRounded:\n", rounded)

    weighted_histogram1 = np.zeros(d_neighborhood.shape)
    less = np.where(d_neighborhood < rounded, True, False)
   
    weighted_histogram1 = w_neighborhood * bin_weight * less
    print(weighted_histogram1)


    histogram = np.zeros(35)
    # histogram[]





    # print(bin_size_rad)


# -----------------------------------------------------------------------------
def orientation_voting(pt, magnitude, direction, sigma):
    sigma_v = 2*sigma
    ksize = int(4*sigma_v+1)
    kernel = cv2.getGaussianKernel(ksize, sigma_v)
    kernel = kernel.T * kernel
    # print("Sum:", np.sum(kernel))
    # print(kernel)
    w = round(2*sigma_v)

    # extract the neighborhood around the point and calculate weight function
    row, col = pt
    weight = kernel * magnitude[row-w : row+w+1, col-w : col+w+1]
    d_neighborhood = direction[row-w : row+w+1, col-w : col+w+1]
    d_neighborhood *= (180.0 / m.pi)
    # print("Weight:\n", weight)
    # print("Direction Neighborhood:\n",d_neighborhood)


    histogram = []
    delimiters = range(-180,180,10)

    next_bin = 0.0
    for d in delimiters:
        histogram.append(next_bin)
        next_bin = 0.0


        bin_center = (d + d + 10) / 2
        index = np.where((d_neighborhood > d) & (d_neighborhood < d+10), 1, 0)
        
        # determine the weight that each bin gets
        dist_to_center = d_neighborhood - bin_center
        interpolation_weight = (1 - np.abs(dist_to_center)/5.0) * index

        before = np.where(dist_to_center < 0, 1, 0)
        after = np.where(dist_to_center > 0, 1, 0)

        histogram[-1] += np.sum(index * weight * interpolation_weight)

        if d != -180:
            histogram[-2] += np.sum(index * weight * (1-interpolation_weight) * before)

        next_bin = np.sum(index * weight * (1-interpolation_weight) * after)

    return histogram

# -----------------------------------------------------------------------------

# =============================================================================
if __name__ == "__main__":
    sigma, img, pts = arg_parse()
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY).astype(float)
    magnitude, direction = calculate_gradients(gray, sigma)

    for i, pt in enumerate(pts):
        h = orientation_voting(pt, magnitude, direction, sigma)
        h_smooth = smooth_histogram(h)
        
        print("\n Point {0}: ({1}, {2})".format(i, pt[0], pt[1]))
        print("Histograms:")
        for k,j in enumerate(range(-18,18)):
            print("[{0},{1}]: {2:.2f}".format(j*10, (j+1)*10, h[k], h_smooth[k]))
