#include "car.hpp"
#include <iostream>

int main(){
    Car c1;
    for (size_t i = 0; i < 10; ++i){
        c1.step(0.1);
        std::cout << c1 << std::endl;
    }
    
}