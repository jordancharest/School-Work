from sys import argv
import os
from math import floor

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


# =============================================================================
if __name__ == "__main__":
    img_list, out_img_name, sigma, p = arg_parse()
    h = floor(2.5  * sigma)
    ksize = 2 * h + 1
    
    img = cv2.imread(img_list[0], cv2.IMREAD_COLOR)
    energy_sum = np.zeros(img.shape, dtype=np.float64)
    weighted_imgs = np.zeros(img.shape, dtype=np.float64)

    # for each image in the directory
    for img_name in img_list:
        img = cv2.imread(img_name, cv2.IMREAD_COLOR)

        # calculate the gradients
        sobelx = cv2.Sobel(img, cv2.CV_64F, 1, 0, ksize=5)
        sobely = cv2.Sobel(img, cv2.CV_64F, 0, 1, ksize=5)
        gradient = sobelx**2 + sobely**2

        # calculate energy for each pixel
        energy = cv2.GaussianBlur(gradient, (ksize, ksize), sigma)
        energy[energy == 0] = 0.000001

        # calculate running sum of the weighted eneergy of each image
        weighted_imgs += energy**p * img
        energy_sum += energy**p

    # normalize
    result = weighted_imgs / energy_sum

    cv2.imwrite(out_img_name, result)