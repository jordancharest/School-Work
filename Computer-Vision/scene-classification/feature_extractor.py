from sys import argv
import glob

import numpy as np
import cv2

# tunable parameters
t = 4
bh = 4  # number of blocks in image height direction
bw = 4  # number of blocks in image width direction

# -----------------------------------------------------------------------------
def arg_parse():
    if len(argv) == 2:
        _, directory = argv
        return directory

    else:
        print("Invalid Argument(s).")
        print("USAGE: {} <training-data-directory>".format(argv[0]))
        exit()

# -----------------------------------------------------------------------------
def get_img_names(root_dir):
    grass = glob.glob(root_dir + "/grass/*.JPEG")
    ocean = glob.glob(root_dir + "/ocean/*.JPEG")
    redcarpet = glob.glob(root_dir + "/redcarpet/*.JPEG")
    road = glob.glob(root_dir + "/road/*.JPEG")
    wheatfield = glob.glob(root_dir + "/wheatfield/*.JPEG")

    print("Grass:", len(grass))
    print("Ocean:", len(ocean))
    print("Red carpet:", len(redcarpet))
    print("Road:", len(road))
    print("Wheatfield:", len(wheatfield))

    return grass, ocean, redcarpet, road, wheatfield

# -----------------------------------------------------------------------------
def get_feature_vector(img):
    dw = img.shape[1] / (bw + 1)
    dh = img.shape[0] / (bh + 1)
    


# -----------------------------------------------------------------------------
def extract_features(img_list, cspace='BGR'):
    features = []
    
    for img_name in img_list:
        img = cv2.imread(img_name, cv2.IMREAD_COLOR)

        # to test out some color conversions
        if cspace != 'RGB' or cspace != 'BGR':
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

        feature_vector = get_feature_vector(img)
        features.append(feature_vector)

    return features

# =============================================================================
if __name__ == "__main__":
    training_data_dir = arg_parse()
    grass_imgs, ocean_imgs, redcarpet_imgs, road_imgs, wheatfield_imgs = 
        get_img_names(training_data_dir)

    # get feature vectors for each image in the set
    grass_features = extract_features(grass_imgs)
    ocean_features = extract_features(ocean_imgs)
    redcarpet_features = extract_features(redcarpet_imgs)
    road_features = extract_features(road_imgs)
    wheatfield_features = extract_features(wheatfield_imgs)

