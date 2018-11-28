from sys import argv

import torch
import torch.nn as nn
import torch.optim as optim
import torch.utils.data as data
import torchvision

from torch.autograd import Variable
from torchvision import transforms
from convolutional import Convolutional


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


# # -----------------------------------------------------------------------------
# def preprocess(img_list, m=0, n=0):
#     X = []
#     y = []
#     for img_name in img_list:
#         img = cv2.imread(img_name, cv2.IMREAD_COLOR)

#         # resize if applicable
#         if m > 0 and n > 0:
#             img = cv2.resize(img, (m, n))

#         # add to the dataset
#         X.append(img)

#     return np.array(X), np.array(y)

# # -----------------------------------------------------------------------------
# def get_data(directory):
#     img_names = get_img_names(directory)

#     # read and preprocess all images
#     start = time.time()
#     X = []
#     for j, img_set in enumerate(img_names):
#         X_partial, y_partial = preprocess(img_set, m, n)
#         X.append(X_partial)

#     # format, shape, and scale
#     X = np.array(X)
#     X = X.reshape(X.shape[0] * X.shape[1], X.shape[2])

#     if scaler == None:
#         scaler = StandardScaler()
#         X = scaler.fit_transform(X)
#     else:
#         X = scaler.transform(X)

# -----------------------------------------------------------------------------
def get_data(train_dir, test_dir, m, n):
    train_transforms = transforms.Compose([
        transforms.RandomHorizontalFlip(),
        transforms.Resize((m, n)),
        transforms.ToTensor(),
        transforms.Normalize((0.5,0.5,0.5), (0.5,0.5,0.5))
    ])

    test_transforms = transforms.Compose([
        transforms.Resize((m, n)),
        transforms.ToTensor(),
        transforms.Normalize((0.5, 0.5, 0.5), (0.5, 0.5, 0.5))
    ])


    # extract train data
    train_data = torchvision.datasets.ImageFolder(root=train_dir, transform=train_transforms)

    # randomly split into training and validation sets
    batch_size = 32
    train_size = int(0.8 * len(train_data))
    valid_size = len(train_data) - train_size
    train_data, valid_data = data.random_split(train_data, [train_size, valid_size])

    train_loader = data.DataLoader(train_data, batch_size=batch_size, num_workers=4)
    validation_loader = data.DataLoader(valid_data, batch_size=batch_size, shuffle=True, num_workers=4)

    # extract test data
    test_data = torchvision.datasets.ImageFolder(root=test_dir, transform=test_transforms)
    test_loader  = data.DataLoader(test_data, batch_size=batch_size, shuffle=True, num_workers=4)

    print("Training images:", len(train_loader.dataset))
    print("Validation images:", len(validation_loader.dataset))
    print("Test images:", len(test_loader.dataset))

    # for (x, y)
    print("Image size:", train_loader.dataset[0][0].size())

    return train_loader, validation_loader, test_loader

# -----------------------------------------------------------------------------
def train(train_loader, validation_loader, m, n):
    num_epochs = 10
    learning_rate=1e-4
    log_interval = 1

    # model definition
    model = Convolutional(m, n)    
    optimizer = optim.Adam(model.parameters(), lr=learning_rate)
    criterion = nn.CrossEntropyLoss()

    # Training
    for epoch in range(num_epochs):       
        for step, (x, y) in enumerate(train_loader):
            batch_X = Variable(x)   # batch x (image)
            batch_y = Variable(y)   # batch y (target)

            # run batch through model
            y_pred = model(batch_X)
            loss = criterion(y_pred, batch_y)

            # run backprop
            optimizer.zero_grad()           
            loss.backward()                 
            optimizer.step()

        if epoch % log_interval == 0:
            print("Epoch {}:\n  ".format(epoch), end='')
            test(model, criterion, validation_loader)

    return model, criterion

# -----------------------------------------------------------------------------
def test(model, criterion, data_loader):
    total_acc = 0.0
    total_correct = 0
    total_incorrect = 0
  
    for step, (x, y) in enumerate(data_loader):
        batch_X = Variable(x)   # batch x (image)
        batch_y = Variable(y)   # batch y (target)

        # run batch through model
        y_pred = model(batch_X)
        loss = criterion(y_pred, batch_y)

        acc, correct, incorrect = success_rate(y_pred, batch_y)
        total_acc += acc
        total_correct += correct
        total_incorrect += incorrect

    accuracy = total_acc / step
    print("Validation accuracy:", accuracy)


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
    training_data, test_data = arg_parse()
    
    # Desired image size
    m = 48
    n = 72

    print("\nGetting data")
    train_loader, validation_loader, test_loader = get_data(training_data, test_data, m, n)
    print(train_loader)
    print(validation_loader)
    print(test_loader)
    model, criterion = train(train_loader, validation_loader, m, n)
    # X_train, y_train, X_valid, y_valid, scaler = get_data(training_data, m , n, split=True, scaler=None)



