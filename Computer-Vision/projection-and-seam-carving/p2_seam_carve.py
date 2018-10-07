from sys import argv

import numpy as np
import cv2


# -----------------------------------------------------------------------------
def arg_parse():
    if len(argv) == 2:
        script, img_name = argv
        img = cv2.imread(img_name, cv2.IMREAD_COLOR)
    else:
        print("Invalid Argument(s).")
        print("USAGE: {0} <img>".format(argv[0]))
        exit()

    return img





if __name__ == "__main__":
    img = arg_parse()
    print(img.shape)