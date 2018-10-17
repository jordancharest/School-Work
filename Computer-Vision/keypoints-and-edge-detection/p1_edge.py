from sys import argv

import numpy as np
import cv2
# -----------------------------------------------------------------------------
def arg_parse():
    if len(argv) == 3:
        script, sigma, img_name = argv
        img = cv2.imread(img_name, cv2.IMREAD_COLOR)
        return sigma, img

    else:
        print("Invalid Argument(s).")
        print("USAGE: {} <sigma> <image>".format(argv[0]))
        exit()


# =============================================================================
if __name__ == "__main__":
    sigma, img = arg_parse()