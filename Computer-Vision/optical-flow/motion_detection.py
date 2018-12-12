from sys import argv

from sklearn.cluster import AffinityPropagation
import cv2
import numpy as np

pt_motion_threshold = 5
camera_motion_threshold = 0.8
flow_dist_scale = 2


# -----------------------------------------------------------------------------
def arg_parse():
    if len(argv) == 3:
        script, img_name0, img_name1 = argv
        img0 = cv2.imread(img_name0, cv2.IMREAD_COLOR)
        img1 = cv2.imread(img_name1, cv2.IMREAD_COLOR)
        return img0, img1

    elif len(argv) == 4:
        script, img_name0, img_name1, app = argv
        img0 = cv2.imread(img_name0, cv2.IMREAD_COLOR)
        img1 = cv2.imread(img_name1, cv2.IMREAD_COLOR)
        return img0, img1, app       

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

    # calculate optical flow
    points0 = cv2.goodFeaturesToTrack(gray0, mask=None, **feature_params)
    points1, status, err = cv2.calcOpticalFlowPyrLK(gray0, gray1, points0, None, **lk_params)
    total_pts = np.sum(status)
    print("Optical flow calculated. Found flow for {} points".format(total_pts))

    # Create a mask image for drawing purposes
    mask = np.zeros_like(img1)

    # Select good matches
    good0 = points0[status == 1]
    good1 = points1[status == 1]

    # draw the flow for each point
    for i,(new,old) in enumerate(zip(good1,good0)):
        a,b = new.ravel()
        c,d = old.ravel()
        mask = cv2.line(mask, (a,b), (c,d), colors[i].tolist(), 2)
        img1 = cv2.circle(img1, (a,b), 5, colors[i].tolist(), -1)
    result = img1 + mask

    # determine if the camera is moving
    pts_that_moved = np.sum(status[err > pt_motion_threshold])
    print("Points that moved more than threshold ({0} pixels): {1}"\
        .format(pt_motion_threshold, pts_that_moved))
    motion_ratio = pts_that_moved / total_pts
    print("Ratio: {0}/{1} = {2:.2f}%".format(pts_that_moved, total_pts, motion_ratio*100))
    if motion_ratio > camera_motion_threshold:
        print("Camera is moving. > {}% of points moved".format(camera_motion_threshold * 100))
        return True, good0, good1, err[status == 1], result
    else:
        print("Camera is stationary. <= {}% of points moved".format(camera_motion_threshold * 100))
        return False, good0, good1, err[status == 1], result

# -----------------------------------------------------------------------------
def FoE_RANSAC(pts0, pts1, flow_dist, samples=50, tau=1.0):
    print("\nStarting RANSAC to estimate Focus of Expansion")
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
    inlier_error = distances[error < tau**2]
    outlier_error = distances[error >= tau**2]

    # separate out the outlier points to later cluster them
    # objects are considered to be independently moving if they are RANSAC outliers
    # and they have a motion vector greater than the motion threshold
    independent_pt0 = pts0[(error >= tau**2) & (flow_dist > pt_motion_threshold)]
    independent_pt1 = pts1[(error >= tau**2) & (flow_dist > pt_motion_threshold)]
    moving_independently = np.hstack((independent_pt0, independent_pt1))
    slope = (moving_independently[:,1] - moving_independently[:,3]) /  \
            (moving_independently[:,0] - moving_independently[:,2])
    slope = np.expand_dims(slope, axis=1)
    flow_dist = flow_dist[(error >= tau**2) & (flow_dist > pt_motion_threshold)] * flow_dist_scale
    if len(flow_dist.shape) == 1:
        flow_dist = np.expand_dims(flow_dist, axis=1)

    moving_independently = np.hstack((moving_independently, slope, flow_dist))

    print("RANSAC results:")
    print("avg inlier dist {0:.3f}".format(np.average(inlier_error)))
    print("avg outlier dist {0:.3f}".format(np.average(outlier_error)))

    return int(best_intersection[0]), int(best_intersection[1]), moving_independently


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


# -----------------------------------------------------------------------------
def cluster(data, img):
    print("\nStarting Affinity Propagation to cluster independently moving objects")

    # run affinity propagation
    af = AffinityPropagation(affinity='euclidean', convergence_iter=10, copy=True,
          damping=0.5, max_iter=200, preference=None, verbose=False).fit(data)
    cluster_centers_indices = af.cluster_centers_indices_
    labels = af.labels_

    # use label vector to separate into clusters
    clusters = []
    cluster_size_threshold = 0
    for label in range(np.max(labels)+1):
        cluster = data[labels == label]
        if len(cluster) > cluster_size_threshold:
            print("Cluster {0}: {1} points".format(label, len(cluster)))
            clusters.append(data[labels == label])

    print("Found {} independently moving objects".format(len(clusters)))


    # draw bounding boxes
    colors = np.random.randint(0,255,(len(clusters),3))
    for i, cluster in enumerate(clusters):
        # draw the flow points in this cluster
        for pts in cluster:
            img = cv2.circle(img, (pts[2], pts[3]), radius=5, color=colors[i].tolist(), thickness=-1)

        x_max = max(np.max(cluster[:,0]), np.max(cluster[:,2]))
        x_min = min(np.min(cluster[:,0]), np.min(cluster[:,2]))
        y_max = max(np.max(cluster[:,1]), np.max(cluster[:,3]))
        y_min = min(np.min(cluster[:,1]), np.min(cluster[:,3]))
        img = cv2.rectangle(img, (x_max,y_max), (x_min,y_min), color=colors[i].tolist(), thickness=3)

    return img

# =============================================================================
if __name__ == "__main__":
    img0, img1, app = arg_parse()
    clusters_img = np.copy(img1)

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
    camera_is_moving, pts0, pts1, flow_dist, img = optical_flow(img0, img1, feature_params, lk_params)
    
    # if it is, estimate the focus of expansion
    if camera_is_moving:
        FoE_x, FoE_y, independent = FoE_RANSAC(pts0, pts1, flow_dist)
        img = cv2.circle(img, (FoE_x, FoE_y), radius=10, color=(0,0,255), thickness=-1)
        img = cv2.circle(img, (FoE_x, FoE_y), radius=5, color=(255,255,255), thickness=2)

    # if not, build the feature vector for affinity propagation using all points
    else:
        ind_pt0 = pts0[flow_dist > pt_motion_threshold]
        ind_pt1 = pts1[flow_dist > pt_motion_threshold]
        independent = np.hstack((ind_pt0, ind_pt1))
        slope = (independent[:,1] - independent[:,3]) / \
                (independent[:,0] - independent[:,2])
        slope = np.expand_dims(slope, axis=1)
        flow_dist = flow_dist[flow_dist > pt_motion_threshold] * flow_dist_scale
        if len(flow_dist.shape) == 1:
            flow_dist = np.expand_dims(flow_dist, axis=1)
    
    cv2.imwrite("optical_flow" + app + ".png", img)

    # draw bounding boxes around independently moving objects
    img = cluster(independent, clusters_img)
    cv2.imwrite("independent_motion" + app + ".png", img)