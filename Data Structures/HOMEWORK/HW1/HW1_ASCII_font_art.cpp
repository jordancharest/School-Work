/*
User Information:
Command Line Arguments:
argv[1]: "display" or "read"; to display or read the bitmap font
argv[2]: filename for the bitmap font
argv[3]: message to be converted
argv[4]: bitmap foreground character
argv[5]: bitmap background character
*/



#include <iostream>
#include <fstream>
#include <cassert>
#include <vector>
#include <string>
#include <iomanip>
#include <cmath>
#include <cstdlib>



// Functions
void ReadFont(const std::string &font_file,
	      int &width,
	      int &height,
	      std::vector<std::vector<std::string> > &bitmap_letters,
	      std::string foregroundSym,
	      std::string backgroundSym) {

  // open the font file for reading
  std::ifstream istr(font_file.c_str());
  if (!istr) {
    std::cerr << "ERROR: cannot open font file " << font_file << std::endl;
    exit(0);
  }

  // read in the width & height for every character in the file
  istr >> width >> height;
  assert (width >= 1);
  assert (height >= 1);

  // Create a vector to store all 256 ASCII characters of the
  // characters.  Each character is represented as a vector of
  // <height> strings that are each <width> wide.  Initially the
  // characters are unknown (represented with the '?' character).
  bitmap_letters = std::vector<std::vector<std::string> >
    ( 256, std::vector<std::string> ( height, std::string(width, '?')));

  // read in all the characters
  // first is the ascii integer representation of the character
  int ascii;
  while (istr >> ascii) {
    assert (ascii >= 0 && ascii < 256);
    // next the character is printed in single quotes
    char c;
    istr >> c;
    assert (c == '\'');
    // use std::noskipws to make sure we can read the space character correctly
    istr >> std::noskipws >> c;
    // verify that the ascii code matches the character
    assert (c == (char)ascii);
    // switch back to std::skipws mode
    istr >> std::skipws >> c;
    assert (c == '\'');
    // read in the letter
    std::vector<std::string> bitmap;
    std::string tmp;
    for (int i = 0; i < height; i++) {
      istr >> tmp;
      assert ((int)tmp.size() == width);
      // make sure the letter uses only '#' and '.' characters
      for (unsigned int j = 0; j < tmp.size(); j++) {
        assert (tmp[j] == '.' || tmp[j] == '#');
      }
      bitmap.push_back(tmp);
    }
    // overwrite the initially unknown letter in the vector
    bitmap_letters[ascii] = bitmap;
  }
}

// MAIN ----------------------------------------------------------------------------------------------
int main (int argc, char* argv[]){

    // Ensure the program was called correctly
    if (argc != 6){
        std::cerr << "ERROR: Incorrect number of arguments";
        return 1;
    }

    // the characters used for the bitmap
    std::string foregroundSym = argv[4];
    std::string backgroundSym = argv[5];

    // read the font file; contains the bitmap conversion for 256 ASCII characters
    std::string fontFile = argv[2];
    int width = 0;
    int height = 0;
    std::vector<std::vector<std::string> > bitmapLetters;

    ReadFont(fontFile, width, height, bitmapLetters, foregroundSym, backgroundSym);

    // print lines out from the bitmap vector
    // Note the string is printed line by line and not by letter
    std::string conversionString = argv[3];
    for (int row=0; row<height; row++){
        for (char ascii : conversionString){
            for (int i=0; i<width; i++){
                if (bitmapLetters[ascii][row][i] == '.'){
                    std::cout << backgroundSym;
                } else if (bitmapLetters[ascii][row][i] == '#') {
                    std::cout << foregroundSym;
                }
            }
            std::cout <<backgroundSym;
        }
        std::cout<<'\n';
    }
}
