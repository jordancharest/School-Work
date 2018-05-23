#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <cassert>
#include <string>
#include <iostream>
#include <limits>
#include "vertex.h"

// ====================================================================
// IMAGE CLASS
// can be saved and loaded from the standard file format .ppm 

class Image {
public:
  // ========================
  // CONSTRUCTOR & DESTRUCTOR
  Image() : width(0), height(0), data(NULL) {}
  Image(const Image &image) { 
    data=NULL;
    CopyHelper(image); }
  const Image& operator=(const Image &image) { 
    if (this != &image)
      CopyHelper(image);
    return *this; }
  ~Image() {
    delete [] data; 
  }

  // =========
  // ACCESSORS
  int Width() const { return width; }
  int Height() const { return height; }
  const Color& GetPixel(int x, int y) const {
    assert(x >= 0 && x < width);
    assert(y >= 0 && y < height);
    return data[y*width + x]; }
  Color& GetPixel(int x, int y) {
    assert(x >= 0 && x < width);
    assert(y >= 0 && y < height);
    return data[y*width + x]; }

  // =========
  // MODIFIERS
  void SetAllPixels(const Color &value) {
    for (int i = 0; i < width*height; i++) {
      data[i] = value; } }
  void SetPixel(int x, int y, const Color &value) {
    assert(x >= 0 && x < width);
    assert(y >= 0 && y < height);
    data[y*width + x] = value; }

  // ===========
  // LOAD & SAVE
  bool Load(const std::string &filename);
  bool Save(const std::string &filename) const; 
  
private:
  // private helper functions
  void Allocate(int w, int h);
  void CopyHelper(const Image &image);

  // ==============
  // REPRESENTATION
  int width;
  int height;
  Color *data;
};

// ====================================================================
#endif
