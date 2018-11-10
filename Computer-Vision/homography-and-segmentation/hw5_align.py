from sys import argv
from copy import deepcopy
import os
import ntpath

import numpy as np
import cv2

# some tunable params
MIN_MATCH_COUNT = 25
LOWE_RATIO = 0.7
FM_RANSAC_THRESHOLD = 0.3
H_RANSAC_THRESHOLD = 0.5
F_TO_H_THRESHOLD = 0.1

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
                            matchesMask=None, color=(0,255,0)):
    
    # draw matches on the images together
    draw_params = dict(matchColor = color,
                   singlePointColor = None,
                   matchesMask = matchesMask, # draw only inliers (if specified)
                   flags = 2)

    out_img = cv2.drawMatches(img1, kp1, img2, kp2, matches, None, **draw_params)
    cv2.imwrite(out_name, out_img)


# -----------------------------------------------------------------------------
def find_match(img1, img2, kp1, kp2, good, stats1, stats2):
    src_pts = np.float32([ kp1[m.queryIdx].pt for m in good]).reshape(-1,1,2)
    dst_pts = np.float32([ kp2[m.trainIdx].pt for m in good]).reshape(-1,1,2)

    # approximate the Fundamental matrix (RANSAC) and output the matches
    # that survive
    F, matches_mask, num_F_matches = calculate_transformation(src_pts,
                                                              dst_pts,
                                                              kind="F")
    out_name = "./results/" + stats1['name'] + "_" + stats2['name'] + "_after_F_mat.jpg"
    draw_keypoint_matches(img1, kp1, img2, kp2, good, out_name,
                            matches_mask, color=(0,255,0))


    # approximate a Homography (RANSAC) and output the matches
    # that survive
    H, matches_mask, num_H_matches = calculate_transformation(src_pts,
                                                              dst_pts,
                                                              kind="H")
    out_name = "./results/" + stats1['name'] + "_" + stats2['name'] + "_after_H_mat.jpg"
    draw_keypoint_matches(img1, kp1, img2, kp2, good, out_name,
                            matches_mask, color=(255,0,0))


    # if they are a good fit update the match statistics
    ratio = num_H_matches / num_F_matches
    max_matches = max(num_H_matches, num_F_matches)

    # if max_matches > 0.33*len(good) and \
    if (ratio > F_TO_H_THRESHOLD or num_H_matches > 200) and \
        num_H_matches > 50:
        
        stats1['total_matches'] += num_H_matches
        stats2['total_matches'] += num_H_matches
        stats1['img_matches'].append(stats2['index'])
        stats2['img_matches'].append(stats1['index'])
        stats1['homography'].append(H)
        stats2['homography'].append(H)
    else:
        print("These two images would not make a good mosaic")

# -----------------------------------------------------------------------------
def calculate_transformation(src_pts, dst_pts, kind="F"):
    if kind == "F":
        print("Calculating Fundamental Matrix")
        M, mask = cv2.findFundamentalMat(src_pts, dst_pts, cv2.FM_RANSAC, FM_RANSAC_THRESHOLD)
    elif kind == "H":
        print("Calculating Homography")
        M, mask = cv2.findHomography(src_pts, dst_pts, cv2.RANSAC, H_RANSAC_THRESHOLD)
    else:
        print("ERROR: Unknown transformation type!")

    matches_mask = mask.ravel().tolist()
    num_matches = np.sum(matches_mask)
    print("Original Keypoints: ", len(matches_mask))
    print("Survived RANSAC   : ", num_matches)

    return M, matches_mask, num_matches

# -----------------------------------------------------------------------------
def stitch(canvas, img2, limits, H, name):
    h1, w1 = img1.shape[:2]
    x_max, x_min, y_max, y_min = limits
    print(x_max, x_min, y_max, y_min)

    # warp the new image onto a black canvas the same size as the original canvas
    translation = [-x_min,-y_min]
    H_trans = np.array([[1, 0, translation[0]], [0, 1, translation[1]], [0,0,1]])
    warped = cv2.warpPerspective(img2, H_trans.dot(H), (x_max-x_min, y_max-y_min))

    # combine
    overlap = np.where((canvas > 0) & (warped > 0), True, False)
    temp = cv2.addWeighted(canvas[overlap], 0.5, warped[overlap].astype(np.float64), 0.5, 0)
    canvas = cv2.addWeighted(canvas, 1.0, warped.astype(np.float64), 1.0, 0)
    canvas[overlap] = temp.ravel()
    
    cv2.imwrite("./results/" + name + ".jpg", canvas)
    
    return canvas


# -----------------------------------------------------------------------------
def detect_and_match(color_img1, color_img2, stats1, stats2):

    print("\nComparing {0} and {1}".format(stats1['name'], stats2['name']))

    # keypoint matching is done in grayscale
    img1 = cv2.cvtColor(color_img1, cv2.COLOR_BGR2GRAY)
    img2 = cv2.cvtColor(color_img2, cv2.COLOR_BGR2GRAY)
    
    # detect keypoints and compute and descriptors for each image
    detector = cv2.xfeatures2d.SURF_create()
    # detector = cv2.xfeatures2d.SIFT_create()
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
        if m.distance < LOWE_RATIO*n.distance:
            good.append(m)

    # draw all keypoints that pass the ratio test
    out_name = "./results/" + stats1['name'] + "_" + stats2['name'] + "_before_F_mat.jpg"
    draw_keypoint_matches(img1, kp1, img2, kp2, good, out_name, color=(0,0,255))

    # if the minimum threshold is met, calculate transformations and attempt
    # to find a match
    if len(good) > MIN_MATCH_COUNT:
        find_match(img1, img2, kp1, kp2, good, stats1, stats2)

    else:
        print("Not enough matches are found - %d/%d" % (len(good),MIN_MATCH_COUNT))
        matchesMask = None


