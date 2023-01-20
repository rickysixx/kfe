#include <iostream>

extern "C" {
    double simple(double, double);
}

int main() {
    int x,y;
    std::cout << "Inserisci due numeri interi, x e y\nx: ";
    std::cin >> x;
    std::cout << "y: ";
    std::cin >> y;
    std::cout << "Il valore di simple(x,y) è " << simple(x,y) << std::endl;
}
