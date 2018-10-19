from sys import argv
import ntpath
import matplotlib.pyplot as plt
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
        img = cv2.imread(path_to_img, cv2.IMREAD_COLOR)

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
    im_harris /= np.max(im_harris)
    im_harris *= 255
    cv2.imwrite("Harris.png", img_harris)
    # plot_pics( [im, im_harris], 1, ['Original', 'Harris'] )


# =============================================================================
if __name__ == "__main__":
    sigma, img, img_name, ext = arg_parse()
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

    harris_img = harris_measure(gray, sigma)





    sift = cv2.xfeatures2d.SIFT_create()
