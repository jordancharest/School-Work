import torch.nn as nn
import torch.nn.functional as F

class FullyConnected(nn.Module):
    def __init__(self, n, m):
        super(FullyConnected, self).__init__()
        self.fc1 = nn.Linear(n * m * 3, 200)
        # self.fc2 = nn.Linear(200, 200)
        self.fc3 = nn.Linear(200, 5)


    def forward(self, x):
        x = F.relu(self.fc1(x))
        # x = F.relu(self.fc2(x))
        x = self.fc3(x)
        return x