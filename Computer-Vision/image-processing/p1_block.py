from sys import argv
import ntpath
import numpy as np
import cv2

# from scipy import ndimage


# parse command line arguments ------------------------------------------------
def arg_parse():
    if (len(argv) == 5):
        script, path, m, n, b = argv
        _, filename = ntpath.split(path)
        split = filename.split(".")
        filename = split[0]
        ext = split[1]
        
        return path, filename, ext, int(m), int(n), int(b)
    else:
        print("Invalid arguments\n")
        exit()
        
# downsample an input image ---------------------------------------------------
def downsample(img, sm, sn):
    downsized = np.ones((m,n), dtype=np.float64)
    for i in range(len(downsized)):
        for j in range(len(downsized[0])):
            downsized[i,j] = np.average(img[int(i*sm):int((i+1)*sm), int(j*sn):int((j+1)*sn)])
         
    return downsized

# upsample an input image -----------------------------------------------------
def upsample(img, b):
    temp =  np.repeat(img, b, axis=1)
    return np.repeat(temp, b, axis=0)
    

# MAIN ========================================================================
if __name__ == "__main__":
    
    # parse args and read image
    path, filename, ext, m, n, b = arg_parse()
    img = cv2.imread(path)
    img = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    
    # Downsize the image
    scale_m, scale_n = img.shape[0]/m, img.shape[1]/n
    downsized = downsample(img, scale_m, scale_n)
    print("Downsized images are", downsized.shape)
    
    # Output
    print("Block images are ({0}, {1})".format(m*b, n*b))
    print("Average intensity at ({0}, {1}) is {2:.2f}".format(m//4, n//4, downsized[m//4, n//4]))
    print("Average intensity at ({0}, {1}) is {2:.2f}".format(m//4, 3*n//4, downsized[m//4, 3*n//4]))
    print("Average intensity at ({0}, {1}) is {2:.2f}".format(3*m//4, n//4, downsized[3*m//4, n//4]))
    print("Average intensity at ({0}, {1}) is {2:.2f}".format(3*m//4, 3*n//4, downsized[3*m//4, 3*n//4]))
    
    # make the binary image
    thresh = np.median(downsized)
    retval, binary = cv2.threshold(downsized, thresh, 255, cv2.THRESH_BINARY)
    print("Binary threshold: {0:.2f}".format(thresh))
    
    # convert gray to int then upsample gray and binary
    downsized = downsized.astype(np.int)
    gray = upsample(downsized, b)
    binary = upsample(binary, b)
    
    # write both to disk
    binary_filename = filename + "_b." + ext
    gray_filename = filename + "_g." + ext
    cv2.imwrite(binary_filename, binary);
    cv2.imwrite(gray_filename, gray)
    print("Wrote image", gray_filename)
    print("Wrote image", binary_filename)