from sys import argv
import itertools
import random
import time

import matplotlib.pyplot as plt

from sklearn.metrics import confusion_matrix
from sklearn.model_selection import cross_val_score
from sklearn.model_selection import GridSearchCV
from sklearn.preprocessing import StandardScaler
from sklearn.svm import LinearSVC
from sklearn.svm import SVC
import numpy as np


# -----------------------------------------------------------------------------
def arg_parse():
    if len(argv) == 2:
        _, features = argv
        return features, False, False

    elif len(argv) == 3:
        _, features, parameter = argv
        if parameter == "cross-validate":
            return features, False, True
        elif parameter == "grid-search":
            return features, True, False

    else:
        print("Invalid Argument(s).")
        print("USAGE: {} <feature-directory> [<cross-validate> <grid-search>]".format(argv[0]))
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
def find_best_parameters(X_train, y_train, X_test, y_test):
    # Applying Grid Search to find the best model and the best parameters
    classifier = SVC()
    parameters = [{'C': [0.01, 0.05], 'kernel': ['linear']},
                  {'C': [0.01, 0.05], 'kernel': ['poly']}]
    # parameters = [{'C': [0.1, 1], 'kernel': ['rbf'], 'gamma': [0.1, 0.5, 0.9]}]
    grid_search = GridSearchCV(estimator = classifier,
                               param_grid = parameters,
                               scoring = 'accuracy',
                               cv = 3,
                               verbose = 1,
                               n_jobs = 4)
    grid_search = grid_search.fit(X_train, y_train)
    best_accuracy = grid_search.best_score_
    best_parameters = grid_search.best_params_

    print("Best Accuracy:", best_accuracy)
    print("Found using:", best_parameters)


# -----------------------------------------------------------------------------
def train(X_train, y_train, X_test, y_test):
    n_predict = 10
    random_test = random.sample(range(1, 100), n_predict)

    class_weight = {0:0.5, 1:0.2, 2:0.3, 3:0.1, 4:0.15}
    print("Class weights:")
    print(class_weight)
    # class_weight='balanced'
    # classifier = SVC(kernel='rbf', C = 0.001, gamma=0.01)
    classifier = LinearSVC(C=0.001, class_weight=class_weight, max_iter=25)
    t=time.time()
    classifier.fit(X_train, y_train)
    y_pred = classifier.predict(X_test)

    print(round(time.time()-t, 2), 'seconds to train and predict')

    # Check the score of the SVC
    print('Test Accuracy of SVC = ', round(classifier.score(X_test, y_test), 4))
    t = time.time()
    print('My SVC predicts:     ', classifier.predict(X_test[0:n_predict]))
    print('For these',n_predict, 'labels: ', y_test[0:n_predict])
    print(round(time.time()-t, 5), 'Seconds to predict', n_predict,'labels with SVC')

    return classifier, y_pred

# -----------------------------------------------------------------------------
def cross_validate(classifier, X_train, y_train, cv=10):
    # Apply k-Fold Cross Validation
    print("\nCross-validating with {} folds...".format(cv))
    accuracies = cross_val_score(estimator = classifier, X = X_train, y = y_train, cv = cv)
    print("Mean Accuracy      : {:.3f}".format(accuracies.mean()))
    print("Standard Deviation : {:.3f}".format(accuracies.std()))

# -----------------------------------------------------------------------------
def plot_confusion_matrix(cm, classes,
                          normalize=False,
                          title='Confusion matrix',
                          cmap=plt.cm.Blues):
    """
    This function prints and plots the confusion matrix.
    Normalization can be applied by setting `normalize=True`.
    """
    if normalize:
        cm = cm.astype('float') / cm.sum(axis=1)[:, np.newaxis]
        print("Normalized confusion matrix")
    else:
        print('Confusion matrix, without normalization')

    print(cm)

    plt.imshow(cm, interpolation='nearest', cmap=cmap)
    plt.title(title)
    plt.colorbar()
    tick_marks = np.arange(len(classes))
    plt.xticks(tick_marks, classes, rotation=45)
    plt.yticks(tick_marks, classes)

    fmt = '.2f' if normalize else 'd'
    thresh = cm.max() / 2.
    for i, j in itertools.product(range(cm.shape[0]), range(cm.shape[1])):
        plt.text(j, i, format(cm[i, j], fmt),
                 horizontalalignment="center",
                 color="white" if cm[i, j] > thresh else "black")

    plt.ylabel('True label')
    plt.xlabel('Predicted label')
    plt.tight_layout()
    plt.savefig("confusion_matrix.png")

# =============================================================================
if __name__ == "__main__":
    np.set_printoptions(precision=3)
    feature_directory, grid_search, cross_validation = arg_parse()
    names = ["grass", "ocean", "redcarpet", "road", "wheatfield"]

    # build feature and label vectors
    X_train, y_train = load_features(feature_directory, "train", names)
    X_test, y_test = load_features(feature_directory, "test", names)

    print("{0} {1}-length feature vectors in the training set".format(X_train.shape[0], X_train.shape[1]))
    print("{0} {1}-length feature vectors in the test set".format(X_test.shape[0], X_train.shape[1]))

    # Fit a per-column scaler
    scaler = StandardScaler().fit(X_train)
    # Apply the scaler to X
    X_train = scaler.transform(X_train)
    X_test = scaler.transform(X_test)

    if grid_search:
        print("\nBegin grid search...")
        find_best_parameters(X_train, y_train, X_test, y_test)
    else:
        print("\nBegin training...")
        classifier, y_pred = train(X_train, y_train, X_test, y_test)

        # cross-validate
        if cross_validation:
            cross_validate(classifier, X_train, y_train)

        # Confusion matrix
        cm = confusion_matrix(y_test, y_pred)
        print("\nConfusion Matrix:")
        print(cm)
        # plot_confusion_matrix(cm,classes=names)

