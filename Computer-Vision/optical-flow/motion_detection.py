from sys import argv

import cv2

# -----------------------------------------------------------------------------
def arg_parse():
    if len(argv) == 3:
        script, img_name1, img_name2 = argv
        img1 = cv2.imread(img_name1, cv2.IMREAD_GRAYSCALE)
        img2 = cv2.imread(img_name2, cv2.IMREAD_GRAYSCALE)
        return img1, img2

    else:
        print("Invalid Argument(s).")
        print("USAGE: {} <image1> <image2>".format(argv[0]))
        exit()

# -----------------------------------------------------------------------------
def optical_flow(img1, img2):
    points1 = cv2.goodFeaturesToTrack(img1, mask=None, **feature_params)

# =============================================================================
if __name__ == "__main__":
    # params for ShiTomasi corner detection
    feature_params = dict( maxCorners = 100,
                           qualityLevel = 0.3,
                           minDistance = 7,
                           blockSize = 7 )
    # Parameters for lucas kanade optical flow
    lk_params = dict( winSize  = (15,15),
                      maxLevel = 2,
                      criteria = (cv.TERM_CRITERIA_EPS | cv.TERM_CRITERIA_COUNT, 10, 0.03))

    img1, img2 = arg_parse()
    optical_flow(img1, img2, feature_params, lk_params)