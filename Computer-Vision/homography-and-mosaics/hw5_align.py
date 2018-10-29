from sys import argv
import os

import numpy as np
from scipy import spatial
import cv2


# -----------------------------------------------------------------------------
def arg_parse():
    if len(argv) == 2:
        script, image_dir = argv
        img_list = os.listdir(image_dir)
        img_list = [image_dir +  "/" + name for name in img_list if '.jpg' in name.lower()]
        img_list.sort()

        return img_list

    else:
        print("Invalid Argument(s).")
        print("USAGE: {} <image-directory>".format(argv[0]))
        exit()

# -----------------------------------------------------------------------------
# def extract_keypoints(img_list):

#     keypoint_trees = []
#     for img_name in img_list:

#         # extract sift_keypoints
#         img = cv2.imread(img_name, cv2.IMREAD_GRAYSCALE)
#         sift = cv2.xfeatures2d.SIFT_create()
#         kp = sift.detect(img, None)

#         # sort?
#         # kp.sort( key = lambda k: k.response)
#         # kp = kp[::-1]

#         # create a KD-tree with the keypoints
#         keypoint_trees.append(spatial.KDTree(kp))

#     return keypoint_trees


# # ---------------------------------------------------------------------------
# def match_keypoints():



# -----------------------------------------------------------------------------
def detect_and_match(img_list):
    # find the keypoints and descriptors of the enchor with SIFT
    anchor = cv2.imread(img_list[0], cv2.IMREAD_GRAYSCALE)
    kp_anchor, desc_anchor = sift.detectAndCompute(anchor, None)

    for img_name in img_list[1:]:

        # read SIFT keypoints and descriptors for the new image
        img = cv2.imread(img_name, cv2.IMREAD_GRAYSCALE)
        sift = cv2.SIFT()
        kp2, des2 = sift.detectAndCompute(img, None)

        # 'Fast Library for Approximate Nearest Neighbor search'
        # Searh for approximate nearest neighbors between the anchor and
        # the current image
        FLANN_INDEX_KDTREE = 0
        index_params = dict(algorithm = FLANN_INDEX_KDTREE, trees = 5)
        search_params = dict(checks = 50)
        flann = cv2.FlannBasedMatcher(index_params, search_params)
        matches = flann.knnMatch(des1,des2,k=2)

        # store all the good matches as per Lowe's ratio test
        good = []
        for m,n in matches:
            if m.distance < 0.7*n.distance:
                good.append(m)

    return good


# =============================================================================
if __name__ == "__main__":
    img_list = arg_parse()
    print(img_list)

    # extract a list containing a kd-tree of keypoints from each image
    matches = detect_and_match(img_list)
    # keypoint_trees = extract_keypoints(img_list)
    # match_keypoints(keypoint_trees)


