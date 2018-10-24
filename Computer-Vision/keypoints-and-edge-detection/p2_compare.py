from sys import argv
import ntpath
# import matplotlib.pyplot as plt
import math as m

import numpy as np
import cv2

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
    # trace^2?
    kappa = 0.04
    determinant = dx_squared*dy_squared - dx_dy*dx_dy
    trace = dx_squared + dy_squared
    img_harris = determinant - kappa*trace*trace

    # grayscale normalization
    img_harris = 255 * (img_harris - np.min(img_harris)) /  \
                        (np.max(img_harris) - np.min(img_harris))

    # cv2.imwrite("Harris.png", img_harris)
    # plot_pics( [im, im_harris], 1, ['Original', 'Harris'] )

    return img_harris

# -----------------------------------------------------------------------------
def extract_harris_keypoints(original, img_harris, sigma, img_name, ext,
                             apply_threshold=False):
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
    # img_harris = img_harris.astype(np.uint8)
    dilated = cv2.dilate(img_harris, kernel)

    # Comparing the dilated image to the Harris image will preserve only those
    # locations that are peaks (non-maximum suppression)
    # harris_peaks = img_harris >= dilated
    harris_peaks = cv2.compare(img_harris, dilated, cv2.CMP_GE)

    # Multiplying by the thresholded image keeps only peaks above the threshold
    # or multiply by the original to keep all peaks
    if apply_threshold:
        harris_peaks = harris_peaks * thresholded
    else:
        harris_peaks = harris_peaks * img_harris

    # grayscale normalization
    harris_peaks = 255 * (harris_peaks - np.min(harris_peaks)) / \
                    (np.max(harris_peaks) - np.min(harris_peaks))

    # Extract all indices of the peaks and build keypoint list
    ys, xs = np.where(harris_peaks > 0)

    # TODO ask question about harris keypoint size
    kp_size = 2*sigma
    harris_keypoints = [
        cv2.KeyPoint(xs[i], ys[i], kp_size, _response=harris_peaks[ys[i]][xs[i]])
        for i in range(len(xs))
    ]

    # sort by keypoint response, largest first
    harris_keypoints.sort( key = lambda k: k.response)
    harris_keypoints = harris_keypoints[::-1]

    index = min(len(harris_keypoints), 200)
    out_img = cv2.drawKeypoints(original.astype(np.uint8), harris_keypoints[:index], None)
    cv2.imwrite(img_name + "_harris." + ext, out_img)


    print("\nTop 10 Harris keypoints:")
    for i,k in enumerate(harris_keypoints[:10]):
        print("{0}: ({1:.2f}, {2:.2f}) {3:.4f} {4:.2f}".format(i, k.pt[0], 
                                                        k.pt[1], k.response, 
                                                        k.size))

    return harris_keypoints

# -----------------------------------------------------------------------------
def extract_SIFT_keypoints(img, sigma, img_name, ext):
    sift = cv2.xfeatures2d.SIFT_create()
    kp = sift.detect(img, None)

    kp.sort( key = lambda k: k.response)
    kp = kp[::-1]
    out_img = cv2.drawKeypoints(img.astype(np.uint8), kp, None)

    '''
    The SIFT result includes many keypoints at the same location, but with 
    different orientations. This filters out the keypoints down to just one
    per location. In matching keypoints between images, this filtering is not 
    a good idea because each orientation will create a different descriptor.
    '''
    kp_unique = []
    for k in kp[1:]:
        if (not kp_unique or k.pt != kp_unique[-1].pt) and k.size <= 3*sigma:
            kp_unique.append( k )

    index = min(len(kp_unique), 200)
    out_img = cv2.drawKeypoints(img.astype(np.uint8), kp_unique[:index], None)
    cv2.imwrite(img_name + "_sift." + ext, out_img)


    print("\nTop 10 SIFT keypoints:")
    for i, k in enumerate(kp_unique[:10]):
        print("{0}: ({1:.2f}, {2:.2f}) {3:.4f} {4:.2f}".format(i, k.pt[0], 
                                                        k.pt[1], k.response, 
                                                        k.size))
    return kp_unique


# -----------------------------------------------------------------------------
def keypoint_match(set1, set2, set1_name, set2_name):
    last_set1 = min(len(set1), 100)
    last_set2 = min(len(set2), 200)

    rank_differences = []
    dist_differences = []

    # for each set1 keypoint in the top 100, find the closest set2
    # keypoint in the top 200
    for i in range(last_set1):
        min_distance = 1e6
        min_rank = 1e6
        for j in range(last_set2):
            distance = m.sqrt((set1[i].pt[0] - set2[j].pt[0])**2 + \
                              (set1[i].pt[1] - set2[j].pt[1])**2)

            if distance < min_distance:
                min_distance = distance
                min_rank = abs(i-j)

        dist_differences.append(min_distance)
        rank_differences.append(min_rank)


    med_dist = np.median(dist_differences)
    avg_dist = np.mean(dist_differences)
    med_rank = np.median(rank_differences)
    avg_rank = np.mean(rank_differences)

    print("\n{0} keypoint to {1} distances:".format(set1_name, set2_name))
    print("num_from", last_set1, "num_to", last_set2)
    print("Median distance: {0:.1f}".format(med_dist))
    print("Average distance: {0:.1f}".format(avg_dist))
    print("Median index difference: {0:.1f}".format(med_rank))
    print("Average index difference: {0:.1f}".format(avg_rank))


# =============================================================================
if __name__ == "__main__":
    sigma, gray, img_name, ext = arg_parse()

    img_harris = harris_measure(gray, sigma)
    harris_keypoints = extract_harris_keypoints(gray, img_harris, sigma, img_name, ext)
    SIFT_keypoints = extract_SIFT_keypoints(gray, sigma, img_name, ext)
    keypoint_match(harris_keypoints, SIFT_keypoints, "Harris", "SIFT")
    keypoint_match(SIFT_keypoints, harris_keypoints, "SIFT", "Harris")