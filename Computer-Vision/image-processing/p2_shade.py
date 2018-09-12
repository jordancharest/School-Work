from sys import argv
import numpy as np
import cv2

# parse command line arguments ------------------------------------------------
def arg_parse():
    if (len(argv) == 4):
        _, in_img, out_img, direction = argv
        directions = ["left", "top", "right","bottom", "center"]
        
        if direction not in directions:
            print("Unknown direction")
            exit()
            
        return in_img, out_img, direction
    
    else:
        print("Invalid Arguments")
        exit()


# generate shader -------------------------------------------------------------
def generate_shader(img, direction):
    if direction == "left" or direction == "right":
        shader = np.arange(img.shape[1]).reshape(1,img.shape[1])
        shader = np.repeat(shader, img.shape[0], axis=0)
        if direction == "left":
            shader = np.fliplr(shader)
            
    elif direction == "top" or direction == "bottom":
        shader = np.arange(img.shape[0]).reshape(img.shape[0], 1)
        shader = np.repeat(shader, img.shape[1], axis=1)
        if direction == "top":
            shader = np.flipud(shader)
            
    elif direction == "center":
        center = (img.shape[0]//2, img.shape[1]//2)
        y,x = np.ogrid[0:img.shape[0], 0:img.shape[1]]
        shader = np.sqrt((center[0] - y)**2  +  (center[1] - x)**2)
     
    # normalize from 0 to 1
    shader = np.true_divide(shader, shader.max())
    
    # convert distances into a multiplier array
    if direction == "center":
        shader = 1-shader
         
    return shader

# MAIN ========================================================================
if __name__ == "__main__":
    in_img, out_img, direction = arg_parse()
    img = cv2.imread(in_img, cv2.IMREAD_COLOR)
    
    shader = generate_shader(img, direction)    
    shaded = np.zeros(img.shape)
    
    # broadcast the shader to all color channels of the original image
    for i in range(img.shape[-1]):
        shaded[:,:,i] = img[:,:,i] * shader
    
    # concatenate the original and shaded image and write to disk    
    result = np.hstack((img, shaded))
    cv2.imwrite(out_img, result)
    
    # Output
    print("({0},{1}) {2:.3f}".format(0, 0, shader[0,0]))
    print("({0},{1}) {2:.3f}".format(0, img.shape[1]//2, shader[0, img.shape[1]//2]))
    print("({0},{1}) {2:.3f}".format(0, img.shape[1]-1, shader[0, img.shape[1]-1]))
    print("({0},{1}) {2:.3f}".format(img.shape[0]//2, 0, shader[img.shape[0]//2, 0]))
    print("({0},{1}) {2:.3f}".format(img.shape[0]//2, img.shape[1]//2, shader[img.shape[0]//2, img.shape[1]//2]))
    print("({0},{1}) {2:.3f}".format(img.shape[0]//2, img.shape[1]-1, shader[img.shape[0]//2, img.shape[1]-1]))
    print("({0},{1}) {2:.3f}".format(img.shape[0]-1, 0, shader[img.shape[0]-1, 0]))
    print("({0},{1}) {2:.3f}".format(img.shape[0]-1, img.shape[1]//2, shader[img.shape[0]-1, img.shape[1]//2]))
    print("({0},{1}) {2:.3f}".format(img.shape[0]-1, img.shape[1]-1, shader[img.shape[0]-1, img.shape[1]-1]))
    