from sys import argv

import numpy as np
import cv2


# -----------------------------------------------------------------------------
def arg_parse():
    if len(argv) == 3:
        script, image, pixel_file = argv
        return image, pixel_file

    else:
        print("Invalid Argument(s).")
        print("USAGE: {} <image> <pixel-coordinates-file>")
        exit()

# -----------------------------------------------------------------------------
def read_rectangles(filename):

    boundary = None
    foreground = None
    background = None

    # read the first line of the file and determine how many
    # foreground rectangles there are
    with open(filename, "r") as file:
        numbers = file.readline().split()
        print(numbers)
        for i in range(len(numbers)):
            if numbers[i].lower() == "foreground":
                num_foreground = int(numbers[i+1])

    all_rectangles = np.loadtxt(filename)

    # first row is boundary, then foreground rows, the rest are background
    boundary = all_rectangles[0, :]
    foreground = all_rectangles[1:num_foreground+1, :]
    background = all_rectangles[num_foreground+1:, :]

    return boundary, foreground, background


# =============================================================================
if __name__ == "__main__":
    img_name, pixel_file = arg_parse()
    boundary, foreground, background = read_rectangles(pixel_file)

    print("Boundary:\n", boundary)
    print("Foreground:\n", foreground)
    print("Background:\n", background)