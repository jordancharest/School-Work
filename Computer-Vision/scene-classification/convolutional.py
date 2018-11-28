import torch.nn as nn
import torch.nn.functional as F

class Convolutional(nn.Module):
    def __init__(self, m, n):
        super(Convolutional, self).__init__()
        self.size_after_conv = (m//12) * (n//12) * 64

        # input_channels = 3, output_channels = 16
        self.conv1 = nn.Conv2d(3, 16, kernel_size=8, stride=4, padding=2)
        self.conv2 = nn.Conv2d(16, 32, kernel_size=4, stride=2, padding=2)
        self.conv3 = nn.Conv2d(32, 64, kernel_size=4, stride=2, padding=2)
        self.dropout1 = nn.Dropout(p=0.2)
        self.dropout2 = nn.Dropout(p=0.5)
        self.fc1 = nn.Linear(self.size_after_conv, 128)
        self.fc2 = nn.Linear(128, 5)

        self.m = m
        self.n = n

    def forward(self, x):
        x = F.elu(self.conv1(x))
        x = F.elu(self.conv2(x))
        x = F.elu(self.conv3(x))
        x = x.view(-1, self.size_after_conv)
        x = self.dropout1(x)
        x = F.elu(self.fc1(x))
        x = self.dropout2(x)
        x = self.fc2(x)

        return x