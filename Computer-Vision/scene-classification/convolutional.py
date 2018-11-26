import torch.nn as nn
import torch.nn.functional as F

class Convolutional(nn.Module):
    def __init__(self):
        super(Convolutional, self).__init__()

    def forward(self, x):
        return x