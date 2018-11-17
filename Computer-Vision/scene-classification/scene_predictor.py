from sys import argv
import random
import time

from sklearn.svm import LinearSVC
from sklearn.preprocessing import StandardScaler
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

# -----------------------------------------------------------------------------
def train(X_train, y_train, X_test, y_test):
    n_predict = 10
    random_test = random.sample(range(1, 100), n_predict)

    classifier = LinearSVC()
    t=time.time()
    classifier.fit(X_train, y_train)

    print(round(time.time()-t, 2), 'seconds to train...')

    # Check the score of the SVC
    print('Test Accuracy of SVC = ', round(classifier.score(X_test, y_test), 4))
    print(X_test[0].shape)
    t = time.time()
    print('My SVC predicts:     ', classifier.predict(X_test[0:n_predict]))
    print('For these',n_predict, 'labels: ', y_test[0:n_predict])
    print(round(time.time()-t, 5), 'Seconds to predict', n_predict,'labels with SVC')

# =============================================================================
if __name__ == "__main__":
    feature_directory = arg_parse()
    names = ["grass", "ocean", "redcarpet", "road", "wheatfield"]

    # build feature and label vectors
    X_train, y_train = load_features(feature_directory, "train", names)
    X_test, y_test = load_features(feature_directory, "test", names)

    print(X_train.shape)
    print(y_train.shape)
    print(X_test.shape)
    print(y_test.shape)

    train(X_train, y_train, X_test, y_test)


    # # Fit a per-column scaler
    # scaler = StandardScaler().fit(X_train)
    # # Apply the scaler to X
    # X_train = scaler.transform(X_train)
    # X_test = scaler.transform(X_test)