# -----------------------------------------------------------------------------
def find_translation(anchor, images, match_stats):
    limits = None
    # print(limits.shape)

    # first find the images that are not directly connected to the anchor, but still
    # have a path to the anchor
    for j, img in enumerate(images):
        if j not in anchor['img_matches'] and j != anchor['index']:
            for k,m in enumerate(match_stats[j]['img_matches']):
                if m in anchor['img_matches']:
                    anchor['total_matches'] += 1
                    match_stats[j]['total_matches'] += 1
                    anchor['img_matches'].append(match_stats[j]['index'])
                    match_stats[j]['img_matches'].append(anchor['index'])

                    # update the homography to include two transformations
                    anchor['homography'].append(anchor['homography'][k].dot(match_stats[j]['homography'][k]))
                    match_stats[j]['homography'].append(anchor['homography'][k].dot(match_stats[j]['homography'][k]))

    # then determine the min and max limits of the canvas needed to fit all
    # the transformed images
    for j, img in enumerate(images):
        if j == anchor['index']:
            h,w = img.shape[:2]
            corners = np.float32([[0,0], [0,h], [w,h], [w,0]]).reshape(-1,1,2)
            if limits is None:
                limits = corners
            else:
                limits = np.concatenate((limits, corners), axis=0)

        elif j in anchor['img_matches']:
            for i, k in enumerate(anchor['img_matches']):
                if k == j:
                    h,w = img.shape[:2]
                    corners = np.float32([[0,0], [0,h], [w,h], [w,0]]).reshape(-1,1,2)

                    if i < anchor['index']:
                        corners = cv2.perspectiveTransform(corners, anchor['homography'][i])
                    else:
                        corners = cv2.perspectiveTransform(corners, np.linalg.inv(anchor['homography'][i]))

                    if limits is None:
                        limits = corners
                    else:
                        limits = np.concatenate((limits, corners), axis=0)

    [x_min, y_min] = np.int32(limits.min(axis=0).ravel() - 0.5)
    [x_max, y_max] = np.int32(limits.max(axis=0).ravel() + 0.5)

    return x_max, x_min, y_max, y_min

# -----------------------------------------------------------------------------
def build_mosaic(images, match_stats, anchor):
    # anchor = match_stats[0]
    anchor_img = images[anchor['index']]
    print("\nAnchor:", anchor['name'])

    limits = find_translation(anchor, images, match_stats)
    x_max, x_min, y_max, y_min = limits
    canvas = np.zeros((y_max-y_min, x_max-x_min, 3))

    # paint anchor on the canvas, no transformation/warping
    h1,w1 = anchor_img.shape[:2]
    translation = [-x_min,-y_min]
    canvas[translation[1]:h1+translation[1], translation[0]:w1+translation[0]] = anchor_img

    # paint every other matching image on the canvas
    names = [anchor["name"]]
    j = 0
    for i in anchor['img_matches']:
        names.append(match_stats[i]["name"])
        names.sort()
        # the order of the images will depend on
        if i < anchor['index']:
            canvas = stitch(canvas, images[i], limits, anchor['homography'][j], "_".join(names))
        else:
            canvas = stitch(canvas, images[i], limits, np.linalg.inv(anchor['homography'][j]), "_".join(names))

        j += 1


# =============================================================================
if __name__ == "__main__":
    img_list = arg_parse()

    # Create target directory if it doesn't exist
    dir_name = "results"
    if not os.path.exists(dir_name):
        os.mkdir(dir_name)

    # hold all images and statistics about the match quality
    images = [None] * len(img_list)
    ms = {   
        'name' : "",
        'index' : 0,
        'total_matches' : 0,
        'img_matches' : [],
        'homography' : []
    }
    match_stats = [deepcopy(ms) for k in range(len(img_list))]
    mosaic_num = 0

    # attempt to match each image with every other image in the directory
    # return a set of characteristics that describe how well it matched
    for i, img_name1 in enumerate(img_list[:-1]):
        for j, img_name2 in enumerate(img_list[i+1:]):
            j += i + 1

            # don't read images in multiple times and
            # initialize the dictionary characterizing this match
            if images[i] is None:
                img1 = cv2.imread(img_name1, cv2.IMREAD_COLOR)
                images[i] = img1
                print(img_name1)
                match_stats[i]['name'], _ = get_output_names([img_name1])
                match_stats[i]['index'] = i
            else:
                img1 = images[i]

            if images[j] is None:
                img2 = cv2.imread(img_name2, cv2.IMREAD_COLOR)
                images[j] = img2
                match_stats[j]['name'], _ = get_output_names([img_name2])
                match_stats[j]['index'] = j
            else:
                img2 = images[j]

            # detect and match keypoints between the two images, return
            # characteristics about the match
            detect_and_match(img1, img2, match_stats[i], match_stats[j])

            mosaic_num += 1

    # use the match characteristics to determine how to build the mosaic(s)
    # the image with the highest number of matches will be the anchor
    sorted_match_stats = deepcopy(match_stats)
    sorted_match_stats.sort(key = lambda k : k['total_matches'], reverse=True)
    print("\n\nMatch Statistics (sorted):")
    for d in sorted_match_stats:
        print()
        for k, v in d.items():
            if k == 'homography':
                print("  {0} :  {1}".format("homographies".ljust(13), len(v)))
            else:
                print("  {0} :  {1}".format(k.ljust(13),v))

    anchor = sorted_match_stats[0]
    build_mosaic(images, match_stats, anchor)