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
    # compute energy (sum of gradients) from grayscaled image
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    sobelx = cv2.Sobel(gray, cv2.CV_64F, 1, 0, ksize=3, borderType=cv2.BORDER_DEFAULT)
    sobely = cv2.Sobel(gray, cv2.CV_64F, 0, 1, ksize=3, borderType=cv2.BORDER_DEFAULT)
    energy = np.abs(sobelx) + np.abs(sobely)

    # assign very high values to the first and last columns 
    # to prevent the seam from reaching them
    energy[:,0] = 1e7
    energy[:,-1] = 1e7

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

        # compute the costs on the edges (artificially high)
        cost[i, 0] = energy[i, 0] + cost[i-1, 0]
        cost[i, -1] = energy[i, 1] + cost[i-1, -1]

        # compute the costs in the middle
        # minimum of           right           left           middle
        prev = np.minimum(cost[i-1, :-2], cost[i-1, 2:], cost[i-1, 1:-1])
        cost[i,1:-1] = energy[i,1:-1] + prev

    return cost


# =============================================================================
if __name__ == "__main__":
    img = arg_parse()
    print(img.shape)

    # carve seams until the image is square
    while img.shape[0] != img.shape[1]:
        energy = calculate_energy(img)
        cost = calculate_cost(energy)
        # seam = find_seam(cost)
        # remove_seam(img)



        # backtrack to find the seam - endpoint is the minimum energy at the bottom of the image
        endpoint = np.argmin(cost[-1, :])
        for i in range(energy.shape[0]):

            # endpoint is the minimum energy at the bottom of the image
            img[-1, endpoint] = [0,0,255]
            break


        print(np.max(energy))
        print(energy)
        print(cost)
        break


    cv2.imwrite("endpoint.jpg", img)