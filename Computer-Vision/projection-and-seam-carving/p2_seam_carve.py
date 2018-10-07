from sys import argv


# -----------------------------------------------------------------------------
def arg_parse():
    if len(argv) == 2:
        pass
    else:
        print("Invalid Argument(s).")
        print("USAGE: {0} <img>".format(argv[0]))
        exit()





if __name__ == "__main__":
    img = arg_parse()