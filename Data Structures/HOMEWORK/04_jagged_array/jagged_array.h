#ifndef JAGGED_ARRAY_H_
#define JAGGED_ARRAY_H_

#include <cstddef>
#include <cstdlib>
#include <iostream>


// TEMPLATED CLASS DEFINITION ============================================================
template <class T> class JaggedArray {

public:
    // TYPEDEFS
    typedef T* iterator;
    typedef const T* const_iterator;
    typedef unsigned int size_type;


    // CONSTRUCTORS, ASSIGNMNENT OPERATOR
    JaggedArray() { this->create(); }
    JaggedArray(size_type n) { this->create(n); }
    JaggedArray(const JaggedArray& J) { copy(J); }
    JaggedArray<T>& operator=(const JaggedArray<T>& J);

    // DESTRUCTOR
    ~JaggedArray() {

        if (totalElements_ != 0 && unpackedValues_){
            for (size_type i=0; i<numBins_; i++){
                // only delete if memory is allocated
                if (counts_[i])
                    delete [] unpackedValues_[i];
            }
        }

        // In C++, calling delete on nullptr does nothing, so we can safely
        // delete every allocated array without verifying if it contains data
        delete [] unpackedValues_;
        delete [] counts_;
        delete [] packedValues_;
        delete [] offsets_;
    }


    // MEMBER MODIFIERS
    void addElement (size_type bin, T element);
    void removeElement(size_type bin, size_type element);
    void clear();
    void unpack();
    void pack();



    // PRINT JAGGED ARRAY
    void print();


    // ACCESSORS (CONSTANT)
    bool isPacked() const { return !unpackedValues_;}
    unsigned int numBins() const { return numBins_; }
    unsigned int numElements() const { return totalElements_; }
    unsigned int numElementsInBin(unsigned int bin) const;
    T getElement(size_type bin, size_type number) const;



private:
    // PRIVATE MEMBER FUNCTIONS
    void create();
    void create(size_type n);
    void copy(const JaggedArray<T>& J);

    // PRIVATE VARIABLES
    size_type* counts_;
    T** unpackedValues_;
    T* packedValues_;
    size_type* offsets_;
    size_type numBins_;
    unsigned int totalElements_;
};
//===============================================================================================

// CREATE - EMPTY CONSTRUCTOR; an empty JaggedArray (null pointers everywhere). --------------------------------------
template <class T>  void JaggedArray<T>::create() {
    counts_ = nullptr;
    unpackedValues_ = nullptr;
    packedValues_ = nullptr;
    offsets_ = nullptr;
    numBins_ = totalElements_ = 0;
}

// CREATE - INTIALIZED CONSTRUCTOR; an initialized JaggedArray ------------------------------------------------------------
template <class T>  void JaggedArray<T>::create(size_type n) {

    // Instantiate the Jagged Array as unpacked (so it can be modified)
    // No elements yet
    counts_ = new size_type [n];
    for(size_type i=0; i<n; i++)
        counts_[i] = 0;

    // all bins are empty
    unpackedValues_ = new T*[n];
    for(size_type i=0; i<n; i++)
        unpackedValues_[i] = nullptr;


    packedValues_ = nullptr;
    offsets_ = nullptr;
    numBins_ = n;
    totalElements_ = 0;
}


// OPERATOR= Assign one vector to another, avoiding duplicate copying. -------------------------
template <class T> JaggedArray<T>& JaggedArray<T>::operator=(const JaggedArray<T>& J) {

    // don't allow self-copying
    if (this != &J) {

        // if the Jagged Array being assigned already holds data,
        // delete it before copying over the new data
        if (!this-> isPacked()){
            delete [] counts_;

            if (totalElements_ != 0){
                for (size_type i=0; i<numBins_; i++)
                    delete [] unpackedValues_[i];
            }
            delete [] unpackedValues_;


        // else J is packed
        } else if (this-> isPacked()) {
            delete [] packedValues_;
            delete [] offsets_;
        }


        this -> copy(J);
      }

  return *this;
}


