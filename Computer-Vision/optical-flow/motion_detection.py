from sys import argv

import cv2
import numpy as np

point_motion_threshold = 5
camera_motion_threshold = 0.9


# -----------------------------------------------------------------------------
def arg_parse():
    if len(argv) == 3:
        script, img_name0, img_name1 = argv
        img0 = cv2.imread(img_name0, cv2.IMREAD_COLOR)
        img1 = cv2.imread(img_name1, cv2.IMREAD_COLOR)
        return img0, img1

    else:
        print("Invalid Argument(s).")
        print("USAGE: {} <image1> <image2>".format(argv[0]))
        exit()

# -----------------------------------------------------------------------------
def optical_flow(img0, img1, feature_params, lk_params):
    gray0 = cv2.cvtColor(img0, cv2.COLOR_BGR2GRAY)
    gray1 = cv2.cvtColor(img1, cv2.COLOR_BGR2GRAY)

    # Create some random colors
    colors = np.random.randint(0,255,(100,3))
    points0 = cv2.goodFeaturesToTrack(gray0, mask=None, **feature_params)
    points1, status, err = cv2.calcOpticalFlowPyrLK(gray0, gray1, points0, None, **lk_params)

    print("Points:\n", points1)
    print("Status:", status)
    print("Error:", err)

    # Create a mask image for drawing purposes
    mask = np.zeros_like(img1)

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

    points_that_moved = np.sum(status[err > point_motion_threshold])
    print("Points that moved:", points_that_moved)
    print("Total matched points:", len(good0))
    if (points_that_moved / len(good0)) > camera_motion_threshold:
        print("Camera is moving")
        return True, good0, good1
    else:
        print("Camera is stationary")
        return False, good0, good1

# -----------------------------------------------------------------------------
def estimate_focus_of_expansion(points0, points1):
    print("RANSAC'n")

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

    camera_is_moving, points0, points1 = optical_flow(img0, img1, feature_params, lk_params)
    if camera_is_moving:
        estimate_focus_of_expansion(points0, points1)
