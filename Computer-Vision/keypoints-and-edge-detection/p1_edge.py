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
def gradient_direction(magnitude, direction, filename, ext, shape):
    """
    east/west           : red
    north/south         : blue
    northeast/southwest : white
    northwest/southeast : green
    image borders       : black
    magnitude < 1.0     : black
    """

    result = np.zeros(shape)

    # rotate so that the boundary between red and green is at 0
    direction += m.pi/8

    # separate into 4 bins
    direction //= (m.pi/4)
    direction %= 4

    # define masks for each bin/color
    red = direction == 0
    red[magnitude < 1.0] = 0
    green = direction == 1
    green[magnitude < 1.0] = 0
    blue = direction == 2
    blue[magnitude < 1.0] = 0
    white = direction == 3
    white[magnitude < 1.0] = 0

    # assign the colors
    result[red] = (0,0,255)
    result[white] = (255,255,255)
    result[blue] = (255,0,0)
    result[green] = (0,255,0)

    # magnitude threshold
    # result[magnitude < 1.0] = (0,0,0)


    # TODO
    # ASSIGN BORDER VALUES TO 0

    cv2.imwrite(filename + "_dir." + ext, result)

    return red, white, blue, green

# -----------------------------------------------------------------------------
def non_max_suppression(mag, E_W, NE_SW, N_S, NW_SE):

    # remove the border of gradient direction images
    E_W = E_W[1:-1, 1:-1]
    NE_SW = NE_SW[1:-1, 1:-1]
    N_S = N_S[1:-1, 1:-1]
    NW_SE = NW_SE[1:-1, 1:-1]
    print(E_W)


    # create shifted images to threshold the gradient magnitude with
    up = mag[2:, 1:-1]
    down = mag[:-2, 1:-1]
    left = mag[1:-1, 2:]
    right = mag[1:-1, :-2]
    up_right = mag[2:, :-2]
    up_left = mag[2:, 2:]
    down_right = mag[:-2, :-2]
    down_left = mag[:-2, 2:]

    result = np.zeros(E_W.shape)

    # threshold each direction separately
    result[E_W >= left] = 1
    result[E_W >= right] = 1

    result[NE_SW >= down_left] = 1
    result[NE_SW >= up_right] = 1

    result[N_S >= up] = 1
    result[N_S >= down] = 1

    result[NW_SE >= up_left] = 1
    result[NW_SE >= down_right] = 1

    print("\nMag: \n", mag)
    print("\nE/W: \n", E_W)
    print("\nNE/SW: \n", NE_SW)
    print("\nN/S: \n", N_S)
    print("\nNW/SE: \n", NW_SE)

    mag[1:-1, 1:-1] *= result
    print(mag)

    return mag

# =============================================================================
if __name__ == "__main__":
    sigma, img, img_name, ext = arg_parse()
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY).astype(float)
    print(gray.dtype)

    # smooth and calculate gradient magnitude and direction
    grad_magnitude, grad_direction = calculate_gradients(gray, sigma)

    # gradient magnitude, converted to scale 0-255
    grad_output = (grad_magnitude / np.max(grad_magnitude)) * 255
    cv2.imwrite(img_name + "_grd." + ext, grad_output)

    # encode the gradient direction into 4 different bins
    # and write to disk
    E_W, NE_SW, N_S, NW_SE = gradient_direction(grad_magnitude, grad_direction, 
                                                img_name, ext, img.shape)



    grad_magnitude = non_max_suppression(grad_magnitude, E_W, NE_SW, N_S, NW_SE)
    grad_output = (grad_magnitude / np.max(grad_magnitude)) * 255
    cv2.imwrite(img_name + "_non_max." + ext, grad_output)




