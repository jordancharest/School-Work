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
        print(len(argv))
        print("Invalid Argument(s).")
        print("USAGE: {0} <directory> <out-img-name> <sigma> <p>".format(argv[0]))
        exit()

# -----------------------------------------------------------------------------
def output(M, N, top_left, top_right, bottom_left, bottom_right, img):
    print("Energies at ({0}, {1})".format(M//4, N//4))
    for i, energy in enumerate(top_left):
        print("{0}: {1:.1f}".format(i, energy))
    print("RGB: ({0:.0f}, {1:.0f}, {2:.0f})\n".format(img[M//4, N//4, 2], img[M//4, N//4, 1], img[M//4, N//4, 0]))

    print("Energies at ({0}, {1})".format(M//4, 3*N//4))
    for i, energy in enumerate(top_right):
        print("{0}: {1:.1f}".format(i, energy))
    print("RGB: ({0:.0f}, {1:.0f}, {2:.0f})\n".format(img[M//4, 3*N//4, 2], img[M//4, 3*N//4, 1], img[M//4, 3*N//4, 0]))


    print("Energies at ({0}, {1})".format(3*M//4, N//4))
    for i, energy in enumerate(bottom_left):
        print("{0}: {1:.1f}".format(i, energy))
    print("RGB: ({0:.0f}, {1:.0f}, {2:.0f})\n".format(img[3*M//4, N//4, 2], img[3*M//4, N//4, 1], img[3*M//4, N//4, 0]))


    print("Energies at ({0}, {1})".format(3*M//4, 3*N//4))
    for i, energy in enumerate(bottom_right):
        print("{0}: {1:.1f}".format(i, energy))
    print("RGB: ({0:.0f}, {1:.0f}, {2:.0f})".format(img[3*M//4, 3*N//4, 2], img[3*M//4, 3*N//4, 1], img[3*M//4, 3*N//4, 0]))


# =============================================================================
if __name__ == "__main__":
    img_list, out_img_name, sigma, p = arg_parse()
    h = floor(2.5  * sigma)
    ksize = 2 * h + 1
    ksize = int(sqrt(ksize))
    
    img = cv2.imread(img_list[0], cv2.IMREAD_COLOR)
    energy_sum = np.zeros(img.shape, dtype=np.float64)
    weighted_imgs = np.zeros(img.shape, dtype=np.float64)

    print("Results:\n")
    top_left = []
    top_right = []
    bottom_left = []
    bottom_right = []
    M, N, _ = img.shape

    # kernelX = cv2.getGaussianKernel(int(sqrt(ksize)), sigma); 
    # kernelY = cv2.getGaussianKernel(int(sqrt(ksize)), sigma); 
    # kernelXY = kernelX * kernelY.T;
    # print(kernelXY)


    # for each image in the directory
    for img_name in img_list:
        img = cv2.imread(img_name, cv2.IMREAD_COLOR)
        gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

        # calculate the gradients
        sobelx = cv2.Sobel(gray, cv2.CV_64F, 1, 0, ksize=ksize)
        sobely = cv2.Sobel(gray, cv2.CV_64F, 0, 1, ksize=ksize)
        gradient = sobelx**2 + sobely**2

        # calculate energy for each pixel
        energy = cv2.GaussianBlur(gradient, (ksize, ksize), sigma)
        # energy = cv2.GaussianBlur(gray, (int(sqrt(ksize)), int(sqrt(ksize))), sigma)

        # avoid divide by zero later
        energy[energy == 0] = 0.000001

        # save for the output
        top_left.append(energy[M//4, N//4])
        top_right.append(energy[M//4, 3*N//4])
        bottom_left.append(energy[3*M//4, N//4])
        bottom_right.append(energy[3*M//4, 3*N//4])

        # calculate running sum of the weighted energy of each image
        energy = np.dstack((energy, energy, energy))
        weighted_imgs += energy**p * img
        energy_sum += energy**p

    # normalize
    # print(energy_sum.shape)
    result = weighted_imgs / energy_sum

    # print(top_left)

    output(M, N, top_left, top_right, bottom_left, bottom_right, result)

    cv2.imwrite(out_img_name, result)
    print("Wrote", out_img_name)
























    # SVD A represents:
    # and eigendecomposition represents:
    # this is how they relate: