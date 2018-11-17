from sys import argv

# -----------------------------------------------------------------------------
def arg_parse():
    if len(argv) == 2:
        _, features = argv
        return features
    else:
        print("Invalid Argument(s).")
        print("USAGE: {} <feature-directory>".format(argv[0]))



# =============================================================================
if __name__ == "__main__":
    feauture_directory = arg_parse()