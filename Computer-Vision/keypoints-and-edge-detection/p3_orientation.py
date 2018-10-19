from sys import argv

import numpy as np
import cv2


# -----------------------------------------------------------------------------
def arg_parse():
    if len(argv) == 4:
        script, sigma, img_name, points_file = argv
        pts = np.loadtxt(points_file, np.int32)
        img = cv2.imread(img_name, cv2.IMREAD_COLOR)
        return sigma, img, pts

    else:
        print("Invalid Argument(s).")
        print("USAGE: {0} <sigma> <img> <points-file>")
        exit()


# =============================================================================
if __name__ == "__main__":
    sigma, img, pts = arg_parse()
    print(pts)