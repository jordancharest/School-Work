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

# -----------------------------------------------------------------------------
def calculate_energy(img):
    img = np.uint8(img)

    # compute energy (sum of gradients) from grayscaled image
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
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
        cost[i, -1] = energy[i, 1] + np.minimum(cost[i-1, -1], cost[i-1, -2])

        # compute the costs in the middle
        # minimum of           right           left           middle
        prev = np.minimum(cost[i-1, :-2], cost[i-1, 2:], cost[i-1, 1:-1])
        cost[i,1:-1] = energy[i,1:-1] + prev

    return cost

# -----------------------------------------------------------------------------
def find_seam(img, cost, first=False):
    seam = []

    # endpoint is the minimum energy at the bottom of the image
    index = np.argmin(cost[-1, :])
    seam.append(index)

    # print("Endpoint:", index)
    # print(cost[-1, index])

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



    # seam is now a list of indices from bottom of image to top
    return seam
   
# -----------------------------------------------------------------------------
def remove_seam(img, seam):
    result = np.zeros((img.shape[0], img.shape[1]-1, img.shape[2]))

    # copy every pixel in each row except for the seam pixel
    for j,index in enumerate(seam[::-1]):
        result[j, :, :] = img[j, np.r_[0:index, index+1:result.shape[1]+1], :]

    return result




# =============================================================================
if __name__ == "__main__":
    img = arg_parse()
    print(img.shape)

    # carve seams until the image is square
    while img.shape[0] != img.shape[1]:
        # print("Finding seam:", img.shape)
        energy = calculate_energy(img)
        cost = calculate_cost(energy)
        seam = find_seam(img, cost, True)
        img = remove_seam(img, seam)


    print(img.shape)
    cv2.imwrite("result.png", img)