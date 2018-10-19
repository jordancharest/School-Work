from sys import argv
import ntpath

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

# =============================================================================
if __name__ == "__main__":
    sigma, img, img_name, ext = arg_parse()

    sift = cv2.xfeatures2d.SIFT_create()
