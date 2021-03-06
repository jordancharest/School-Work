from sys import argv
import math as m
import ntpath

import numpy as np
import cv2

# -----------------------------------------------------------------------------
def plot_pics(image_list, num_cols=2, title_list=[]):
    if len(image_list) == 0: return
    if len(image_list[0].shape) == 2:
        plt.gray()
    num_rows = int( m.ceil(len(image_list)/float(num_cols)))

    if num_cols > 2:
        plt.figure(figsize=(10,10))
    else:
        plt.figure(figsize=(15,15))

    for i in range(len(image_list)):
        im = image_list[i]
        print(num_rows, num_cols, i+1)
        plt.subplot(num_rows, num_cols, i+1)
        plt.imshow(im)
        if i < len(title_list):
            plt.title(title_list[i])
        plt.xticks([]), plt.yticks([])

    plt.show()


# -----------------------------------------------------------------------------
def arg_parse():
    if len(argv) == 3:
        script, sigma, path_to_img = argv

        # extract image name and read it
        _, filename = ntpath.split(path_to_img)
        filename, ext = filename.split(".")
        img = cv2.imread(path_to_img, cv2.IMREAD_GRAYSCALE).astype(np.float64)

        return float(sigma), img, filename, ext

    else:
        print("Invalid Argument(s).")
        print("USAGE: {} <sigma> <image>".format(argv[0]))
        exit()

# -----------------------------------------------------------------------------
def calculate_gradients(img, sigma=1.0):
    ksize = (int(4*sigma+1), int(4*sigma+1))
    blurred = cv2.GaussianBlur(img, ksize, sigma)

    kx,ky = cv2.getDerivKernels(1,1,3)
    kx = np.transpose(kx/2)
    ky = ky/2
    x_gradient = cv2.filter2D(blurred, -1, kx)
    y_gradient = cv2.filter2D(blurred, -1, ky)

    gradient_magnitude = np.sqrt(x_gradient**2 + y_gradient**2)
    gradient_direction = np.arctan2(y_gradient, x_gradient)
    # print("Maximum direction:",np.max(gradient_direction))
    # print("Minimum direction:",np.min(gradient_direction))

    return gradient_magnitude, gradient_direction

# -----------------------------------------------------------------------------
def gradient_direction(magnitude, direction, filename, ext, shape):
    """
    east/west           : red
    north/south         : blue
    northeast/southwest : white
    northwest/southeast : green
    image borders       : black
    magnitude < 1.0     : black
    """
    shape = list(shape)
    shape.append(3)
    result = np.zeros(shape)

    # rotate so that the boundary between red and green is at 0
    direction += m.pi/8

    # separate into 4 bins, 45 degrees each
    direction //= (m.pi/4)
    direction %= 4

    # define masks for each bin/color
    red = np.where(direction == 0, magnitude, 0)
    green = np.where(direction == 1, magnitude, 0)
    blue = np.where(direction == 2, magnitude, 0)
    white = np.where(direction == 3, magnitude, 0)

    # assign the colors
    result[np.nonzero(red)] = (0,0,255)
    result[np.nonzero(white)] = (255,255,255)
    result[np.nonzero(blue)] = (255,0,0)
    result[np.nonzero(green)] = (0,255,0)
    result[magnitude < 1.0] = 0

    cv2.imwrite(filename + "_dir." + ext, result)

    return red, white, blue, green

# -----------------------------------------------------------------------------
def non_max_suppression(mag, E_W, NE_SW, N_S, NW_SE):

    # remove the border of gradient direction images
    E_W = E_W[1:-1, 1:-1]
    NE_SW = NE_SW[1:-1, 1:-1]
    N_S = N_S[1:-1, 1:-1]
    NW_SE = NW_SE[1:-1, 1:-1]

    # create shifted images to threshold the gradient magnitude with
    # not memory efficient but code readability greatly increases
    up = mag[2:, 1:-1]
    down = mag[:-2, 1:-1]
    left = mag[1:-1, 2:]
    right = mag[1:-1, :-2]
    up_right = mag[2:, :-2]
    up_left = mag[2:, 2:]
    down_right = mag[:-2, :-2]
    down_left = mag[:-2, 2:]

    # threshold each direction separately; if pixel is greater than both
    # of its neighbors(shifted to align) then pixel = 1, otherwise 0
    result = np.zeros(E_W.shape)
    result += np.where((E_W >= left) & (E_W >= right), 1, 0)
    result += np.where((NE_SW >= down_left) & (NE_SW >= up_right), 1, 0)
    result += np.where((N_S >= up) & (N_S >= down), 1, 0)
    result += np.where((NW_SE >= up_left) & (NW_SE >= down_right), 1, 0)
    result = np.where(result > 0, 1, 0)

    mag[1:-1, 1:-1] *= result

    return mag

# -----------------------------------------------------------------------------
def edge_threshold(mag, sigma):
    print("Number after non-maximum:", np.count_nonzero(mag))
    mag[mag < 1.0] = 0
    mu = np.mean(mag[mag > 0])
    s = np.std(mag[mag > 0])
    threshold = min(30/sigma, mu + 0.5*s)

    print("Number after 1.0 threshold:", np.count_nonzero(mag))
    print("Average: {0:.2f}".format(mu))
    print("Std dev: {0:.2f}".format(s))
    print("Threshold: {0:.2f}".format(threshold))

    mag[mag < threshold] = 0
    print("Number after threshold:", np.count_nonzero(mag))

    return mag

# =============================================================================
if __name__ == "__main__":
    sigma, gray, img_name, ext = arg_parse()
    # gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY).astype(float)

    # smooth and calculate gradient magnitude and direction
    grad_magnitude, grad_direction = calculate_gradients(gray, sigma)

    # suppress borders
    grad_magnitude[0, :] = 0
    grad_magnitude[-1, :] = 0
    grad_magnitude[:, 0] = 0
    grad_magnitude[:, -1] = 0

    # gradient magnitude, converted to scale 0-255
    grad_output = (grad_magnitude / np.max(grad_magnitude)) * 255
    
    cv2.imwrite(img_name + "_grd." + ext, grad_output)

    # separate the gradient direction into 4 different bins, 45 degrees each
    E_W, NE_SW, N_S, NW_SE = gradient_direction(grad_magnitude, grad_direction, 
                                                img_name, ext, gray.shape)

    # attempt to trim edges to a single pixel wide and remove spurious detections
    grad_magnitude = non_max_suppression(grad_magnitude, E_W, NE_SW, N_S, NW_SE)
    grad_magnitude = edge_threshold(grad_magnitude, sigma)

    # normalize from 0-255
    grad_output = (grad_magnitude / np.max(grad_magnitude)) * 255
    cv2.imwrite(img_name + "_thr." + ext, grad_output)