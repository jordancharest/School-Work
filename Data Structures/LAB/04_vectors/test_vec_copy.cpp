#include <iostream>
#include <cmath>
using namespace std;

#include "vec.h"

// REMOVE MATCHING ELEMENTS -----------------------------------------------
template <class T> int remove_matching_elements(Vec<T> &returnVec, T element){
    int total = 0;
    Vec<T> tempVec;
    Vec<int>::size_type i;

    for (i=0; i<returnVec.size(); i++){
        if (returnVec[i] == element){
            returnVec.erase(returnVec.begin()+i);
            total++;
            i--;        // removal of an element shifts everything forward, need to recheck vec[i]

            if (returnVec.size()*2 < returnVec.sizeAllocated())
                returnVec.resize(returnVec.size(), 0);

        }
    }

    return total;
}

// PRINT
template <class T> void print(Vec<T> &myVec){
    cout << "\n\nPRINTING VECTOR INFORMATION\n"
         << "First element in the array (m_data): "
         << *myVec.begin()
         << "\nAmount of dynamically allocated space: "
         << myVec.sizeAllocated()
         << "\nNumber of initiated elements: "
         << (myVec.end() - myVec.begin());
}



// MAIN -------------------------------------------------------------------
int main() {

  // ---------------------------------------------------
  // initialize v1 with 10 values...  the multiples of 5
  Vec<int> v1( 10, 0 );
  Vec<int>::size_type i;
  for ( i = 0; i < v1.size(); i++) {
    v1[i] = 5 * i;
  }
  cout << "v1.size() = " << v1.size() << ".  Should be 10.\n";
  cout << "Contents of v1 (multiples of 5):";
  for ( i = 0; i<v1.size(); ++i ) {
    cout << " " << v1[i];
  }
  cout << endl;

  // --------------------------------------------------------------------------
  // make v2 be a copy of v1, but then overwrite the 2nd half with the 1st half
  Vec<int> v2( v1 );
  v2[ 9 ] = v2[ 0 ];
  v2[ 8 ] = v2[ 1 ];
  v2[ 7 ] = v2[ 2 ];
  v2[ 6 ] = v2[ 3 ];
  v2[ 5 ] = v2[ 4 ];

  Vec<int> v4( v2 );

  cout << "Contents of v1 (still multiples of 5):";
  for ( i = 0; i<v1.size(); ++i )
    cout << " " << v1[i];
  cout << endl;
  cout << "Contents of v2 (now palindrome):";
  for ( i = 0; i<v2.size(); ++i )
    cout << " " << v2[i];
  cout << endl;

  // ------------------------------------------
  // make v3 be a copy of v2, but then clear it
  Vec<int> v3;
  v3 = v2;
  v3.clear();
  cout << "\nAfter copying v2 to v3 and clearing v3,  v2.size() = "
       << v2.size() << " and v3.size() = " << v3.size() << endl;
  cout << "Contents of v2 (should be unchanged):";
  for ( i = 0; i<v2.size(); ++i ) {
    cout << " " << v2[i];
  }
  cout << endl;

  // --------------
  // test push back
  cout << "\nNow testing push_back.  Adding 3, 6, 9 to v2:\n";
  v2.push_back( 3 );
  v2.push_back( 6 );
  v2.push_back( 9 );
  cout << "v2 is now: \n";
  for ( i = 0; i<v2.size(); ++i ) {
    cout << " " << v2[i];
  }
  cout << endl;

  // -----------
  // test resize
  v1.resize(20,100);
  cout << "\nNow testing resize.  Resizing v1 to have 20 elements and v2 to have 2 elements\n";
  cout << "v1 is now (should have 100s at the end): \n";
  for ( i = 0; i<v1.size(); ++i )
    cout << " " << v1[i];
  cout << endl;




  print(v2);
  v2.resize(2,100);
  print(v2);





  cout << "v2 is now: \n";
  for ( i = 0; i<v2.size(); ++i )
    cout << " " << v2[i];
  cout << endl;

  // ------------------------
  // test of a vec of doubles
  cout << "\nStarting from an empty vector, z,  of doubles and doing\n"
       << "5 push_backs\n";
  Vec<double> z;
  for ( i = 0; i<5; ++i )
    z.push_back( sqrt( double(10*(i+1)) ));
  cout << "Contents of vector z: ";
  for ( Vec<double>::size_type j = 0; j < z.size(); j++ )
    cout << " " << z[j];
  cout << endl;



  // Test of remove_matching_elements
  cout << "\n\nNow testing remove_matching_elements non-member function:\n"
       << "Vector before remove matching elements\n";
  for ( i = 0; i<v1.size(); ++i )
    cout << " " << v1[i];

  int totalRemoved;
  int element = 100;

  totalRemoved = remove_matching_elements(v1, element);

  cout << "\nAfter removing " << element << " from the vector:\n"
       << "Number Removed: " << totalRemoved << "\n"
       << "The modified vector:\n";

  for ( i = 0; i<v1.size(); ++i )
    cout << " " << v1[i];


  cout << "\n\nTesting Again\n"
       << "Vector before remove matching elements\n";
  for ( i = 0; i<v4.size(); ++i )
    cout << " " << v4[i];


  element = 15;
  totalRemoved = remove_matching_elements(v4, element);

  cout << "\nAfter removing " << element << " from the vector:\n"
       << "Number Removed: " << totalRemoved << "\n"
       << "The modified vector:\n";

  for ( i = 0; i<v4.size(); ++i )
    cout << " " << v4[i];

    cout << "\n";

  // Testing print
  print(v4);
  print(z);

  cout << "\n";
  for (i = 0; i < 100; i++){
    cout << "\nAmount of dynamically allocated space: "
         << v4.sizeAllocated();
    v4.push_back(0);
  }


  // TESTING remove_matching_elements AGAIN
  cout << "\n\nTESTING REMOVE MATCHING ELEMENTS";

  Vec<int> a;
  a.push_back(10);
  print(a);

  cout << "\n\nTesting Again\n"
       << "Vector before remove matching elements\n";
  for ( i = 0; i<a.size(); ++i )
    cout << " " << a[i];


  element = 10;
  totalRemoved = remove_matching_elements(a, element);

  cout << "\nAfter removing " << element << " from the vector:\n"
       << "Number Removed: " << totalRemoved << "\n"
       << "The modified vector:\n";

  for ( i = 0; i<a.size(); ++i )
    cout << " " << a[i];

  print(a);
  cout << "\n";


  // TESTING THE RESIZE
  Vec<int> b(50, 10);
  b.push_back(5);
  b.push_back(5);
  print(b);

  element = 10;
  cout << "Removing all " << element << "s\n";
  totalRemoved = remove_matching_elements(b, element);
  print(b);


  return 0;
}
