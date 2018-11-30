import torch.nn as nn
import torch.nn.functional as F

class Convolutional(nn.Module):
    def __init__(self, m, n):
        super(Convolutional, self).__init__()
        self.size_after_conv = 640

        # input_channels = 3, output_channels = 16
        self.conv1 = nn.Conv2d(3, 16, kernel_size=8, stride=2, padding=3)
        self.conv2 = nn.Conv2d(16, 32, kernel_size=4, stride=2, padding=1)
        self.conv3 = nn.Conv2d(32, 64, kernel_size=4, stride=1, padding=2)
        self.conv4 = nn.Conv2d(64, 64, kernel_size=3, stride=1, padding=0)
        self.conv5 = nn.Conv2d(64, 128, kernel_size=3, stride=1, padding=0)
        self.conv6 = nn.Conv2d(128, 128, kernel_size=2, stride=1, padding=0)
        self.dropout2 = nn.Dropout(p=0.2)
        self.dropout5 = nn.Dropout(p=0.5)
        self.fc1 = nn.Linear(self.size_after_conv, 128)
        self.fc2 = nn.Linear(128, 5)
        self.bn16 = nn.BatchNorm2d(num_features=16)
        self.bn32 = nn.BatchNorm2d(num_features=32)
        self.bn64 = nn.BatchNorm2d(num_features=64)
        self.bn128 = nn.BatchNorm2d(num_features=128)
        self.max_pool = nn.MaxPool2d(2)

        self.m = m
        self.n = n

    def forward(self, x):
        x = F.elu(self.bn16(self.conv1(x)))
        x = F.elu(self.bn32(self.conv2(x)))
        x = F.elu(self.bn64(self.conv3(x)))
        x = self.max_pool(x)
        x = F.elu(self.bn64(self.conv4(x)))
        x = F.elu(self.bn128(self.conv5(x)))
        x = self.max_pool(x)
        x = F.elu(self.bn128(self.conv6(x)))
        x = x.view(-1, self.size_after_conv)
        x = self.dropout2(x)
        x = F.elu(self.fc1(x))
        x = self.dropout5(x)
        x = self.fc2(x)

        return x