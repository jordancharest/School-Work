#include <vector>
#include <unordered_map>
#include <iomanip>

#include "image.h"
#include "visualization.h"


typedef std::unordered_map<double, Point> HASHED_IMAGE_T;



// ======================================================================
// Helper function to read the optional arguments and filenames from
// the command line.
void parse_arguments(int argc, char* argv[],
                     std::string& method, int& seed, int& table_size, float& compare,
                     std::vector<std::string>& filenames) {
  for (int i = 1; i < argc; i++) {
    if (argv[i] == std::string("-method")) {
      i++;
      assert (i < argc);
      method = argv[i];
      assert (method == "simple" || method == "hashtable");
    } else if (argv[i] == std::string("-seed")) {
      i++;
      assert (i < argc);
      seed = atoi(argv[i]);
      assert (seed >= 1);
    } else if (argv[i] == std::string("-table")) {
      i++;
      assert (i < argc);
      table_size = atoi(argv[i]);
      assert (table_size >= 1);
    } else if (argv[i] == std::string("-compare")) {
      i++;
      assert (i < argc);
      compare = atof(argv[i]);
      assert (compare > 0.0 && compare <= 1.0);
    } else {
      filenames.push_back(argv[i]);
    }
  }
  assert (filenames.size() > 0);
}


// ======================================================================
// This simple algorithm is rather inefficient, and may not find the
// largest overlapping subregion.  But it will find a subregion match
// of size seed x seed, if one exists.
void SimpleCompare(const Image<int>& a, const Image<int>& b,
                   Image<Color> &out, int which_color,int seed, float& percent,
                   std::vector<std::pair<BoundingBox,BoundingBox> >& regions) {

  // First, find a small seed that matches
  bool found = false;
  Point offset_a;
  Point offset_b;

  // Search over all possible points in image a
  for (int i = 0; i <= a.Width()-seed && !found; i++) {
    for (int j = 0; j <= a.Height()-seed && !found; j++) {
      // Search over all possible points in image b
      for (int i2 = 0; i2 <= b.Width()-seed && !found; i2++) {
        for (int j2 = 0; j2 <= b.Height()-seed && !found; j2++) {
          bool match = true;
          // Check for small seed match
          for (int x = 0; x < seed && match; x++) {
            for (int y = 0; y < seed && match; y++) {
              if (a.GetPixel(i+x,j+y) != b.GetPixel(i2+x,j2+y)) {
                match = false;
              }
            }
          }
          if (match) {
            // break out of these loops!
            HighlightSeed(out,which_color,Point(i,j),seed);
            found = true;
            offset_a = Point(i,j);
            offset_b = Point(i2,j2);
          }
        }
      }
    }
  }
  if (!found) {
    // no match between these images
    percent = 0.0;
    return;
  }

  int width = seed;
  int height = seed;

  // First, expand the region match widthwise, until we hit the right
  // edge of one of the images or a mismatched pixel.
  while (a.GetPixel(offset_a.x + width, offset_a.y) == b.GetPixel(offset_b.x + width, offset_b.y)  &&
         (offset_a.x + width < a.Width()-1  &&  offset_b.x + width < b.Width()-1) ) {

    ++width;
  }

  // Then, expand the region match heightwise, until we hit the top
  // edge of one of the images or a mismatched pixel.
  while (a.GetPixel(offset_a.x, offset_a.y + height) == b.GetPixel(offset_b.x, offset_b.y + height)  &&
         (offset_a.y + height < a.Height()-1  &&  offset_b.y + height < b.Height()-1) ) {

    ++height;
  }


  BoundingBox bbox_a(offset_a,Point(offset_a.x+width,offset_a.y+height));
  BoundingBox bbox_b(offset_b,Point(offset_b.x+width,offset_b.y+height));
  regions.push_back(std::make_pair(bbox_a,bbox_b));
  // return fraction of pixels
  percent = bbox_a.Width()*bbox_a.Height() / float (a.Width()*a.Height());
}

