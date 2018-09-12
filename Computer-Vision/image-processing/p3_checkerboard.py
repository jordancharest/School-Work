from sys import argv
import os

import numpy as np
import cv2



# -----------------------------------------------------------------------------
def arg_parse():
    if len(argv) == 6:
        _, directory, out_img, M, N, size = argv
        return directory, out_img, int(M), int(N), int(size)

    else:
        print("Invalid arguments")
        exit()


# -----------------------------------------------------------------------------
def crop_and_resize(img_list, size):
    result = []

    for img in img_list:
        cropped = False
        top_left = [0,0]
        bottom_right = [img.shape[0]-1, img.shape[1]-1]

        # print(img.shape)

        # handle cropping
        if img.shape[0] == img.shape[1]:
            print("Image does not require cropping")

        # horizontal crop
        elif img.shape[0] < img.shape[1]:
            excess = img.shape[1] - img.shape[0]
            left = excess//2
            right = excess - left
            
            # for diagnostic output
            top_left[1] = left
            bottom_right[1] = img.shape[1] - right - 1
            cropped = True

            # actual cropping
            img = img[:, left:img.shape[1]-right]


        # vertical crop
        elif img.shape[0] > img.shape[1]:
            excess = img.shape[0] - img.shape[1]
            top = excess//2
            bottom = excess - top

            # for diagnostic output
            top_left[0] = top
            bottom_right[0] = img.shape[0] - bottom - 1
            cropped = True 

            # actual cropping
            img = img[top:img.shape[0]-bottom, :]

        # diagnostic output
        if cropped:
            print("Image cropped at ({0},{1}) and ({2},{3})".format(top_left[0], top_left[1], bottom_right[0], bottom_right[1]))

        # handle resizing if necessary
        if img.shape[0] == size:
            print("No resizing needed")
        else:
            print("Resized from {0}".format(img.shape), end="")
            img = cv2.resize(img, (size,size))
            print(" to {0}".format(img.shape))

        result.append(img)

    return result

# -----------------------------------------------------------------------------
def gen_black_and_white(size):
    white = 255 * np.ones((size,size,3))
    black = np.zeros((size,size,3))
    return white, black

# -----------------------------------------------------------------------------
def whitest_and_blackest(img_list, size):
    max_distance = 0.0
    max_index = 0
    min_index = 0

    # find the two most unalike images
    for i, img_name in enumerate(img_list):
        img = cv2.imread(img_name, cv2.IMREAD_COLOR)
        filename = os.path.basename(img_name)
        blue_avg = np.average(img[:,:,0])
        green_avg = np.average(img[:,:,1])
        red_avg = np.average(img[:,:,2])

        # output diagnostic info
        print("{0} ({1:.1f}, {2:.1f}, {3:.1f})".format(filename, red_avg, green_avg, blue_avg))


        # check this image against every other image in the directory
        for j, test in enumerate(img_list):
            img2 = cv2.imread(test, cv2.IMREAD_COLOR)
            j_blue_avg = np.average(img2[:,:,0])
            j_green_avg = np.average(img2[:,:,1])
            j_red_avg = np.average(img2[:,:,2])
            distance = np.sqrt((blue_avg-j_blue_avg)**2 + (green_avg-j_green_avg)**2 + (red_avg-j_red_avg)**2)

            # if we found a new max distance, save this pair of images
            if distance > max_distance:
                max_distance = distance

                # img1 is the whiter of the two (max_index)
                i_size = np.sqrt(blue_avg**2 + green_avg**2 + red_avg**2)
                j_size = np.sqrt(j_blue_avg**2 + j_green_avg**2 + j_red_avg**2)
                if (i_size > j_size):
                    max_index,  min_index = i, j
                else:
                    max_index, min_index = j, i

    # output
    filename1 = os.path.basename(img_list[max_index])
    filename2 = os.path.basename(img_list[min_index])
    print("Checkerboard from {0} and {1}. Distance between them is {2:.1f}".format(filename1, filename2, max_distance))

    # read in the images, crop and resize them
    img1 = cv2.imread(img_list[max_index], cv2.IMREAD_COLOR)
    img2 = cv2.imread(img_list[min_index], cv2.IMREAD_COLOR)
    img1, img2 = crop_and_resize([img1,img2], size)

    return img1, img2

# -----------------------------------------------------------------------------
def build_checkerboard(img1, img2, M, N):
    tile = np.vstack((np.hstack((img1,img2)), np.hstack((img2,img1))))
    return np.tile(tile, (M//2, N//2, 1))

# MAIN ========================================================================
if __name__ == "__main__":
    directory, out_img, M, N, size = arg_parse()
    img_list = os.listdir(directory)
    img_list = [directory +  "/" + name for name in img_list if '.jpg' in name.lower()]
    img_list.sort()

    # handle four different cases
    # no images, generate a default checkerboard
    if len(img_list) == 0:
        print("No images. Creating an ordinary checkerboard.")
        img1, img2 = gen_black_and_white(size)

        # only needed to get correct output on submitty, doesn't do anything
        img1, img2 = crop_and_resize([img1, img2], size)

    # crop and resize the image 1, generate a black image for image 2
    elif len(img_list) == 1:
        filename = os.path.basename(img_list[0])
        print("One image: {0}. It will form the white square.".format(filename))
        img1 = cv2.imread(img_list[0], cv2.IMREAD_COLOR)
        img2 = np.zeros((size, size, 3))
        img1, img2 = crop_and_resize([img1, img2], size)

    # read, crop, and resize both images
    elif len(img_list) == 2:
        filename1 = os.path.basename(img_list[0])
        filename2 = os.path.basename(img_list[1])
        print("Exactly two images: {0} and {1}. Creating checkerboard from these.".format(filename1, filename2))
        img1 = cv2.imread(img_list[0], cv2.IMREAD_COLOR)
        img2 = cv2.imread(img_list[1], cv2.IMREAD_COLOR)
        img1, img2 = crop_and_resize([img1,img2], size)

    # check every image to find the most unalike images, crop and resize
    else:
        img1, img2 = whitest_and_blackest(img_list, size)

    # make checkerboard and write to disk
    result = build_checkerboard(img1, img2, M, N)
    cv2.imwrite(out_img, result)