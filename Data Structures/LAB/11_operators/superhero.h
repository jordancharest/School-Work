#ifndef SUPERHERO_H_INCLUDED
#define SUPERHERO_H_INCLUDED

#include <string>
#include <vector>
#include <utility>
#include <iostream>

class Superhero {
public:
    Superhero(std::string name, std::string true_identity, std::string power) : name_(name), true_identity_(true_identity), power_(power), good(true) {}
    friend class Team;

    // ACCCESORS
    std::string getPower() const { return power_; }
    std::string getName() const { return name_; }
    bool isGood() const { return good; }

    // COMPARISON OPERATORS
    bool operator== (std::string guess) const { return this->true_identity_ == guess; }
    bool operator== (Superhero const& hero2) const { return this->name_ == hero2.name_; }
    bool operator!= (std::string guess) const { return this->true_identity_ != guess; }
    void operator- () { flip(); }

    bool operator>(const Superhero &hero2) {
      std::vector<std::pair<std::string, std::string> >::iterator it;
      for (it = superpower_rank.begin(); it != superpower_rank.end(); it++)
        if (it->first == this->power_ && it->second == hero2.power_)
          return true;
      return false;
    }




    // Rank the superpowers
    static void populateSuperpowerRank() {
        superpower_rank.push_back(std::pair<std::string, std::string>("Fire", "Wood"));
        superpower_rank.push_back(std::pair<std::string, std::string>("Wood", "Water"));
        superpower_rank.push_back(std::pair<std::string, std::string>("Water", "Fire"));
    }



private:
    // PRIVATE ACCESSOR
    std::string getTrueIdentity() const { return true_identity_; }
    void flip() { good = !good;}

    // MEMBER REPRESENTATION
    std::string name_;
    std::string true_identity_;
    std::string power_;
    bool good;

    // ranking of superpowers
    static std::vector<std::pair<std::string, std::string> > superpower_rank;
};

// OUTPUT
inline std::ostream& operator<< (std::ostream &ostr, Superhero const& hero) {
    ostr << "Super" << (hero.isGood()? "hero " : "villain ") << hero.getName() << " has power " << hero.getPower() << "\n";
    return ostr;
}





#endif // SUPERHERO_H_INCLUDED
