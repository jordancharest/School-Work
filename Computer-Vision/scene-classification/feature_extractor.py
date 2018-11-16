from sys import argv

import numpy

def arg_parse():
    if len(argv) == 2:
        _, directory = argv
        return directory

    else:
        print("Invalid Argument(s).")
        print("USAGE: {} <training-data-directory>")
        exit()


# =============================================================================
if __name__ == "__main__":
    training_data_dir = arg_parse()

    print(training_data_dir)