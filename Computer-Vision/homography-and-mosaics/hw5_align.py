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
    img1 = cv2.imread(img_list[0], cv2.IMREAD_GRAYSCALE)
    # sift = cv2.SIFT()
    sift = cv2.xfeatures2d.SURF_create()
    kp1, des1 = sift.detectAndCompute(img1, None)

    for img_name in img_list[1:]:

        # read SIFT keypoints and descriptors for the new image
        img2 = cv2.imread(img_name, cv2.IMREAD_GRAYSCALE)
        # sift = cv2.SIFT()
        kp2, des2 = sift.detectAndCompute(img2, None)

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



    # check if the images match
    if len(good) > MIN_MATCH_COUNT:

        src_pts = np.float32([ kp1[m.queryIdx].pt for m in good]).reshape(-1,1,2)
        dst_pts = np.float32([ kp2[m.trainIdx].pt for m in good]).reshape(-1,1,2)

        M, mask = cv2.findHomography(src_pts, dst_pts, cv2.RANSAC,5.0)
        matchesMask = mask.ravel().tolist()

        h,w = img1.shape
        pts = np.float32([ [0,0],[0,h-1],[w-1,h-1],[w-1,0] ]).reshape(-1,1,2)
        dst = cv2.perspectiveTransform(pts,M)

        img2 = cv2.polylines(img2,[np.int32(dst)],True,255,3, cv2.LINE_AA)

    else:
        print("Not enough matches are found - %d/%d" % (len(good),MIN_MATCH_COUNT))
        matchesMask = None

    # draw matches on the images together
    draw_params = dict(matchColor = (0,255,0), # draw matches in green color
                   singlePointColor = None,
                   matchesMask = matchesMask, # draw only inliers
                   flags = 2)

    img3 = cv2.drawMatches(img1,kp1,img2,kp2,good,None,**draw_params)
    cv2.imwrite("Keypoint Match.jpg", img3)

    # plt.imshow(img3, 'gray'),plt.show()


# =============================================================================
if __name__ == "__main__":

    img_list = arg_parse()
    MIN_MATCH_COUNT = 10
    print(img_list)

    # extract a list containing a kd-tree of keypoints from each image
    matches = detect_and_match(img_list)




