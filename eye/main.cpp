#include <iostream>
#include "ControllersHoarder.h"




int main()
{
    eye::ControllersHoarder ch(100);
    std::cout << "t tg: " << ch.ft5p().add(0, 158) << "\n";
    std::cout << ch.update_all() << "\n";

    for (auto & e: ch.ft5p().params()) {
        std::cout << e.value << "\n";
    }
}
