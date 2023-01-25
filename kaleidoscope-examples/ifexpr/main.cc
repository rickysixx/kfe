#include <iostream>

extern "C"
{
    double f(double, double);
}

int main(int argc, char** argv)
{
    double x, y;

    std::cout << "Inserisci il valore di x: ";
    std::cin >> x;

    std::cout << "Inserisci il valore di y: ";
    std::cin >> y;

    std::cout << "f(" << x << ", " << y << ") = " << f(x, y) << std::endl;

    return 0;
}