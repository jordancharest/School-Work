from sys import argv

import cv2
import numpy as np

point_motion_threshold = 5
camera_motion_threshold = 0.8


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
def FoE_RANSAC(pts0, pts1, samples=50, tau=0.75):
    print("RANSAC'n")
    k_max = 0
    best_i, best_j = 0,1
    best_intersection = None

    # RANSAC - pick two random lines, find their intersection, then compute the
    # distance between that intersection and all other lines
    for num in range(samples):
        i,j = np.random.randint(0, len(pts0), 2)

        # don't attempt to find an intersection with itself
        if i == j:
            continue

        # find the intersection of two optical flow lines
        intersection = find_intersection(pts0[i], pts1[i], pts0[j], pts1[j])

        # calculate the distance between every other optical flow line and this intersection
        error = np.cross(pts1-pts0, pts0-intersection) / np.linalg.norm(pts1-pts0)
        error = error**2

        # only keep the points that are less than the tolerance
        error = error[error < tau**2]
        k = len(error)

        # if we have a new best fit, then save it
        if k > k_max:
            best_i, best_j = i,j
            best_intersection = intersection
            k_max = k

            # output statistics about this fit
            print("Sample {0}:".format(num))
            print("indices ({0},{1})".format(i, j))
            print("inliers", k)
            print()

    # calculate inlier and outlier average distances
    distances = np.cross(pts1-pts0, pts0-intersection) / np.linalg.norm(pts1-pts0)
    error = distances**2
    inliers = distances[error < tau**2]
    outliers = distances[error >= tau**2]

    print("avg inlier dist {0:.3f}".format(np.average(inliers)))
    print("avg outlier dist {0:.3f}".format(np.average(outliers)))

    return int(best_intersection[0]), int(best_intersection[1])


# -----------------------------------------------------------------------------
# From here: https://stackoverflow.com/a/42727584
def find_intersection(a1, a2, b1, b2):
    """ 
    Returns the point of intersection of the lines passing through a2,a1 and b2,b1.
    a1: [x, y] a point on the first line
    a2: [x, y] another point on the first line
    b1: [x, y] a point on the second line
    b2: [x, y] another point on the second line
    """
    s = np.vstack([a1,a2,b1,b2])        # s for stacked
    h = np.hstack((s, np.ones((4, 1)))) # h for homogeneous
    l1 = np.cross(h[0], h[1])           # get first line
    l2 = np.cross(h[2], h[3])           # get second line
    x, y, z = np.cross(l1, l2)          # point of intersection
    if z == 0:                          # lines are parallel
        return (float('inf'), float('inf'))
    return (x/z, y/z)


# =============================================================================
if __name__ == "__main__":
    img0, img1 = arg_parse()
    foe_img = np.copy(img1)

    # params for ShiTomasi corner detection
    feature_params = dict( maxCorners = 100,
                           qualityLevel = 0.3,
                           minDistance = 7,
                           blockSize = 7 )
    # Parameters for lucas kanade optical flow
    lk_params = dict( winSize  = (15,15),
                      maxLevel = 2,
                      criteria = (cv2.TERM_CRITERIA_EPS | cv2.TERM_CRITERIA_COUNT, 10, 0.03))

    # determine if camera is moving
    camera_is_moving, pts0, pts1 = optical_flow(img0, img1, feature_params, lk_params)
    
    # if it is, estimate the focus of expansion
    if camera_is_moving:
        FoE = FoE_RANSAC(pts0, pts1)
        foe_img = cv2.circle(foe_img, FoE, radius=10, color=(0,0,255), thickness=-1)
        foe_img = cv2.circle(foe_img, FoE, radius=5, color=(255,255,255), thickness=2)
        cv2.imwrite("focus_of_expansion.png", foe_img)