// HASH COMPARE ==========================================================
void HashCompare(HASHED_IMAGE_T& hashed_a, HASHED_IMAGE_T& hashed_b,
                 const Image<int>& a, const Image<int>& b,
                   Image<Color> &out, int which_color, int seed, float& percent,
                   std::vector<std::pair<BoundingBox,BoundingBox> >& regions,
                   float compare) {

    bool match = false;
    //int image_size = a.Width()*a.Height();
    //int num_comparisons = image_size*compare;

    // traverse through one hashed image
    HASHED_IMAGE_T::const_iterator itr;
    int i = 0;
    int x_a_min = a.Width(), y_a_min = a.Height(), x_a_max = 0, y_a_max = 0; // to form the bounding boxes
    int x_b_min = b.Width(), y_b_min = b.Height(), x_b_max = 0, y_b_max = 0;

    for (itr = hashed_a.begin(); i < hashed_a.size(); ++itr, ++i) {
        //if (i%num_comparisons != 0) continue;

        // check if one pixel hash is equal to a pixel hash in the other image
        if (hashed_b.count(itr->first)) {
            match = true;

            // the two images have matching seeds
            //std::cout << "Seeds match!\n";
            HighlightSeed(out,which_color,itr->second,seed);

            // resize the bounding box for image a
            x_a_max = std::max(x_a_max, itr->second.x);
            x_a_min = std::min(x_a_min, itr->second.x);

            y_a_max = std::max(y_a_max, itr->second.y);
            y_a_min = std::min(y_a_min, itr->second.y);


            // resize the bounding box for image b
            x_b_max = std::max(x_b_max, hashed_b[itr->first].x);
            x_b_min = std::min(x_b_min, hashed_b[itr->first].x);

            y_b_max = std::max(y_b_max, hashed_b[itr->first].y);
            y_b_min = std::min(y_b_min, hashed_b[itr->first].y);

        } else {
            //std::cout << "NO MATCH\n";
        }
    }

    if (!match) {
        // no match between these images
        percent = 0.0;
        return;
    }

    /*
    std::cout << "\n Bounding Box A: (" << x_a_min << ", " << y_a_min << ")\n"
              << "\t(" << x_a_max << ", " << y_a_max << ")";


    std::cout << "\n Bounding Box B: (" << x_b_min << ", " << y_b_min << ")\n"
              << "\t(" << x_b_max << ", " << y_b_max << ")";

    */
    BoundingBox bbox_a(Point(x_a_min,y_a_min), Point(x_a_max+seed-1, y_a_max+seed-1));
    BoundingBox bbox_b(Point(x_b_min,y_b_min), Point(x_b_max+seed-1, y_b_max+seed-1));
    regions.push_back(std::make_pair(bbox_a,bbox_b));
    // return fraction of pixels
    percent = bbox_a.Width()*bbox_a.Height() / float (a.Width()*a.Height());

}

// HASH IMAGES ==========================================================
void HashImage(Image<int> const& a, std::vector<HASHED_IMAGE_T> &hashed_images, int seed) {

    // use these as the hash seeds, hopefully will be unique for every pixel seed square
    double sum = 0;
    double salt = 0;
    HASHED_IMAGE_T image_hash;

    // traverse over every pixel in the image
    for (int i = 0; i <= a.Width()-seed; ++i) {
        for (int j = 0; j <= a.Height()-seed; ++j) {
            // traverse over seed sized squares and hash them
            sum = 0;
            salt = 50;
            for (int x = 0; x < seed; x++) {
                for (int y = 0; y < seed; y++) {
                    sum += a.GetPixel(i+x, j+y)*salt + x*y*salt - (x+y)*salt;   // no significant meaning; just attempting to create unique hash values
                    salt += 123.56789;
                }
                salt += 1234.56789;
            }

            // if we have non-unique hashes, the image comparison may create false positives
            if (image_hash.count(sum)) {
                std::cout << "Non-Unique Hash Value!\nRecommend changing the seed size.";
                exit(-1);
            }

            image_hash[sum] = Point(i,j);
        }
    }

    hashed_images.push_back(image_hash);
}

// MAIN =================================================================
int main(int argc, char* argv[]) {

  // default command line argument values
  std::string method = "simple";
  int seed = 5;
  int table_size = 1000000;
  float compare = 0.05;
  std::vector<std::string> filenames;
  parse_arguments(argc,argv,method,seed,table_size,compare,filenames);


  // Load all of the images
  std::vector<Image<int> > images(filenames.size());
  for (int i = 0; i < filenames.size(); i++) {
    images[i].Load(filenames[i]);
  }

  // hash for use later
  std::vector<HASHED_IMAGE_T> hashed_images;

  if (method == "hashtable") {
    for (int i = 0; i < filenames.size(); ++i) {
        HashImage(images[i], hashed_images, seed);
    }
  }

  // Loop over all input images, comparing to every other input image
  for (int a = 0; a < filenames.size(); a++) {
    std::cout << filenames[a] << std::endl;
    // prepare a color visualization image for each input file
    Image<Color> out;
    InitializeOutputFile(images[a],out);
    int which_color = -1;
    for (int b = 0; b < filenames.size(); b++) {
      if (a == b) continue;
      which_color++;

      // Find the one (or more!) subregions that match between this pair of images
      std::vector<std::pair<BoundingBox,BoundingBox> > regions;
      float percent = 0.0;
      if (method == "simple") {
        SimpleCompare(images[a],images[b],out,which_color,seed,percent,regions);
      } else {
        assert (method == "hashtable");
        HashCompare(hashed_images[a], hashed_images[b], images[a], images[b], out, which_color, seed, percent, regions, compare);
      }

      std::cout << std::right << std::setw(7) << std::fixed << std::setprecision(1)
                << 100.0 * percent << "% match with " << std::left << std::setw(20) << filenames[b];

      for (int i = 0; i < regions.size(); i++) {
        std::cout << "   " << regions[i].first << " similar to " << regions[i].second;
        // add the frame data to the visualization image
        DrawBoundingBox(out,regions[i].first,which_color);
      }
      std::cout << std::endl;
      // Save the visualization image
      std::string f = "output_" + filenames[a].substr(0,filenames[a].size()-4) + ".ppm";
      out.Save(f);
    }
  }
}
