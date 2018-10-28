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
        img = cv2.imread(img_name, cv2.IMREAD_GRAYSCALE).astype(np.float64)
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

    return gradient_magnitude, gradient_direction

# -----------------------------------------------------------------------------
def orientation_voting(pt, magnitude, direction, sigma):
    sigma_v = 2*sigma
    ksize = int(4*sigma_v+1)
    kernel = cv2.getGaussianKernel(ksize, sigma_v)
    kernel = kernel * kernel.T
    w = round(2*sigma_v)

    # extract the neighborhood around the point and calculate weight function
    row, col = pt
    weight = kernel * magnitude[row-w : row+w+1, col-w : col+w+1]

    d_neighborhood = direction[row-w : row+w+1, col-w : col+w+1]
    d_neighborhood *= (180.0 / m.pi)

    histogram = []
    delimiters = range(-180,180,10)

    next_bin = 0.0
    peaks = []

    # fill each bin
    for i, d in enumerate(delimiters):
        histogram.append(next_bin)
        next_bin = 0.0

        # we only care about the indexes that have values within the current bins
        bin_center = (d + d + 10) / 2
        index = np.where((d_neighborhood > d) & (d_neighborhood < d+10), 1, 0)
        
        # determine the weight that each bin gets
        dist_to_center = d_neighborhood - bin_center
        interpolation_weight = (1 - np.abs(dist_to_center)/10.0) * index

        # interpolated weight for the adjacent bins
        before = np.where(dist_to_center < 0, 1, 0)
        after = np.where(dist_to_center > 0, 1, 0)

        # add weight to the current bin
        histogram[-1] += np.sum(index * weight * interpolation_weight)

        # add to the previous bin, handle -180/180 in a special way to wrap around
        if d != -180:
            histogram[-2] += np.sum(index * weight * (1-interpolation_weight) * before)
        else:
            wrap = np.sum(index * weight * (1-interpolation_weight) * before)

        # save to add to the next bin
        next_bin = np.sum(index * weight * (1-interpolation_weight) * after)

        # generate a list of potential peak locations
        max_index = np.argmax(index * weight * interpolation_weight)
        theta = d_neighborhood.ravel()[max_index]
        value = weight.ravel()[max_index]
        peaks.append(theta)

    # add the weights that have wrapped around
    histogram[-1] += wrap
    histogram[0]  += next_bin

    return histogram, peaks

# -----------------------------------------------------------------------------
def smooth_histogram(hist):
    # mod by length of histogram so that the first and last values are
    # wrapped around
    smooth = []
    peaks = []
    for i in range(len(hist)):
        avg = (((hist[(i-1) % len(hist)] + hist[(i+1) % len(hist)]) / 2.0) + hist[i]) / 2.0
        smooth.append(avg)

    # keep track of peak locations
    for i in range(len(smooth)):
        if smooth[i] > smooth[(i-1) % len(smooth)] and \
            smooth[i] > smooth[(i+1) % len(smooth)] and smooth[i] >= 0.1:
            peaks.append(i)

    return smooth, peaks

# -----------------------------------------------------------------------------
def find_peaks(h, h_smooth, peak_vals, peak_locations):
    peaks = []

    for loc in peak_locations:
        peaks.append((h_smooth[loc], peak_vals[loc]))

    peaks.sort()
    peaks = peaks[::-1]
    max_val = peaks[0][0]
    strong_peaks = 0

    for i in range(len(peaks)):
        print("Peak {0}: theta {1:.1f}, value {2:.2f}".format(i, peaks[i][1], 
                                                                peaks[i][0]))
        if peaks[i][0] >= 0.8 * max_val:
            strong_peaks += 1

    print("Number of strong orientation peaks:", strong_peaks)


# =============================================================================
if __name__ == "__main__":
    sigma, gray, pts = arg_parse()
    # gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY).astype(float)
    magnitude, direction = calculate_gradients(gray, sigma)

    for i, pt in enumerate(pts):
        h, peak_vals = orientation_voting(pt, magnitude, direction, sigma)
        h_smooth, peak_locations = smooth_histogram(h)
        
        print("\n Point {0}: ({1},{2})".format(i, pt[0], pt[1]))
        print("Histograms:")
        step = 10
        for k,j in enumerate(range(-180,180,step)):
            print("[{0},{1}]: {2:.2f} {3:.2f}".format(j, j+step, h[k], h_smooth[k]))

        peaks = find_peaks(h, h_smooth, peak_vals, peak_locations)
