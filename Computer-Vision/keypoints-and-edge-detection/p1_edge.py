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
        img = cv2.imread(path_to_img, cv2.IMREAD_COLOR)

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

    return gradient_magnitude, gradient_direction

# -----------------------------------------------------------------------------
def gradient_direction_image(magnitude, direction, filename, ext, shape):
    """
    east/west           : red
    north/south         : blue
    northeast/southwest : white
    northwest/southeast : green
    image borders       : black
    magnitude < 1.0     : black
    """

    result = np.zeros(shape)


    direction[direction < 0] += m.pi
    direction += m.pi/8

    direction //= m.pi/4
    direction %= 4

    red = direction == 0
    green = direction == 1
    blue = direction == 2
    white = direction == 3

    # assign the colors
    result[red] = (0,0,255)
    result[white] = (255,255,255)
    result[blue] = (255,0,0)
    result[green] = (0,255,0)

    result[magnitude < 1.0] = (0,0,0)

    cv2.imwrite(filename + "_dir." + ext, result)





# =============================================================================
if __name__ == "__main__":
    sigma, img, img_name, ext = arg_parse()
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY).astype(float)
    print(gray.dtype)

    # smooth and calculate gradient magnitude and direction
    grad_magnitude, grad_direction = calculate_gradients(gray, sigma)

    # encode the gradient direction into 5 different color bins
    # and write to disk
    gradient_direction_image(grad_magnitude, grad_direction, img_name, ext, img.shape)