// COPY Create the Jagged Array as a copy of the argument Jagged Array. --------------------------------------
template <class T> void JaggedArray<T>::copy(const JaggedArray<T>& J) {

    this-> totalElements_ = J.totalElements_;
    this-> numBins_ = J.numBins_;

    // Copy different arrays dependent upon if JaggedArray is packed or unpacked
    if (!J.isPacked()){
        this-> counts_ = new size_type [this-> numBins_];
        this-> unpackedValues_ = new T*[this-> numBins_];

        // for each bin
        for (size_type bin = 0; bin < numBins_; bin++){
            this-> counts_[bin] = J.counts_[bin];

            // don't allocate to an empty array
            if (this-> numElementsInBin(bin) != 0)
                this-> unpackedValues_[bin] = new T [this-> numElementsInBin(bin)];
            else
                this-> unpackedValues_[bin] = nullptr;

            // copy the elements in that bin
            for (size_type j = 0; j < this-> numElementsInBin(bin); j++)
                this-> unpackedValues_[bin][j] = J.unpackedValues_[bin][j];

        }

        this-> offsets_ = nullptr;
        this-> packedValues_ = nullptr;

    // else the Jagged Array is packed
    } else if (J.isPacked()) {
        this-> offsets_ = new size_type [this-> numBins_];
        packedValues_ = new T [this-> totalElements_];

        // copy the data
        for (size_type i=0; i < this->numBins_; i++)
            this-> offsets_[i] = J.offsets_[i];

        for (size_type i=0; i < this->totalElements_; i++)
            this-> packedValues_[i] = J.packedValues_[i];


        this-> counts_ = nullptr;
        this-> unpackedValues_ = nullptr;
    }
}



// NUMBER OF ELEMENTS IN BIN ------------------------------------------------------------------
template <class T> unsigned int JaggedArray<T>::numElementsInBin(unsigned int bin) const {

    if(unpackedValues_){
        return counts_[bin];

    } else {    // the data structure is packed
        if (bin < (numBins_-1) && bin >= 0){
            return (offsets_[bin+1] - offsets_[bin]);

        } else if (bin == numBins_-1){
            return (totalElements_-offsets_[bin]);

        } else {
            std::cerr << "ERROR: Invalid bin number";
            exit(1);
        }
    }
}


// GET ELEMENT -------------------------------------------------------------------------------
template <class T> T JaggedArray<T>::getElement(size_type bin, size_type element) const {

    if (unpackedValues_){     // if the data structure is unpacked
       return unpackedValues_[bin][element];


    } else {        // the data structure is packed

        if (this -> numElementsInBin(bin) == 0){
            std::cerr << "ERROR: No elements in that bin";
            exit(1);
        }


        if (bin == 0){
            return packedValues_[element];

        } else if (bin < (numBins_) && bin > 0) {
            return packedValues_[offsets_[bin] + element];

        } else {
            std::cerr << "ERROR: Invalid bin number";
            exit(1);
        }
    }
}

// ADD ELEMENT ------------------------------------------------------------------------------
template <class T>  void JaggedArray<T>::addElement(size_type bin, T element) {

    if (unpackedValues_) {     // if the data structure is unpacked and the element isn't nothing
        T* newPointer = new T[counts_[bin]+1];  // allocate a new array for the specified bin that is one element larger

        // copy the current bin elements into a temporary pointer
        size_type i;
        for (i=0; i<(counts_[bin]); ++i)
            newPointer[i] = unpackedValues_[bin][i];


        // add the element to the end of the new, larger bin
        newPointer[i] = element;


        // delete the old bin and assign the new bin in its place
        if (counts_[bin] != 0)
            delete [] unpackedValues_[bin];
        unpackedValues_[bin] = newPointer;

        counts_[bin]++;
        totalElements_++;


    } else {        // the data structure is packed and we cannot modify it
        std::cerr << "Cannot modify a packed Jagged Array";
        exit(1);
    }
}


// REMOVE ELEMENT --------------------------------------------------------------------------
template <class T>  void JaggedArray<T>::removeElement(size_type bin, size_type element){

    if (unpackedValues_){     // if the data structure is unpacked

        if ((counts_[bin] == 0) || (element > counts_[bin])){
            std::cerr << "ERROR: That element does not exist";
            exit(1);
        }

        // allocate a new array for the specified bin that is one element smaller
        T* newPointer = new T[counts_[bin]-1];

        // fill the new array with values from the old, but skip the removed element
        size_type index = 0;
        for (size_type i = 0; i < counts_[bin]; i++){
            if (i != element) {
                newPointer[index] = unpackedValues_[bin][i];
                index++;
            }
        }

        // assign the new pointer
        delete [] unpackedValues_[bin];
        unpackedValues_[bin] = newPointer;

        counts_[bin]--;
        totalElements_--;

    } else {        // the data structure is packed and we cannot modify it
        std::cerr << "ERROR: Cannot modify a packed Jagged Array";
        exit(1);
    }

}


