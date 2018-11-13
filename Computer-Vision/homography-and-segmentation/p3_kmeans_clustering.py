from sys import argv

import cv2
import matplotlib.pyplot as plt
import numpy as np

# to scale the image coordinates so they have less of 
# an effect during clustering
scale = 9

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
def crop_image(img, gray, tol=0):
    # img is image data
    # tol  is tolerance
    mask = gray > tol
    # print(mask.any(0))
    return img[np.ix_(mask.any(1),mask.any(0), (True, True, True))]

# -----------------------------------------------------------------------------
def plot_multiple(images, rows, cols, titles=[], figsize=(20,10), fontsize=30,
                    save=False, save_name=""):

    f, axs = plt.subplots(rows, cols, figsize=figsize)
    axs = axs.ravel()

    if len(images) != len(titles):
        print("ERROR: Number of titles not equal to number of images")
        return

    for j, img in enumerate(images):
        img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
        axs[j].imshow(img)
        if titles:
            axs[j].set_title(titles[j], fontsize=fontsize)

    if save:
        plt.savefig(save_name, bbox_inches='tight', pad_inches=0)

# -----------------------------------------------------------------------------
def get_vectors(img):

    # generate pixel coordinate values
    # print(img.shape[0]-1)
    y = np.linspace(0, img.shape[0]-1, num=img.shape[0])
    x = np.linspace(0, img.shape[1]-1, num=img.shape[1])
    XX,YY = np.meshgrid(x,y)
    YY = np.flipud(YY)
    # print(XX)
    # print(YY)

    # calculate std deviation of the image within a small kernel
    blur = cv2.blur(img,(5,5))
    blur2 = cv2.blur(img**2, (5,5))
    std_dev = np.sqrt(blur2 - blur**2)
    
    # stack them on the actual RGB values
    # place x and y first so that we can
    vectors = np.dstack((XX/scale, YY/scale, img, std_dev))
    # vectors = np.dstack((XX/scale, YY/scale, img))

    # reshape into individual pixel / coordinate vectors
    vectors = np.reshape(vectors, (-1, vectors.shape[-1]))

    return vectors

# -----------------------------------------------------------------------------
def cluster(img, vectors, k):
    # Specify the termination criteria, including the number of iterations and
    # an upper bound on the amount of change in the position of the center.
    criteria = (cv2.TERM_CRITERIA_EPS | cv2.TERM_CRITERIA_MAX_ITER, 100, 1.0)
    num_reinitializations = 5
    # initialization_method = cv2.KMEANS_RANDOM_CENTERS
    initialization_method = cv2.KMEANS_PP_CENTERS
    ret, label, center = cv2.kmeans(vectors.astype(np.float32), k, None, criteria,
                                    num_reinitializations, initialization_method)
    center *= scale
    print("Segmenting into {} clusters".format(k))
    print("Centers:\n", center)

    # plot
    for i in range(k):
        cluster = vectors[label.ravel() == i]
        x = cluster[:, 0] * scale
        y = cluster[:, 1] * scale
        print('Cluster %d: %d points' % (i, len(x)))
        c = np.random.random(3).reshape(1,3)
        plt.scatter(x, y, c=c)
    plt.scatter(center[:,0], center[:,1], s=1, c='y', marker='s')
    # plt.axis('equal')
    plt.axis('off')
    plt.savefig('clusters.png', bbox_inches='tight', pad_inches=0)
    # plt.show()


    # crop the white borders of the matplotlib fig
    clusters = cv2.imread("clusters.png", cv2.IMREAD_COLOR)
    gray = cv2.cvtColor(clusters, cv2.COLOR_BGR2GRAY)
    gray[gray == 255] = 0
    clusters = crop_image(clusters, gray)

    # resize to fit to original image, and combine together
    clusters = cv2.resize(clusters, (img.shape[1], img.shape[0]))
    combined = cv2.addWeighted(img, 0.5, clusters, 0.5, 0)

    # display in a nice format
    imgs = [img, clusters, combined]
    titles = ["Original", "Clustered", "Combined"]
    plot_multiple(imgs, 1, 3, titles, save=True, save_name="kmeans_combined.png")


# =============================================================================
if __name__ == "__main__":
    k, img = arg_parse()
    print(img.shape)
    vectors = get_vectors(img)

    cluster(img, vectors, k)

    # test_split(img)
