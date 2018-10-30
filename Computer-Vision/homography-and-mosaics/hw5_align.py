from sys import argv
import os
import ntpath

import numpy as np
from scipy import spatial
import cv2


# some tunable params
MIN_MATCH_COUNT = 25
FM_RANSAC_THRESHOLD = 0.1

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
def draw_keypoint_matches(img1, kp1, img2, kp2, matches, out_name,
                            matchesMask=None):
    
    # draw matches on the images together
    draw_params = dict(matchColor = (0,255,0), # draw matches in green color
                   singlePointColor = None,
                   matchesMask = matchesMask, # draw only inliers (if specified)
                   flags = 2)

    out_img = cv2.drawMatches(img1, kp1, img2, kp2, matches, None, **draw_params)
    cv2.imwrite(out_name, out_img)


# -----------------------------------------------------------------------------
def detect_and_match(img_name1, img_name2):

    _, filename = ntpath.split(img_name1)
    out_name1, ext = filename.split(".")
    _, filename = ntpath.split(img_name2)
    out_name2, _ = filename.split(".")

    img1 = cv2.imread(img_name1, cv2.IMREAD_GRAYSCALE)
    img2 = cv2.imread(img_name2, cv2.IMREAD_GRAYSCALE)
    
    # detect and compute keypoints and descriptors for each image
    # detector = cv2.xfeatures2d.SURF_create()
    detector = cv2.xfeatures2d.SIFT_create()
    kp1, des1 = detector.detectAndCompute(img1, None)
    kp2, des2 = detector.detectAndCompute(img2, None)

    # 'Fast Library for Approximate Nearest Neighbor search'
    # Searh for approximate nearest neighbors between the two images
    FLANN_INDEX_KDTREE = 0
    index_params = dict(algorithm = FLANN_INDEX_KDTREE, trees = 5)
    search_params = dict(checks = 50)
    flann = cv2.FlannBasedMatcher(index_params, search_params)
    matches = flann.knnMatch(des1,des2,k=2)

    # store all the good matches per Lowe's ratio test
    good = []
    for m,n in matches:
        if m.distance < 0.7*n.distance:
            good.append(m)

    # draw all keypoints that pass the ratio test
    out_name = out_name1 + "_" + out_name2 + "_before_F_mat." + ext
    draw_keypoint_matches(img1, kp1, img2, kp2, good, out_name)

    # if the minimum threshold is met
    if len(good) > MIN_MATCH_COUNT:
        src_pts = np.float32([ kp1[m.queryIdx].pt for m in good]).reshape(-1,1,2)
        dst_pts = np.float32([ kp2[m.trainIdx].pt for m in good]).reshape(-1,1,2)
    
        # calculate fundamental matrix, output surviving keypoint match
        F, mask = cv2.findFundamentalMat(src_pts, dst_pts, cv2.FM_RANSAC, FM_RANSAC_THRESHOLD)
        matchesMask = mask.ravel().tolist()
        print("Original: ", len(matchesMask))
        print("Survived: ", np.sum(matchesMask))
        out_name = out_name1 + "_" + out_name2 + "_after_F_mat." + ext
        draw_keypoint_matches(img1, kp1, img2, kp2, good, out_name, matchesMask)

        # h,w = img1.shape
        # pts = np.float32([ [0,0],[0,h-1],[w-1,h-1],[w-1,0] ]).reshape(-1,1,2)
        # dst = cv2.perspectiveTransform(pts, F)

        # calculate homography
        M, mask = cv2.findHomography(src_pts, dst_pts, cv2.RANSAC, 5.0)
        matchesMask = mask.ravel().tolist()
        # dst = cv2.perspectiveTransform(pts,M)
        # img2 = cv2.polylines(img2,[np.int32(dst)],True,255,3, cv2.LINE_AA)



    else:
        print("Not enough matches are found - %d/%d" % (len(good),MIN_MATCH_COUNT))
        matchesMask = None


# =============================================================================
if __name__ == "__main__":

    img_list = arg_parse()
    img_list.sort()
    print(img_list)

    # attempt to match adjacent images in the directory
    for i in range(len(img_list[:-1])):
        detect_and_match(img_list[i], img_list[i+1])