// CLEAR ----------------------------------------------------------------------------------
template <class T>  void JaggedArray<T>::clear(){

    if (unpackedValues_){
        totalElements_ = 0;

        for (size_type i=0; i<numBins_; i++){
            delete [] unpackedValues_[i];
            counts_[i] = 0;
        }
    } else {
        std::cerr << "ERROR: Cannot modify a packed Jagged Array";
        exit(1);
    }
}


// UNPACK ---------------------------------------------------------------------------------
template <class T>  void JaggedArray<T>::unpack(){

    // don't pack if it already is
    if(!this-> isPacked()) return;


    // WARNING: Easiest way to unpack is to fill count_ first (since numElementsInBin()
    // works on unpacked and packed versions). This allows us to blindly fill unpackedValues_.
    // Instantiating unpackedValues_ first requires significant code changes
    counts_ = new size_type [numBins_];
    for(size_type i = 0; i < numBins_; i++){
        counts_[i] = this -> numElementsInBin(i);
    }

    int index = 0;
    unpackedValues_ = new T*[numBins_];

    // for every bin
    for(size_type bin = 0; bin < numBins_; bin++){

        // first check if allocated space is needed
        if (counts_[bin])
            unpackedValues_[bin] = new T [counts_[bin]];
        else
            unpackedValues_[bin] = nullptr;

        // then blindly fill unpackedValues_ in order from packedValues_
        // works because counts_ was created first, and this loop will
        // skip empty bins, (empty bins = nullptr)
        for(size_type element = 0; element < counts_[bin]; element++){
            unpackedValues_[bin][element] = packedValues_[index];
            index++;
        }
    }

    delete [] packedValues_;
    delete [] offsets_;

    packedValues_ = nullptr;
    offsets_ = nullptr;
}


// PACK -----------------------------------------------------------------------------------
template <class T>  void JaggedArray<T>::pack(){

    // don't pack if it already is
    if(this-> isPacked()) return;

    packedValues_ = new T [totalElements_];
    offsets_ = new size_type [numBins_];
    int index = 0;

    // place the elements into packed_, in order
    for (size_type i=0; i < numBins_; i++){
        for(size_type j=0; j < counts_[i]; j++){
            packedValues_[index] = unpackedValues_[i][j];
            index++;
        }
    }

    // bizarre packing structure specified in the homework
    // don't bother questioning, it works
    offsets_[0] = 0;
    size_type i;
    for (i = 1; i < (numBins_); i++){
        offsets_[i] = offsets_[i-1] + counts_[i-1];
    }


    // delete all allocated memory
    for (size_type i=0; i<numBins_; i++)
        delete [] unpackedValues_[i];

    delete [] unpackedValues_;
    delete [] counts_;

    unpackedValues_ = nullptr;
    counts_ = nullptr;
}


// PRINT ---------------------------------------------------------------------------------
template <class T>  void JaggedArray<T>::print(){
    // Prints the data structure in a nice formatted way

    // if the Jagged Array is unpacked
    if (unpackedValues_ && totalElements_ > 0){
        std::cout << "\nunpacked Jagged Array\n"
                  << "  num_bins: " << numBins_ << "\n"
                  << "  num_elements: " << totalElements_ << "\n"
                  << "  counts:  ";

        for(size_type i=0; i<numBins_; i++){
            std::cout << counts_[i] << " ";
        }
        std::cout << "\n  values:  ";

        size_type bin = 0;
        size_type element = 0;
        size_type elementsPrinted = 0;
        while(elementsPrinted < totalElements_) {
            if (bin == (numBins_)){
                    bin = 0;
                    element++;
                    std::cout << "\n           ";
            }


            if (element < counts_[bin]){
                std::cout << unpackedValues_[bin][element] << " ";
                elementsPrinted++;

            } else if (element >= counts_[bin]){
                std::cout << "  ";
            }
            bin++;
        }

        std::cout << "\n";
    } else if (totalElements_ == 0){
        std::cout << "\nThe Jagged Array is empty\n";

    // else it is packed
    } else  if (!unpackedValues_) {
        std::cout <<"\npacked Jagged Array\n"
        << "  num_bins: " << numBins_ << "\n"
        << "  num_elements: " << totalElements_ << "\n"
        << "  offsets:  ";

        for(size_type i=0; i<numBins_; i++){
            std::cout << offsets_[i] << " ";
        }
        std::cout << "\n  values:   ";

        for(size_type i=0; i<totalElements_; i++){
            std::cout << packedValues_[i] << " ";
        }
        std::cout << "\n";

    // Something went wrong
    } else {
        std::cerr << "ERROR: Something went wrong";
        exit(1);
    }
}

#endif // JAGGED_ARRAY_H_INCLUDED
