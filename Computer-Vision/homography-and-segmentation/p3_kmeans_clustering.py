from sys import argv

import numpy as np
import cv2

# -----------------------------------------------------------------------------
def arg_parse():
    if len(argv) == 3:
        script, k, img_name = argv
        img = cv2.imread(img_name, cv2.IMREAD_COLOR)
        return int(k), img

    else:
        print("Invalid Argument(s).")
        print("USAGE: {} <k> <image>".format(argv[0]))
        exit()


# -----------------------------------------------------------------------------
def test_split(img):
    # img = np.dstack((img, img, img))
    print(img.shape)
    

    print("\nBefore:\n", img)

    num_sections = img.shape[0]*img.shape[1]
    img = np.reshape(img, (num_sections,3))

    # img = np.hsplit(img, img.shape[1])
    # img = np.hsplit(img, img.shape[1])
    print("\nAfter:\n", img)

# -----------------------------------------------------------------------------
def get_vectors(img):

    # generate pixel coordinate values
    y = np.linspace(0, img.shape[0]-1, num=img.shape[0])
    x = np.linspace(0, img.shape[1]-1, num=img.shape[1])
    XX,YY = np.meshgrid(x,y)
    
    # print(XX)
    # print(YY)
    # print(img)

    # stack them on the actual RGB values
    vectors = np.dstack((XX, YY, img))

    # reshape into individual pixel / coordinate vectors
    # num_pixels = img.shape[0]*img.shape[1]
    vectors = np.reshape(vectors, (-1, 5))

    print(vectors)

    return vectors

# -----------------------------------------------------------------------------
def cluster(vectors, k):
    # Specify the termination criteria, including the number of iterations and
    # an upper bound on the amount of change in the position of the center.
    criteria = (cv2.TERM_CRITERIA_EPS | cv2.TERM_CRITERIA_MAX_ITER, 100, 1.0)
    num_reinitializations = 1
    # initialization_method = cv2.KMEANS_RANDOM_CENTERS
    initialization_method = cv2.KMEANS_PP_CENTERS
    ret, label, center = cv2.kmeans(all_values, k, None, criteria,
                                    num_reinitializations, initialization_method)
    print(center)

    # don't know what this does
    # for i in range(num_clusters):
    #     cluster = all_values[label.ravel() == i]
    #     x = cluster[:, 0]
    #     y = cluster[:, 1]
    #     print('Cluster %d: %d points' % (i, len(x)))
    #     c = np.random.random(3).reshape(1,3)
    #     plt.scatter(x, y, c=c)
    # plt.scatter(center[:,0],center[:,1],s = 80,c = 'y', marker = 's')
    # plt.axis('equal')
    # plt.show()




# =============================================================================
if __name__ == "__main__":
    k, img = arg_parse()
    img = np.ones((5, 8, 3))
    img[:,:,1] *= 2
    img[:,:,2] *= 3
    vectors = get_vectors(img)

    # test_split(img)
