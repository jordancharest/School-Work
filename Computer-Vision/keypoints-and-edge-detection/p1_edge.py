from sys import argv
import ntpath

import numpy as np
import cv2
# -----------------------------------------------------------------------------
def arg_parse():
    if len(argv) == 3:
        script, sigma, img_name = argv
        img = cv2.imread(img_name, cv2.IMREAD_COLOR)

        # extract image name
        _, filename = ntpath.split(path)
        split = filename.split(".")
        filename = split[0]
        ext = split[1]

        return sigma, img, img_name, ext

    else:
        print("Invalid Argument(s).")
        print("USAGE: {} <sigma> <image>".format(argv[0]))
        exit()


# =============================================================================
if __name__ == "__main__":
    sigma, img, img_name, ext = arg_parse()