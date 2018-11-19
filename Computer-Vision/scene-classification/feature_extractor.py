from sys import argv
import glob
import os
import time

from skimage.feature import hog
import numpy as np
import cv2



# tunable parameters
t = 4
bh = 4  # number of blocks in image height direction
bw = 4  # number of blocks in image width direction

# for HoG extraction
# best so far:
# orient = 11
# pix_per_cell = 16
# cell_per_block = 2

orient = 11
pix_per_cell = 24
cell_per_block = 2

# -----------------------------------------------------------------------------
def arg_parse():
    if len(argv) == 3:
        _, training_data, test_data = argv
        return training_data, test_data, None, False

    elif len(argv) == 4:
        _, training_data, test_data, parameter = argv
        if parameter == "use-hog":
            return training_data, test_data, None, True
        else:
            return training_data, test_data, parameter, False

    elif len(argv) == 5:
        _, training_data, test_data, colorspace, use_hog = argv
        if use_hog == "use-hog":
            return training_data, test_data, colorspace, True
        else:
            print("Unknown parameter. To use HoG features type 'use-hog'")
            exit()

    else:
        print("Invalid Argument(s).")
        print("USAGE: {} <training-data> <test-data> [<colorspace> <use-hog>]".format(argv[0]))
        exit()

# -----------------------------------------------------------------------------
def get_img_names(root_dir):
    grass = glob.glob(root_dir + "/grass/*.JPEG")
    ocean = glob.glob(root_dir + "/ocean/*.JPEG")
    redcarpet = glob.glob(root_dir + "/redcarpet/*.JPEG")
    road = glob.glob(root_dir + "/road/*.JPEG")
    wheatfield = glob.glob(root_dir + "/wheatfield/*.JPEG")

    print("Grass:", len(grass), "images")
    print("Ocean:", len(ocean), "images")
    print("Red carpet:", len(redcarpet), "images")
    print("Road:", len(road), "images")
    print("Wheatfield:", len(wheatfield), "images")
    print()

    return grass, ocean, redcarpet, road, wheatfield

# -----------------------------------------------------------------------------
def get_hog_features(img):
    hog_img = cv2.resize(img, (48, 72))
    hog_img = cv2.cvtColor(hog_img, cv2.COLOR_BGR2LUV)

    hog_features = []
    for channel in range(hog_img.shape[2]):
        features = hog(hog_img[:, :, channel],
                        orientations=orient, 
                        pixels_per_cell=(pix_per_cell, pix_per_cell),
                        cells_per_block=(cell_per_block, cell_per_block),
                        transform_sqrt=False, 
                        feature_vector=True)
        hog_features.append(features)

    hog_features = np.ravel(hog_features)
    return hog_features

# -----------------------------------------------------------------------------
def get_feature_vector(img, use_hog=False):
    dh = int(img.shape[0] / (bh + 1))
    dw = int(img.shape[1] / (bw + 1))
    block_size = (2*dh, 2*dw)

    feature_vector = []

    for m in range(bh):
        for n in range(bw):
            # define the block
            y = m * dh
            x = n * dw
            pixels = img[y:y+block_size[0], x:x+block_size[1], :]
            pixels = pixels.reshape(block_size[0]*block_size[1], 3)

            # compute histogram of the block and 'concatenate'
            hist, _ = np.histogramdd(pixels, (t, t, t))
            hist = hist.ravel()

            feature_vector.append(hist)

    # fix the shape - unraveled numpy array
    feature_vector = np.ravel(feature_vector)

    if use_hog:
        hog_features = get_hog_features(img)
        feature_vector = np.hstack((feature_vector, hog_features))

    return feature_vector

# -----------------------------------------------------------------------------
def extract_features(img_list, cspace=None, use_hog=False):
    features = []
    
    for i, img_name in enumerate(img_list):
        img = cv2.imread(img_name, cv2.IMREAD_COLOR)

        # to test out some color conversions
        if cspace != 'RGB' or cspace != 'BGR' or cspace != None:
            if cspace == 'HSV':
                img = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)
            elif cspace == 'LUV':
                img = cv2.cvtColor(img, cv2.COLOR_BGR2LUV)
            elif cspace == 'HLS':
                img = cv2.cvtColor(img, cv2.COLOR_BGR2HLS)
            elif cspace == 'YUV':
                img = cv2.cvtColor(img, cv2.COLOR_BGR2YUV)
            elif cspace == 'YCrCb':
                img = cv2.cvtColor(img, cv2.COLOR_BGR2YCrCb)

        feature_vector = get_feature_vector(img, use_hog)
        features.append(feature_vector)

    features = np.array(features)
    return features

# =============================================================================
if __name__ == "__main__":
    training_data, test_data, colorspace, use_hog = arg_parse()
    if colorspace:
        print("Using the {} colorspace".format(colorspace))
    names = ["grass", "ocean", "redcarpet", "road", "wheatfield"]

    # Create target directory if it doesn't exist
    dir_name = "features"
    if not os.path.exists(dir_name):
        os.mkdir(dir_name)

    # get feature vectors for each image in the training set
    print("Extracting feature vectors from the training set")
    all_imgs = get_img_names(training_data)
    for j, img_set in enumerate(all_imgs):
        start = time.time()
        features = extract_features(img_set, colorspace, use_hog)

        np.save("./features/train_" + names[j], features)
        print(round(time.time()-start, 2), "seconds to extract", names[j], "train features...")

    # get feature vectors for each image in the test set
    print("\nExtracting feature vectors from the test set")
    all_imgs = get_img_names(test_data)
    for j, img_set in enumerate(all_imgs):
        start = time.time()
        features = extract_features(img_set, colorspace, use_hog)

        np.save("./features/test_" + names[j], features)
        print(round(time.time()-start, 2), "seconds to extract", names[j], "test features...")    