from sys import argv

import cv2
import numpy as np

# TODO:
# Keep a color version to draw on but compute optical flow on grayscale


# -----------------------------------------------------------------------------
def arg_parse():
    if len(argv) == 3:
        script, img_name0, img_name1 = argv
        img0 = cv2.imread(img_name0, cv2.IMREAD_GRAYSCALE)
        img1 = cv2.imread(img_name1, cv2.IMREAD_GRAYSCALE)
        return img0, img1

    else:
        print("Invalid Argument(s).")
        print("USAGE: {} <image1> <image2>".format(argv[0]))
        exit()

# -----------------------------------------------------------------------------
def optical_flow(img0, img1, feature_params, lk_params):
    # Create some random colors
    colors = np.random.randint(0,255,(100,3))
    points0 = cv2.goodFeaturesToTrack(img0, mask=None, **feature_params)
    points1, status, err = cv2.calcOpticalFlowPyrLK(img0, img1, points0, None, **lk_params)

    # Create a mask image for drawing purposes
    mask = np.zeros_like(img1)


    # print("Points:\n", points1)
    # print("Status:\n", status)
    # print("Error:\n", err)

    # Select good points
    good0 = points0[status == 1]
    good1 = points1[status == 1]

    # draw the tracks
    for i,(new,old) in enumerate(zip(good1,good0)):
        a,b = new.ravel()
        c,d = old.ravel()
        mask = cv2.line(mask, (a,b), (c,d), colors[i].tolist(), 2)
        img1 = cv2.circle(img1, (a,b), 5, colors[i].tolist(), -1)
    result = img1 + mask
    cv2.imwrite("result.png", result)

    # print()


# =============================================================================
if __name__ == "__main__":
    img0, img1 = arg_parse()

    # params for ShiTomasi corner detection
    feature_params = dict( maxCorners = 100,
                           qualityLevel = 0.3,
                           minDistance = 7,
                           blockSize = 7 )
    # Parameters for lucas kanade optical flow
    lk_params = dict( winSize  = (15,15),
                      maxLevel = 2,
                      criteria = (cv2.TERM_CRITERIA_EPS | cv2.TERM_CRITERIA_COUNT, 10, 0.03))

    optical_flow(img0, img1, feature_params, lk_params)