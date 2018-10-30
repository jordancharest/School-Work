from sys import argv
import os
import ntpath

import numpy as np
from scipy import spatial
import cv2


# some tunable params
MIN_MATCH_COUNT = 25
FM_RANSAC_THRESHOLD = 0.25
H_RANSAC_THRESHOLD = 1.0

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
def get_output_names(name_list):
    output_names = []
    for i, name in enumerate(name_list):
        _, filename = ntpath.split(name)
        if i == 0:
            out_name, ext = filename.split(".")
        else:
            out_name, _ = filename.split(".")

        output_names.append(out_name)

    output_names.append(ext)
    return output_names

# -----------------------------------------------------------------------------
def draw_keypoint_matches(img1, kp1, img2, kp2, matches, out_name,
                            matchesMask=None, matchColor=(0,255,0)):
    
    # draw matches on the images together
    draw_params = dict(matchColor = matchColor,
                   singlePointColor = None,
                   matchesMask = matchesMask, # draw only inliers (if specified)
                   flags = 2)

    out_img = cv2.drawMatches(img1, kp1, img2, kp2, matches, None, **draw_params)
    cv2.imwrite(out_name, out_img)


# -----------------------------------------------------------------------------
def stitch(img1, img2, H, i):
    rows1, cols1 = img1.shape[:2]
    rows2, cols2 = img2.shape[:2]

    list_of_points_1 = np.float32([[0,0], [0,rows1], [cols1,rows1], [cols1,0]]).reshape(-1,1,2)
    temp_points = np.float32([[0,0], [0,rows2], [cols2,rows2], [cols2,0]]).reshape(-1,1,2)
    list_of_points_2 = cv2.perspectiveTransform(temp_points, H)
    list_of_points = np.concatenate((list_of_points_1, list_of_points_2), axis=0)

    [x_min, y_min] = np.int32(list_of_points.min(axis=0).ravel() - 0.5)
    [x_max, y_max] = np.int32(list_of_points.max(axis=0).ravel() + 0.5)
    translation_dist = [-x_min,-y_min]
    H_translation = np.array([[1, 0, translation_dist[0]], [0, 1, translation_dist[1]], [0,0,1]])

    output_img = cv2.warpPerspective(img2, H_translation.dot(H), (x_max-x_min, y_max-y_min))
    output_img[translation_dist[1]:rows1+translation_dist[1], translation_dist[0]:cols1+translation_dist[0]] = img1

    cv2.imwrite("Mosaic" + str(i) + ".jpg", output_img)
    
    # return output_img


# -----------------------------------------------------------------------------
def detect_and_match(img_name1, img_name2, i):
    out_name1, out_name2, ext = get_output_names((img_name1, img_name2))

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
    draw_keypoint_matches(img1, kp1, img2, kp2, good, out_name, matchColor=(0,0,255))

    # if the minimum threshold is met
    if len(good) > MIN_MATCH_COUNT:
        src_pts = np.float32([ kp1[m.queryIdx].pt for m in good]).reshape(-1,1,2)
        dst_pts = np.float32([ kp2[m.trainIdx].pt for m in good]).reshape(-1,1,2)
    
        # calculate fundamental matrix, output surviving keypoint match
        print("Calculating Fundamental Matrix")
        F, mask = cv2.findFundamentalMat(src_pts, dst_pts, cv2.FM_RANSAC, FM_RANSAC_THRESHOLD)
        matchesMask = mask.ravel().tolist()
        num_F_matches = np.sum(matchesMask)
        print("Original: ", len(matchesMask))
        print("Survived: ", num_F_matches)
        out_name = out_name1 + "_" + out_name2 + "_after_F_mat." + ext
        draw_keypoint_matches(img1, kp1, img2, kp2, good, out_name, matchesMask, (0,255,0))

        # h,w = img1.shape
        # pts = np.float32([ [0,0],[0,h-1],[w-1,h-1],[w-1,0] ]).reshape(-1,1,2)
        # dst = cv2.perspectiveTransform(pts, F)

        # calculate homography
        print("Calculating Homography")
        H, mask = cv2.findHomography(src_pts, dst_pts, cv2.RANSAC, H_RANSAC_THRESHOLD)
        matchesMask = mask.ravel().tolist()
        num_H_matches = np.sum(matchesMask)
        print("Original: ", len(matchesMask))
        print("Survived: ", num_H_matches)
        out_name = out_name1 + "_" + out_name2 + "_after_H_mat." + ext
        draw_keypoint_matches(img1, kp1, img2, kp2, good, out_name, matchesMask, (255,0,0))


        # if they are a good fit
        if True:
            stitch(img2, img1, H, i)

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
        detect_and_match(img_list[i], img_list[i+1], i)




