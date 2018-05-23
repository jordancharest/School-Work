#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <list>
#include <vector>
#include <iomanip>
#include <algorithm>

typedef std::map<std::string, int> INVENTORY_TYPE;
typedef std::map<std::string, std::string> CUSTOMER_TYPE;
typedef std::map<std::string, std::list<std::string> > COSTUME_TYPE;

// prototypes for the helper functions
void addCostume(INVENTORY_TYPE &inventory, COSTUME_TYPE &rental_history, std::string costume, int quantity);
void rentCostume(INVENTORY_TYPE &inventory, CUSTOMER_TYPE &customers, std::string first_name, std::string last_name,
                  std::string costume, COSTUME_TYPE &rental_history);

void lookup(INVENTORY_TYPE const& inventory, CUSTOMER_TYPE const& customers, COSTUME_TYPE &rental_history, std::string costume);
void printCustomers(CUSTOMER_TYPE const& customers);
std::string swapName(std::string last_name_first);

void printInventory(INVENTORY_TYPE const& inventory);






// MAIN ==============================================================================================================================
int main(int argc, char* argv[]) {

    if (argc != 1){
        std::cerr << "USAGE: " << argv[0] << " < command-file > output-file\n";
        exit(0);
    }


    // two data structures store all of the information for efficiency
    INVENTORY_TYPE inventory;       // stores each costume with the quantity available
    CUSTOMER_TYPE customers;        // stores all every customer that has ever visited and their current costume
    COSTUME_TYPE rental_history;   // stores each costume and a chronological list of renters of that costume

    std::string costume;
    std::string first_name;
    std::string last_name;
    std::string name;
    int quantity;

    char c;
    //int i = 0;
    while (std::cin >> c) {
        if (c == 'a') {
            std::cin >> costume >> quantity;
            addCostume(inventory, rental_history, costume, quantity);

        } else if (c == 'r') {
            std::cin >> first_name >> last_name >> costume;
            rentCostume(inventory, customers, first_name, last_name, costume, rental_history);

        } else if (c == 'l') {
            std::cin >> costume;
            lookup(inventory, customers, rental_history, costume);

        } else if (c == 'p') {
            printCustomers(customers);

        } else {
            std::cerr << "ERROR: Unknown command " << c << std::endl;
            exit(0);
        }

    /*
        std::cout << "\n\nCYCLE " << i;
        printInventory(inventory);
        i++;
    */
    }


}


// ADD COSTUME ==========================================================================================================================
void addCostume(INVENTORY_TYPE& inventory, COSTUME_TYPE &rental_history, std::string costume, int quantity) {
    inventory[costume] += quantity;
    rental_history[costume];

    std::cout << "Added " << quantity << " " << costume << " costume";
    if (quantity > 1)   std::cout << "s";
    std::cout << ".\n";
}


// RENT COSTUME =========================================================================================================================
void rentCostume(INVENTORY_TYPE &inventory, CUSTOMER_TYPE &customers, std::string first_name, std::string last_name,
                  std::string costume, COSTUME_TYPE &rental_history) {


    std::string name = last_name + " " + first_name;    // map will store the customers sorted by last name

    INVENTORY_TYPE::iterator location = inventory.find(costume);

    // ensure the shop carries the costume
    if (location == inventory.end()){
        std::cout <<"The shop doesn't carry " << costume << " costumes.\n";
        customers[name]; // generates a key-value pair if it doesn't exist, otherwise does nothing
        return;

    // costume exists but none available
    } else if (location->second == 0) {
        std::cout << "No " << costume << " costumes available.\n";
        customers[name];    // generates a key-value pair if it doesn't exist, otherwise does nothing
        return;
    }

    // Customer already has the costume they want to rent
    if (customers[name] == costume) {
        std::cout << swapName(name) << " already has a " << costume << " costume.\n";
        return;
    }


    // Customer has already rented a different costume, return it to inventory
    if (customers[name] != ""){
        inventory[customers[name]]++;
        std::cout << swapName(name) << " returns a " << customers[name] << " costume before renting a " << costume << " costume.\n";
        rental_history[customers[name]].remove(name);

    } else {
        std::cout << swapName(name) << " rents a " << costume << " costume.\n";
    }

    customers[name] = costume;
    location->second--;
    rental_history[costume].push_back(name);
}



// LOOKUP ===============================================================================================================================
void lookup(INVENTORY_TYPE const& inventory, CUSTOMER_TYPE const& customers, COSTUME_TYPE &rental_history, std::string costume) {

    std::map<std::string, int>::const_iterator itr_map = inventory.find(costume);
    std::list<std::string>::const_iterator itr_list;

    // ensure the costume is in inventory
    if (itr_map == inventory.end()){
        std::cout <<"The shop doesn't carry " << costume << " costumes.\n";
        return;
    }

    std::cout << "Shop info for " << costume << " costumes:\n";

    if (itr_map-> second > 1)       std::cout << "  " << itr_map-> second << " copies available\n";
    else if (itr_map-> second == 1) std::cout << "  " << itr_map-> second << " copy available\n";

    if (rental_history[costume].begin() != rental_history[costume].end()){
        std::cout << "  " << rental_history[costume].size();
        if (rental_history[costume].size() > 1)    std::cout << " copies";
        else                        std::cout << " copy";
        std::cout << " rented by:\n";

        for (itr_list = rental_history[costume].begin(); itr_list != rental_history[costume].end(); itr_list++) {
            std::cout << "    " << swapName(*itr_list) << "\n";
        }
    }
}


// PRINT PEOPLE =========================================================================================================================
void printCustomers(CUSTOMER_TYPE const& customers) {
    CUSTOMER_TYPE::const_iterator itr_map;
    std::string name;

    std::cout << "Costume info for the " << customers.size() << " party attendee";
    if (customers.size() > 1)   std::cout << "s";
    std::cout << ":\n";

    for (itr_map = customers.begin(); itr_map != customers.end(); itr_map++){
        name = swapName(itr_map->first);
        std::cout << "  " << name;

        if (itr_map->second != "")
            std::cout << " is wearing a " << itr_map->second << " costume.\n";
        else
            std::cout << " does not have a costume.\n";
    }
}


// PRINT INVENTORY ====================================================================================================================
void printInventory(INVENTORY_TYPE const& inventory){
    std::map<std::string, int>::const_iterator itr_map;

    std::cout << "\nCostume        |  Quantity\n";
    for (itr_map = inventory.begin(); itr_map != inventory.end(); itr_map++) {
        std::cout << std::setw(15) << std::left << itr_map-> first
                  << std::setw(8) << std::right << itr_map-> second << "\n";
    }
}

// PRINT RENTALS ======================================================================================================================
void printRentals (CUSTOMER_TYPE& customers) {
    std::map<std::string, std::string>::const_iterator itr_map;

    std::cout << "\nRenter       |  Costume\n";
    for (itr_map = customers.begin(); itr_map != customers.end(); itr_map++) {
        std::cout << std::setw(15) << std::left << itr_map-> first
                  << std::setw(8) << std::right << itr_map-> second << "\n";
    }
}



/* SWAP NAME =========================================================================================================================
    Takes in a name that is formatted with the last name first and returns the name with the first name first
*/
std::string swapName(std::string last_name_first){

    int index = last_name_first.find(" ");
    return last_name_first.substr(index+1) + " " + last_name_first.substr(0, index);
}


