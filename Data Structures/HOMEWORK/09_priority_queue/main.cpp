#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstdlib>

#include "mesh.h"

#define NUM_DEBUG_COLLAPSES 10

int main(int argc, char* argv[]) {

  // default values
  int rows = 10;
  int cols = 10;
  std::string image = "sunflowers.ppm";
  int target = 150;
  std::string which = "shortest";
  std::string method = "linear";
  bool preserve_area = false;
  bool debug = false;

  // process the command line arguments
  for (int i = 1; i < argc; i++) {
    if (std::string(argv[i]) == std::string("-image")) {
      assert (i+1 < argc);
      image = argv[i+1];
      i+=1;
    } else if (std::string(argv[i]) == std::string("-dimensions")) {
      assert (i+2 < argc);
      cols = atoi(argv[i+1]);
      rows = atoi(argv[i+2]);
      i+=2;
    } else if (std::string(argv[i]) == std::string("-target")) {
      assert (i+1 < argc);
      target = atoi(argv[i+1]);
      i+=1;
    } else if (std::string(argv[i]) == std::string("-shortest")) {
      which = "shortest";
    } else if (std::string(argv[i]) == std::string("-random")) {
      which = "random";
    } else if (std::string(argv[i]) == std::string("-color")) {
      which = "color";
    } else if (std::string(argv[i]) == std::string("-preserve_area")) {
      preserve_area = true;
    } else if (std::string(argv[i]) == std::string("-debug")) {
      debug = true;
    } else if (std::string(argv[i]) == std::string("-linear")) {
      method = "linear";
    } else if (std::string(argv[i]) == std::string("-priority_queue")) {
      method = "priority_queue";
    } else {
      std::cerr << "ERROR: unknown argument " << argv[i] << std::endl;
      exit(0);
    }
  }

  // clean up files (if any) from previous run
  std::remove("mesh_original.html");
  for (int i = 1; i <= NUM_DEBUG_COLLAPSES; i++) {
    std::stringstream ss;
    ss << "mesh_collapse_" << std::setfill('0') << std::setw(2) << i << ".html";
    std::remove(ss.str().c_str());
  }
  std::remove("mesh_final.html");

  // create the mesh
  Mesh m(image, rows, cols, which, method, preserve_area, debug);
  std::string prev_svg="";
  std::string current_svg="mesh_original.html";
  std::string next_svg="mesh_final.html";
  if (debug) { next_svg = "mesh_collapse_01.html"; }
  m.CreateSVG(current_svg,prev_svg,next_svg);
  std::cout << "ORIGINAL:          " << m;

  if (debug) {
    // do a small number of collapses one at a time, to visualize the process
    for (int i = 1; i <= NUM_DEBUG_COLLAPSES; i++) {
      prev_svg=current_svg;
      current_svg = next_svg;
      if (i < NUM_DEBUG_COLLAPSES) {
        std::stringstream ss;
        ss << "mesh_collapse_" << std::setfill('0') << std::setw(2) << i+1 << ".html";
        next_svg = ss.str();
      } else {
        next_svg="mesh_final.html";
      }
      m.Collapse();
      m.CreateSVG(current_svg,prev_svg,next_svg);
      std::cout << "AFTER COLLAPSE " << std::setw(2)<< i << ": " << m;
      m.Check();
    }
  }

  // then finish the simplification
  m.Simplify(target);
  std::cout << "AFTER SIMPLIFY:    " <<  m;
  prev_svg=current_svg;
  current_svg = next_svg;
  next_svg = "";
  m.CreateSVG(current_svg,prev_svg,next_svg);
}
