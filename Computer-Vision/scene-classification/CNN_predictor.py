from sys import argv


# -----------------------------------------------------------------------------
def arg_parse():
    if len(argv) == 2:
        _, training_data = argv
        return training_data, None
    elif len(argv) == 3:
        _, training_data, test_data = argv
        return training_data, test_data
    else:
        print("Invalid Argument(s).")
        print("USAGE: {0} <training-data> <test-data>".format(argv[0]))
        exit()

# -----------------------------------------------------------------------------
def get_img_names(root_dir):
    grass = glob.glob(root_dir + "/grass/*.JPEG")
    ocean = glob.glob(root_dir + "/ocean/*.JPEG")
    redcarpet = glob.glob(root_dir + "/redcarpet/*.JPEG")
    road = glob.glob(root_dir + "/road/*.JPEG")
    wheatfield = glob.glob(root_dir + "/wheatfield/*.JPEG")

    print("Grass:", len(grass), "images")
    print("Ocean:", len(ocean), "images")
    print("Red carpet:", len(redcarpet), "images")
    print("Road:", len(road), "images")
    print("Wheatfield:", len(wheatfield), "images")
    print()

    return grass, ocean, redcarpet, road, wheatfield

# -----------------------------------------------------------------------------
def preprocess(img_list, m=0, n=0):
    X = []
    y = []
    for img_name in img_list:
        img = cv2.imread(img_name, cv2.IMREAD_COLOR)

        # resize if applicable
        if m > 0 and n > 0:
            img = cv2.resize(img, (m, n))

        # add to the dataset
        X.append(img)

    return np.array(X), np.array(y)

# -----------------------------------------------------------------------------
def get_data(directory):
    img_names = get_img_names(directory)

    # read and preprocess all images
    start = time.time()
    X = []
    for j, img_set in enumerate(img_names):
        X_partial, y_partial = preprocess(img_set, m, n)
        X.append(X_partial)

    # format, shape, and scale
    X = np.array(X)
    X = X.reshape(X.shape[0] * X.shape[1], X.shape[2])

    if scaler == None:
        scaler = StandardScaler()
        X = scaler.fit_transform(X)
    else:
        X = scaler.transform(X)


# =============================================================================
if __name__ == "__main__":
    training_data, test_data = arg_parse()
    
    # Desired image size
    m = 36
    n = 54

    print("\nGetting training data")
    X_train, y_train, X_valid, y_valid, scaler = get_data(training_data, m , n, split=True, scaler=None)



