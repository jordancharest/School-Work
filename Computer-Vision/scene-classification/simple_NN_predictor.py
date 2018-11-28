from sys import argv
import glob
import time

from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler
from sklearn.preprocessing import LabelBinarizer
from torch import optim
from torch.autograd import Variable
import torch.nn as nn
import torch
import cv2
import numpy as np

from fully_connected import FullyConnected


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

        # flatten and add to the dataset
        img = np.ravel(img)
        X.append(img)

    return np.array(X), np.array(y)


# -----------------------------------------------------------------------------
def make_label_vectors(length):
    # build label vector
    grass = np.zeros(length)
    ocean = np.ones(length)
    redcarpet = 2 * np.ones(length)
    road = 3 * np.ones(length)
    wheatfield = 4 * np.ones(length)

    y = np.hstack((grass, ocean, redcarpet, road, wheatfield))

    # Binarize the labels
    # lb = LabelBinarizer()
    # y = lb.fit_transform(y)

    return y
# -----------------------------------------------------------------------------
def get_data(directory, m, n, split=False, scaler=None):
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

    # build label vector
    y = make_label_vectors(len(img_names[0]))


    if split:
        X_train, X_valid, y_train, y_valid = train_test_split(X, y, test_size = 0.2, random_state = 0)

    duration = round(time.time()-start, 2)
    print(duration, "seconds to extract images and build label tensor")
    print("Total images:", X.shape[0])
    print("Flattened image size:", X.shape[1])

    if split:
        X_train = Variable(torch.tensor(X_train, dtype=torch.float))
        y_train = Variable(torch.tensor(y_train, dtype=torch.long))
        X_valid = Variable(torch.tensor(X_valid, dtype=torch.float))
        y_valid = Variable(torch.tensor(y_valid, dtype=torch.long))
        return X_train, y_train, X_valid, y_valid, scaler
    else:
        X = Variable(torch.tensor(X, dtype=torch.float))
        y = Variable(torch.tensor(y, dtype=torch.long))
        return X, y

# -----------------------------------------------------------------------------
def train(X_train, y_train, X_valid, y_valid, m, n):
    #  Set parameters to control the process
    num_epochs = 3
    batch_size = 16
    learning_rate=1e-4
    n_train = X_train.size()[0]
    n_batches = int(np.ceil(n_train / batch_size))
    log_interval = 1

    print("\nNumber of batches:", n_batches)

    model = FullyConnected(m,n)
    optimizer = optim.Adam(model.parameters(), lr=learning_rate)
    criterion = nn.CrossEntropyLoss()

    print("\n\nBegin Training...")
    start = time.time()

    for ep in range(num_epochs):
        #  Create a random permutation of the indices of the row vectors.
        indices = torch.randperm(n_train)
        
        #  Run through each mini-batch
        for b in range(n_batches):
            #  Use slicing (of the pytorch Variable) to extract the
            #  indices and then the data instances for the next mini-batch
            batch_indices = indices[b*batch_size:(b+1)*batch_size]
            batch_X = X_train[batch_indices]
            batch_Y = y_train[batch_indices]
            
            #  Run the network on each data instance in the minibatch
            #  and then compute the object function value
            y_pred = model(batch_X)
            loss = criterion(y_pred, batch_Y)
            
            #  Back-propagate the gradient through the network using the
            #  implicitly defined backward function, but zero out the
            #  gradient first.  The step is made here by the optimizer
            optimizer.zero_grad()
            loss.backward()
            optimizer.step()

        if ep % log_interval == 0:
            print("Epoch {}:\n  ".format(ep), end='')
            test(model, criterion, X_valid, y_valid)

    print(round(time.time()-start, 2), "seconds to train\n")
    return model, criterion


# -----------------------------------------------------------------------------
def test(model, criterion, X_test, y_test):
    # make predictions
    y_pred = model(X_test)

    # sum up batch loss and compute accuracy
    loss = criterion(y_pred, y_test)
    accuracy, correct, incorrect = success_rate(y_pred, y_test)

    print('Average loss: {:.4f}, Accuracy: {}/{} ({:.0f}%)'.format(
            loss, correct, X_test.size()[0], accuracy * 100.0))

# -----------------------------------------------------------------------------
def success_rate(pred_Y, Y):
    '''
    Calculate and return the success rate from the predicted output Y and the
    expected output.  There are several issues to deal with.  First, the pred_Y
    is non-binary, so the classification decision requires finding which column
    index in each row of the prediction has the maximum value.  This is achieved
    by using the torch.max() method, which returns both the maximum value and the
    index of the maximum value; we want the latter.  We do this along the column,
    which we indicate with the parameter 1.  Second, the once we have a 1-d vector
    giving the index of the maximum for each of the predicted and target, we just
    need to compare and count to get the number that are different.  We could do
    using the Variable objects themselve, but it is easier syntactically to do this
    using the .data Tensors for obscure PyTorch reasons.
    '''
    _,pred_Y_index = torch.max(pred_Y, -1)
    num_equal = torch.sum(pred_Y_index.data == Y.data).item()
    num_different = torch.sum(pred_Y_index.data != Y.data).item()
    rate = num_equal / float(num_equal + num_different)
    return rate, num_equal, num_different # rate.item()

# =============================================================================
if __name__ == "__main__":
    training_data, test_data= arg_parse()

    # Desired image size
    m = 48
    n = 72

    print("\nGetting training data")
    X_train, y_train, X_valid, y_valid, scaler = get_data(training_data, m , n, split=True, scaler=None)


    model, criterion = train(X_train, y_train, X_valid, y_valid, m, n)

    if test_data:
        print("Getting test data")
        X_test, y_test = get_data(test_data, m, n, split=False, scaler=scaler)
        print("\nRunning through model")
        test(model, criterion, X_test, y_test)