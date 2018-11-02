from sys import argv

import cv2
import matplotlib.pyplot as plt
import numpy as np


# -----------------------------------------------------------------------------
def arg_parse():
    if len(argv) == 3:
        script, img_name, pixel_file = argv
        img = cv2.imread(img_name, cv2.IMREAD_COLOR)
        return img, pixel_file

    else:
        print("Invalid Argument(s).")
        print("USAGE: {} <image> <pixel-coordinates-file>")
        exit()


# -----------------------------------------------------------------------------
def show_with_pixel_values(im):
    '''
    Create a plot of a single image so that pixel intensity or color
    values are shown as the mouse is moved over the image.

    This is a tool everyone can use, but it is not the main focus of
    this example
    '''
    fig, ax = plt.subplots()
    numrows, numcols = im.shape[0], im.shape[1]
    plt.gray()
    ax.imshow(im)

    def format_coord(x, y):
        col = int(x+0.5)
        row = int(y+0.5)
        if col >= 0 and col < numcols and row >= 0 and row < numrows:
            z = im[row, col]
            if type(z) is np.ndarray:    # color image
                return '(%d,%d): [%d,%d,%d]' \
                    % (col, row, int(z[0]), int(z[1]), int(z[2]))
            else:                       # grayscale image
                return '(%d,%d): %d' % (x, y, int(z))
        else:
            return '(%d,%d)' % (x, y)

    ax.format_coord = format_coord  
    plt.show()

# -----------------------------------------------------------------------------
def read_rectangles(filename):
    # read the first line of the file and determine how many
    # foreground rectangles there are
    with open(filename, "r") as file:
        numbers = file.readline().split()
        print(numbers)
        for i in range(len(numbers)):
            if numbers[i].lower() == "foreground":
                num_foreground = int(numbers[i+1])

    all_rectangles = np.loadtxt(filename, dtype=np.int32)

    # first row is boundary, then foreground rows, the rest are background
    boundary = all_rectangles[0, :]
    foreground = all_rectangles[1:num_foreground+1, :]
    background = all_rectangles[num_foreground+1:, :]

    return boundary, foreground, background


# -----------------------------------------------------------------------------
def draw_rectangles(img, boundary, background, foreground):
    pass


# =============================================================================
if __name__ == "__main__":
    img, pixel_file = arg_parse()
    boundary, foreground, background = read_rectangles(pixel_file)

    labeled_imgs = draw_rectangles(img, boundary, foreground, background)
    boundary_img, fground_img, bground_img = labeled_imgs


    print("Boundary:\n", boundary)
    print("Foreground:\n", foreground)
    print("Background:\n", background)

    pt1 = tuple(boundary[:2])
    pt2 = tuple(boundary[2:])
    print(pt1)
    print(pt2)
    boundary_img = cv2.rectangle(img, pt1, pt2, color=(255,0,0), thickness=2)
    show_with_pixel_values(boundary_img)