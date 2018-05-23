#include <iostream>
#include <list>
#include <vector>
#include <cassert>


void update(std::list<std::string> &data, const std::vector<std::string> &add, const std::vector<std::string> &remove){
    std::list<std::string>::iterator it;
    bool added = false;


    // iterate through the add list, add if the item is not in the list
    for (int i = 0; i < add.size(); i++){
        if (data.size() != 0){
            for (it = data.begin(); it != data.end(); it++){
                // don't add duplicates to the list
                if (add[i] == *it){
                    added = true;
                    break;

                // place the item if we are in its sorted position
                } else if (add[i] < *it){
                    data.insert(it, add[i]);
                    added = true;
                    break;
                }
            }

            if (!added){
                data.push_back(add[i]);
            }


        } else {
            data.insert(data.begin(), add[i]);
        }
    }

    // iterate through remove and remove if the substring exists in the list item
    for (int i = 0; i < remove.size(); i++){
        for (it = data.begin(); it != data.end(); it++){
            // remove if the substring matches
            if (it->find(remove[i])!= std::string::npos)
                it = data.erase(it);
        }
    }
}



// PRINT ==================================================================================
void print(const std::string &label, const std::list<std::string> &data) {
  std::cout << label;
  for (std::list<std::string>::const_iterator itr = data.begin();
       itr != data.end(); itr++) {
    std::cout << " " << *itr;
  }
  std::cout << std::endl;
}



// MAIN =====================================================================================
int main() {


  // Examples demonstrating STL string find:
  // catfish contains the substring fish
  assert (std::string("catfish").find(std::string("fish")) != std::string::npos);
  // fish contains the substring fish
  assert (std::string("fish").find(std::string("fish")) != std::string::npos);
  // fish does not contain the substring catfish
  assert (std::string("fish").find(std::string("catfish")) == std::string::npos);


  // A simple example:
  std::list<std::string> data;
  data.push_back("antelope");
  data.push_back("catfish");
  data.push_back("giraffe");
  data.push_back("jellyfish");
  data.push_back("llama");
  data.push_back("whale_shark");
  data.push_back("zebra");

  std::vector<std::string> add;
  add.push_back("tiger");
  add.push_back("llama");
  add.push_back("elephant");

  std::vector<std::string> remove;
  remove.push_back("fish");
  remove.push_back("bear");
  remove.push_back("whale");
  remove.push_back("zebra");

  print ("\nbefore:",data);
  update(data,add,remove);
  print ("after: ",data);



  // TEST CASES
  data.clear();
  std::cout << "\nEMPTY LIST";
  print ("\nbefore:",data);
  update(data,add,remove);
  print ("after: ",data);

  data.push_back("fish");
  data.push_back("fish");
  data.push_back("fish");
  std::cout << "\nREMOVING DUPLICATES";
  print ("\nbefore:",data);
  update(data,add,remove);
  print ("after: ",data);


  add.push_back("aardvark");
  add.push_back("zebra");
  std::cout << "\nADDING TO THE END AND BEGINNING";
  print ("\nbefore:",data);
  update(data,add,remove);
  print ("after: ",data);

  remove.push_back("aardvark");
  remove.push_back("ele");
  remove.push_back("ll");
  remove.push_back("ti");
  print ("\nbefore:",data);
  update(data,add,remove);
  print ("after: ",data);

}
