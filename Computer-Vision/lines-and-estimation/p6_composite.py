from sys import argv
import os

# -----------------------------------------------------------------------------
def arg_parse():
    if len(argv) == 5:
        script, directory, out_img, sigma, p = argv
        img_list = os.listdir(directory)
        img_list = [directory +  "/" + name for name in img_list if '.jpg' in name.lower()]
        return img_list, out_img, abs(float(sigma)), abs(float(p))
    else:
        print(len(argv))
        print("Invalid Argument(s).")
        print("USAGE: {0} <directory> <out-img-name> <sigma> <p>".format(argv[0]))
        exit()


# =============================================================================
if __name__ == "__main__":
    img_list, out_img, sigma, p = arg_parse()

    print(img_list)