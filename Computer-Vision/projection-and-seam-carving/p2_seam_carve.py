from sys import argv
import ntpath
import math as m

import numpy as np
import cv2


# -----------------------------------------------------------------------------
def arg_parse():
    if len(argv) == 2:
        script, path = argv
        img = cv2.imread(path, cv2.IMREAD_COLOR)

        # extract image name
        _, filename = ntpath.split(path)
        split = filename.split(".")
        filename = split[0]
        ext = split[1]

    else:
        print("Invalid Argument(s).")
        print("USAGE: {0} <img>".format(argv[0]))
        exit()

    return img, filename, ext

# -----------------------------------------------------------------------------
def calculate_energy(img):
    img = np.uint8(img)

    sigma = 0.5
    h = m.floor(2.5*sigma)
    ksize = (2*h+1, 2*h+1)

    # compute energy (sum of gradients) from grayscaled image
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    gray = cv2.GaussianBlur(gray, ksize, sigma, borderType=cv2.BORDER_DEFAULT)
    sobelx = cv2.Sobel(gray, cv2.CV_64F, 1, 0, ksize=3, borderType=cv2.BORDER_DEFAULT)
    sobely = cv2.Sobel(gray, cv2.CV_64F, 0, 1, ksize=3, borderType=cv2.BORDER_DEFAULT)
    energy = np.abs(sobelx) + np.abs(sobely)

    # assign very high values to the first and last columns 
    # to prevent the seam from reaching them
    energy[:,0] = 1e9
    energy[:,-1] = 1e9

    return energy


# -----------------------------------------------------------------------------
def calculate_cost(energy):
    # first row of cost function equals energy of that row 
    cost = np.zeros(energy.shape)
    cost[0,:] = energy[0,:]

    # calculate the cost of each pixel, row by row
    for i in range(energy.shape[0]-1):
        # already calculated the first row
        i += 1

        # compute the costs on the edges
        cost[i, 0] = energy[i, 0] + np.minimum(cost[i-1, 0], cost[i-1, 1])
        cost[i, -1] = energy[i, -1] + np.minimum(cost[i-1, -1], cost[i-1, -2])

        # compute the costs in the middle
        # minimum of           right           left           middle
        prev = np.minimum(cost[i-1, :-2], cost[i-1, 2:], cost[i-1, 1:-1])
        cost[i,1:-1] = energy[i,1:-1] + prev

    return cost

# -----------------------------------------------------------------------------
def find_seam(img, cost, filename="", ext="", first=False):
    seam = []

    # endpoint is the minimum energy at the bottom of the image
    index = np.argmin(cost[-1, :])
    seam.append(index)

    # paint the seam on the image if it's the first run through
    if first:
        img[-1, index] = [0,0,255]

    # backtrack to trace the seam to the top of the image 
    for i in range(energy.shape[0]-1)[::-1]:
        # print(cost[i, index-1:index+2])
        index = index + np.argmin(cost[i, index-1:index+2]) - 1

        # paint the seam on the image if it's the first run through
        if first:
            img[i, index] = [0,0,255]

        seam.append(index)



    if first:
        cv2.imwrite(filename + "_seam." + ext, img)

    # seam is now a list of indices from bottom of image to top
    # return it from top to bottom order
    return seam[::-1], cost[-1, index] / img.shape[0]
   
# -----------------------------------------------------------------------------
def remove_seam(img, seam):
    result = np.zeros((img.shape[0], img.shape[1]-1, img.shape[2]))

    # copy every pixel in each row except for the seam pixel
    for j,index in enumerate(seam):
        result[j, :, :] = img[j, np.r_[0:index, index+1:result.shape[1]+1], :]

    return result


# -----------------------------------------------------------------------------
def output(i, img, seam, rotated, avg_energy):
    # if i == 0:
        # for j, index in enumerate(seam):
            # print("Writing on image at", j, index)
    

    print("\nPoints on seam {0}:".format(i))
    if rotated:
        print("horizontal")
        print("{0}, {1}".format(seam[0], 0))
    else:
        print("vertical")
        print("{0}, {1}".format(0, seam[0]))
        
    print("{0}, {1}".format(img.shape[0] // 2, seam[img.shape[0] // 2]))
    print("{0}, {1}".format(img.shape[0]-1, seam[img.shape[0]-1]))
    print("Energy of seam {0}: {1:.2f}".format(i, avg_energy))


# =============================================================================
if __name__ == "__main__":
    img, filename, ext = arg_parse()

    rotated = False
    # rotate the image if it is vertical
    if img.shape[0] > img.shape[1]:
        img = np.rot90(img, k=-1, axes=(1,0))
        rotated = True

    # carve seams until the image is square
    i = 0
    while img.shape[0] != img.shape[1]:
        # print("Finding seam:", img.shape)
        energy = calculate_energy(img)
        cost = calculate_cost(energy)

        # draw the seam on the first run through
        if i == 0:
            seam, avg_energy = find_seam(img, cost, filename, ext, True)
        else:
            seam, avg_energy = find_seam(img, cost)

        # print some statistics for the first and second runs
        if i == 0 or i == 1:
            output(i, img, seam, rotated, avg_energy)

        img = remove_seam(img, seam)
        i += 1

    i -= 1

    # print some statistics for the last run
    output(i, img, seam, rotated, avg_energy)

    if rotated:
        img = np.rot90(img, k=1, axes=(1,0))

    cv2.imwrite(filename + "_final." + ext, img)