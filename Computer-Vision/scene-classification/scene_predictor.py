from sys import argv

import numpy as np

# -----------------------------------------------------------------------------
def arg_parse():
    if len(argv) == 2:
        _, features = argv
        return features
    else:
        print("Invalid Argument(s).")
        print("USAGE: {} <feature-directory>".format(argv[0]))
        exit()

# -----------------------------------------------------------------------------
def load_features(root_dir, kind, names):
    # build feature vector
    X = []
    for name in names:
        features = np.load(root_dir + kind + "_" + name + ".npy")
        X.append(features)

    X = np.array(X)
    X = X.reshape(X.shape[0] * X.shape[1], X.shape[2]) 
    
    # build label vector
    grass = np.zeros(len(features))
    ocean = np.ones(len(features))
    redcarpet = 2 * np.ones(len(features))
    road = 3 * np.ones(len(features))
    wheatfield = 4 * np.ones(len(features))

    y = np.hstack((grass, ocean, redcarpet, road, wheatfield))

    return X, y

# =============================================================================
if __name__ == "__main__":
    feature_directory = arg_parse()
    names = ["grass", "ocean", "redcarpet", "road", "wheatfield"]

    # unpickle all feature vectors
    X_train, y_train = load_features(feature_directory, "train", names)
    X_test, y_test = load_features(feature_directory, "test", names)