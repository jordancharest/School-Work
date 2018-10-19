from sys import argv
import ntpath
import matplotlib.pyplot as plt
import math as m

import numpy as np
import cv2

# TODO:
# output the harris and SIFT keypoints on gray image with correct filename,
# e.g. wisconsin_SIFT.jpg, wisconsin_harris.jpg




# -----------------------------------------------------------------------------
def arg_parse():
    if len(argv) == 3:
        script, sigma, path_to_img = argv

        # extract image name and read it
        _, filename = ntpath.split(path_to_img)
        filename, ext = filename.split(".")
        img = cv2.imread(path_to_img, cv2.IMREAD_GRAYSCALE)

        return float(sigma), img, filename, ext

    else:
        print("Invalid Argument(s).")
        print("USAGE: {} <sigma> <image>".format(argv[0]))
        exit()

# -----------------------------------------------------------------------------
def plot_pics(image_list, num_cols=2, title_list=[]):
    if len(image_list) == 0: return
    if len(image_list[0].shape) == 2:
        plt.gray()
    num_rows = int( m.ceil(len(image_list)/float(num_cols)))

    if num_cols > 2:
        plt.figure(figsize=(10,10))
    else:
        plt.figure(figsize=(15,15))

    for i in range(len(image_list)):
        im = image_list[i]
        print(num_rows, num_cols, i+1)
        plt.subplot(num_rows, num_cols, i+1)
        plt.imshow(im)
        if i < len(title_list):
            plt.title(title_list[i])
        plt.xticks([]), plt.yticks([])

    plt.show()


# -----------------------------------------------------------------------------
def harris_measure(img, sigma):
    ksize = (int(4*sigma+1), int(4*sigma+1))
    smoothed = cv2.GaussianBlur(img.astype(np.float64), ksize, sigma)

    # Compute x and y derivatives
    kx,ky = cv2.getDerivKernels(1,1,3)
    kx = np.transpose(kx/2)
    ky = ky/2
    dx = cv2.filter2D(smoothed, -1, kx)
    dy = cv2.filter2D(smoothed, -1, ky)

    # Compute components of the outer product
    dx_squared = dx * dx
    dy_squared = dy * dy
    dx_dy = dx * dy

    # Convolution of the outer product with the Gaussian kernel
    # gives the summed values desired
    h_sigma = 2*sigma
    h_ksize = (int(4*h_sigma+1), int(4*h_sigma+1))
    dx_squared = cv2.GaussianBlur(dx_squared, h_ksize, h_sigma)
    dy_squared = cv2.GaussianBlur(dy_squared, h_ksize, h_sigma)
    dx_dy = cv2.GaussianBlur(dx_dy, h_ksize, h_sigma)

    # Compute the Harris measure
    kappa = 0.004
    determinant = dx_squared*dy_squared - dx_dy*dx_dy
    trace = dx_squared + dy_squared
    img_harris = determinant - kappa*trace*trace

    # grayscale normalization
    img_harris = 255 * (img_harris - np.min(img_harris)) / (np.max(img_harris) - np.min(img_harris))

    cv2.imwrite("Harris.png", img_harris)
    # plot_pics( [im, im_harris], 1, ['Original', 'Harris'] )

    return img_harris

# -----------------------------------------------------------------------------
def extract_harris_keypoints(original, img_harris, sigma, apply_threshold=False):
    # Compute a thresholded one the Harris measure by extracting the Harris 
    # image intensities, sorting them, and setting the threshold so that a 
    # small percentage of the points could pass
    values = np.sort(img_harris,axis=None)
    threshold_frac = 0.25
    threshold_index = int((1-threshold_frac)*len(values))
    threshold = values[threshold_index]
    rv,thresholded = cv2.threshold(img_harris.astype(np.uint8),threshold,1,cv2.THRESH_BINARY)

    # form structuring kernel and dilate
    max_dist = int(2*sigma)
    kernel = np.ones((2*max_dist+1, 2*max_dist+1), np.uint8)
    img_harris = img_harris.astype(np.uint8)
    dilated = cv2.dilate(img_harris, kernel)

    # Comparing the dilated image to the Harris image will preserve only those
    # locations that are peaks (non-maximum suppression)
    harris_peaks = img_harris >= dilated

    # Multiplying by the thresholded image keeps only peaks above the threshold
    # or multiply by the original to keep all peaks
    if apply_threshold:
        harris_peaks = harris_peaks * thresholded
    else:
        harris_peaks = harris_peaks * img_harris

    # Extract all indices of the peaks and build keypoint list
    ys, xs = np.where(harris_peaks > 0)
    kp_size = 4*sigma
    harris_keypoints = [
        cv2.KeyPoint(xs[i], ys[i], kp_size)
        for i in range(len(xs))
    ]

    print("We have", len(harris_keypoints), "keypoints.")

    out_img = cv2.drawKeypoints(original.astype(np.uint8), harris_keypoints, None)
    cv2.imwrite("Keypoints.jpg", out_img)
    # plot_pics( [img_harris, out_img], 1, ['Harris Measure', 'Keypoints shown'] )

    return harris_keypoints

# -----------------------------------------------------------------------------
def extract_SIFT_keypoints(img,):
    sift = cv2.xfeatures2d.SIFT_create()
    kp = sift.detect(img, None)

    print(type(kp))
    print(len(kp))
    kp.sort( key = lambda k: k.response)
    kp = kp[::-1]
    out_img = cv2.drawKeypoints(img.astype(np.uint8), kp, None)
    # ipu.show_with_pixel_values(out_img)

    '''
    The SIFT result includes many keypoints at the same location, but with different orientations.
    This filters out the keypoints down to just one per location.  In matching keypoints between
    images, this filtering is not a good idea because each orientation will create a different
    descriptor.
    '''
    kp_unique = [kp[0]]
    for k in kp[1:]:
        if k.pt != kp_unique[-1].pt:
            kp_unique.append( k )

    print(len(kp_unique))
    # for k in kp_unique:
        # print(k.response, k.pt, k.angle, k.size)



# =============================================================================
if __name__ == "__main__":
    sigma, gray, img_name, ext = arg_parse()

    img_harris = harris_measure(gray, sigma)
    harris_keypoints = extract_harris_keypoints(gray, img_harris, sigma)
    extract_SIFT_keypoints(gray)



