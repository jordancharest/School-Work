from sys import argv
import glob
import time

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
    if len(argv) == 3:
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
        img = cv2.imread(img_name, cv2.IMREAD_GRAYSCALE)

        # resize if applicable
        if m > 0 and n > 0:
            img = cv2.resize(img, (m, n))
        
        # normalize
        img = (img - 128) / 128

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

    labels = np.hstack((grass, ocean, redcarpet, road, wheatfield))
    print(labels.shape)

    # Binarize the labels
    lb = LabelBinarizer()
    y = lb.fit_transform(labels)
    print(y.shape)

    return y

# -----------------------------------------------------------------------------
def train(X_train, y_train, m, n, learning_rate=1e-4, momentum=0.9):
    #  Set parameters to control the process
    epochs = 1000
    batch_size = 16
    print(X_train.size())
    n_train = X_train.size()[0]
    n_batches = int(np.ceil(n_train / batch_size))
    log_interval = 10

    print("# batches:", n_batches)
    print("# train:", n_train)

    model = FullyConnected(24,36)
    optimizer = optim.Adam(model.parameters(), lr=learning_rate)
    criterion = nn.MSELoss()

    for epoch in range(epochs):
        #  Create a random permutation of the indices of the row vectors
        indices = torch.randperm(n_train)
        
        #  Run through each mini-batch
        for batch_idx, b in enumerate(range(n_batches)):

            # use the last batch of each epoch as validation set
            if batch_idx == n_batches-1:
                test(model, criterion, X_train[batch_indices], y_train[batch_indices])
                continue

            #  Use slicing (of the pytorch Variable) to extract the
            #  indices and then the data instances for the next mini-batch
            batch_indices = indices[b*batch_size: (b+1)*batch_size]
            batch_X = X_train[batch_indices]
            batch_y = y_train[batch_indices]
            
            # zero gradients, make prediction on the batch
            optimizer.zero_grad()
            y_pred = model(batch_X)

            # compute loss and back-propagate
            loss = criterion(y_pred, batch_y)
            loss.backward()
            optimizer.step()

        if epoch % log_interval == 0:
            print('Train Epoch: {} [{}/{} ({:.0f}%)]\tLoss: {:.6f}'.format(
                    epoch, (batch_idx+1) * batch_size, n_train,
                           100. * batch_idx / n_batches, loss.data[0]))

    return model, optimizer, criterion


# -----------------------------------------------------------------------------
def test(model, criterion, X_test, y_test):
    # run a test loop
    test_loss = 0
    correct = 0

    for X, y in zip(X_test, y_test):
        y_pred = model(X)

        # sum up batch loss
        test_loss += criterion(y_pred, y).data.item()
        pred = y_pred.data.max(0)[1]  # get the index of the max in the prediction tensor

        # check that the max of the predicted is the same as the label
        correct += pred.eq(y.data.max(0)[1]).sum()

    test_loss /= X_test.size()[0]
    print('Test set: Average loss: {:.4f}, Accuracy: {}/{} ({:.0f}%)'.format(
            test_loss, correct, X_test.size()[0],
            100. * correct / X_test.size()[0]))


# =============================================================================
if __name__ == "__main__":
    training_data, test_data = arg_parse()

    m = 24
    n = 36

    # data extraction
    names = ["grass", "ocean", "redcarpet", "road", "wheatfield"]
    training_img_names = get_img_names(training_data)

    X_train = []
    y_train = []
    y_train = make_label_vectors(len(training_img_names[0]))
    print(y_train)
    for j, img_set in enumerate(training_img_names):
        start = time.time()
        X, y = preprocess(img_set, m, n)
        print(round(time.time()-start, 2), "seconds to extract", names[j], "train features...")
        print(X.shape)
        X_train.append(X)

    X_train = np.array(X_train)
    X_train = X_train.reshape(X_train.shape[0] * X_train.shape[1], X_train.shape[2]) 

    X_train = Variable(torch.Tensor(X_train))
    y_train = Variable(torch.Tensor(y_train)) 
    model, criterion = train(X_train, y_train, m, n)





    # extract the test images and labels
    test(model, criterion, X_test, y_test)

    

    # learning