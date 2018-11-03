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
def plot_multiple(images, rows, cols, titles=[], figsize=(20,10), fontsize=30,
                    save=False, save_name=""):

    f, axs = plt.subplots(rows, cols, figsize=figsize)
    axs = axs.ravel()

    if len(images) != len(titles):
        print("ERROR: Number of titles not equal to number of images")
        return

    for j, img in enumerate(images):
        axs[j].imshow(img)
        if titles:
            axs[j].set_title(titles[j], fontsize=fontsize)

    if save:
        plt.savefig(save_name)

# -----------------------------------------------------------------------------
def read_rectangles(filename):
    # read the first line of the file to determine how many
    # foreground rectangles there are
    with open(filename, "r") as file:
        numbers = file.readline().split()
        for i in range(len(numbers)):
            if numbers[i].lower() == "foreground":
                num_foreground = int(numbers[i+1])

    all_rectangles = np.loadtxt(filename, dtype=np.int32)

    # first row is boundary, then foreground rows, the rest are background
    foreground = []
    background = []
    if len(all_rectangles.shape) > 1:
        boundary = all_rectangles[0, :]
        foreground = all_rectangles[1:num_foreground+1, :]
        background = all_rectangles[num_foreground+1:, :]
    else:
        boundary = all_rectangles

    return boundary, foreground, background


# -----------------------------------------------------------------------------
def draw_rectangles(img, boundary, foreground, background):
    boundary_img = np.copy(img)

    # draw boundary
    pt1 = tuple(boundary[:2])
    pt2 = tuple(boundary[2:])
    boundary_img = cv2.rectangle(boundary_img, pt1, pt2, color=(255,0,0), thickness=2)

    # draw foreground rectangles
    for row in foreground:
        pt1 = tuple(row[:2])
        pt2 = tuple(row[2:])
        boundary_img = cv2.rectangle(boundary_img, pt1, pt2, color=(0,255,0), thickness=1)

    # draw background rectangles
    for row in background:
        pt1 = tuple(row[:2])
        pt2 = tuple(row[2:])
        boundary_img = cv2.rectangle(boundary_img, pt1, pt2, color=(0,0,255), thickness=1)

    cv2.imwrite("boundary.jpg", boundary_img)    

# -----------------------------------------------------------------------------
def segment(img, boundary, foreground, background):

    # define the entire mask as definite background to start
    mask = np.ones(img.shape[:2], np.uint8)
    mask *= cv2.GC_BGD

    # mask the boundary rectangle as probable foreground
    x1,y1,x2,y2 = boundary
    mask[y1:y2, x1:x2] = cv2.GC_PR_FGD
    
    # set definite foreground masks
    for x1,y1,x2,y2 in foreground:
        mask[y1:y2, x1:x2] = cv2.GC_FGD

    # set definite background masks
    for x1,y1,x2,y2 in background:
        mask[y1:y2, x1:x2] = cv2.GC_BGD

    # run grabcut using the mask
    bgdModel = np.zeros((1,65),np.float64)
    fgdModel = np.zeros((1,65),np.float64)
    rect = tuple(boundary)
    mask, fgdModel, bgdModel = cv2.grabCut(img, mask, rect, bgdModel,
                                            fgdModel, 5, 
                                            cv2.GC_INIT_WITH_MASK)

    mask = np.where((mask==2)|(mask==0),0,1).astype('uint8')
    img = img*mask[:,:,np.newaxis]

    return img

# =============================================================================
if __name__ == "__main__":
    img, pixel_file = arg_parse()

    # reduce size
    h,w = img.shape[:2]
    while max(h,w) > 600:
        h //= 2
        w //= 2
    img = cv2.resize(img,(w, h), interpolation = cv2.INTER_CUBIC)

    # read, draw, and save user defined rectangles
    boundary, foreground, background = read_rectangles(pixel_file)
    draw_rectangles(img, boundary, foreground, background)

    print("Boundary:\n", boundary)
    print("Foreground:\n", foreground)
    print("Background:\n", background)

    # run GrabCut
    segmented = segment(img, boundary, foreground, background)

    # save result
    cv2.imwrite("segmented.jpg", segmented)
    # show_with_pixel_values(img)
    # show_with_pixel_values(segmented)