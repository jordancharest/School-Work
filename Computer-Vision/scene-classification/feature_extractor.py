from sys import argv
import glob

import numpy

# tunable parameters
t = 4
bh = 4  # number of blocks in image height direction
bw = 4  # number of blocks in image width direction

def arg_parse():
    if len(argv) == 2:
        _, directory = argv
        return directory

    else:
        print("Invalid Argument(s).")
        print("USAGE: {} <training-data-directory>".format(argv[0]))
        exit()


# =============================================================================
if __name__ == "__main__":
    training_data_dir = arg_parse()

    grass = glob.glob(training_data_dir + "/grass/*.JPEG")
    ocean = glob.glob(training_data_dir + "/ocean/*.JPEG")
    redcarpet = glob.glob(training_data_dir + "/redcarpet/*.JPEG")
    road = glob.glob(training_data_dir + "/road/*.JPEG")
    wheatfield = glob.glob(training_data_dir + "/wheatfield/*.JPEG")
    print("Grass:", len(grass))
    print("Ocean:", len(ocean))
    print("Red carpet:", len(redcarpet))
    print("Road:", len(road))
    print("Wheatfield:", len(wheatfield))
