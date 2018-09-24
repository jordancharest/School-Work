from sys import argv

import numpy as np
import cv2

def arg_parse():
    if len(argv) == 3:
        script, in_img, out_img = argv
        return in_img, out_img
    else:
        print("Invalid Argument(s).")
        print("USAGE: {0} <in-image> <out-image>\n".format(argv[0]))
        exit()

# =============================================================================
if __name__ == "__main__":
    in_img_name, out_img_name = arg_parse()
    img = cv2.imread(in_img_name, cv2.IMREAD_COLOR)

    # calculate the size of the output image
    result = 0
    height, width, _ = img.shape
    while width >= 20 and height >= 20:
        result += width
        width //= 2
        height //= 2

    # create empty output image
    out_img = np.zeros((img.shape[0], result, img.shape[2]))

    # continuously copy then resize the original image into the output
    row = 0
    col = 0
    while img.shape[0] >= 20 and img.shape[1] >= 20:
        out_img[row:row+img.shape[0], col:col+img.shape[1], :] = img

        #  resize image and adjust the row and column
        col += img.shape[1]
        img = cv2.resize(img, (img.shape[1]//2, img.shape[0]//2))
        row = out_img.shape[0]//2 - img.shape[0]//2

    cv2.imwrite(out_img_name, out_img)