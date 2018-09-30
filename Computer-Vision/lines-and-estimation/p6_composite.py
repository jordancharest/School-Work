from sys import argv
import os
from math import floor, sqrt

import cv2
import numpy as np

# -----------------------------------------------------------------------------
def arg_parse():
    if len(argv) == 5:
        script, directory, out_img, sigma, p = argv
        img_list = os.listdir(directory)
        img_list = [directory +  "/" + name for name in img_list if ".jpg" in name.lower()]
        return img_list, out_img, abs(float(sigma)), abs(float(p))
    else:
        print("Invalid Argument(s):", len(argv))
        print("USAGE: {0} <directory> <out-img-name> <sigma> <p>".format(argv[0]))
        exit()

# -----------------------------------------------------------------------------
def output(M, N, location_list, img):
    locations = ((M//4, N//4), (M//4, 3*N//4), (3*M//4, N//4), (3*M//4, 3*N//4))

    for i,(m,n) in enumerate(locations):
        print("\nEnergies at ({0}, {1})".format(m, n))
        for j, energy in enumerate(location_list[i]):
            print("{0}: {1:.1f}".format(j, energy))
        print("RGB: ({0:.0f}, {1:.0f}, {2:.0f})".format(img[m, n, 2], img[m, n, 1], img[m, n, 0]))

# =============================================================================
if __name__ == "__main__":
    img_list, out_img_name, sigma, p = arg_parse()
    h = floor(2.5  * sigma)
    ksize = 2 * h + 1
    img_list.sort()
    
    img = cv2.imread(img_list[0], cv2.IMREAD_COLOR)
    energy_sum = np.zeros(img.shape[0:2], dtype=np.float64)
    weighted_imgs = np.zeros(img.shape, dtype=np.float64)

    print("Results:")
    top_left, top_right, bottom_left, bottom_right = [], [], [], []
    M, N, _ = img.shape

    # for each image in the directory
    for img_name in img_list:
        img = cv2.imread(img_name, cv2.IMREAD_COLOR)
        gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

        # calculate the gradients
        btype = cv2.BORDER_DEFAULT
        sobelx = cv2.Sobel(gray, cv2.CV_64F, 1, 0, ksize=3, borderType=btype)
        sobely = cv2.Sobel(gray, cv2.CV_64F, 0, 1, ksize=3, borderType=btype)
        gradient = sobelx**2 + sobely**2

        # calculate energy for each pixel
        energy = cv2.GaussianBlur(gradient, (ksize, ksize), sigma, borderType=btype)

        # avoid divide by zero later
        energy[energy < 0.000001] = 0.000001

        # save for the output
        top_left.append(energy[M//4, N//4])
        top_right.append(energy[M//4, 3*N//4])
        bottom_left.append(energy[3*M//4, N//4])
        bottom_right.append(energy[3*M//4, 3*N//4])

        # calculate running sum of the weighted energy of each image
        weighted_imgs += energy[:, :, None]**p * img
        energy_sum += energy**p

    # normalize
    result = weighted_imgs / energy_sum[:, :, None]

    # output results
    output(M, N, (top_left, top_right, bottom_left, bottom_right), result)
    cv2.imwrite(out_img_name, result)
    print("Wrote", out_img_name)