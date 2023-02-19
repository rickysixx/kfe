#include <iostream>

extern "C"
{
    double lt(double, double);
    double le(double, double);
    double gt(double, double);
    double ge(double, double);
    double eq(double, double);
    double ne(double, double);
}

int main(int argc, char** argv)
{
    double x, y;

    std::cout << "Inserisci il valore di x: ";
    std::cin >> x;

    std::cout << "Inserisci il valore di y: ";
    std::cin >> y;

    std::cout << "lt(" << x << ", " << y << ") = " << lt(x, y) << std::endl;
    std::cout << "le(" << x << ", " << y << ") = " << le(x, y) << std::endl;
    std::cout << "gt(" << x << ", " << y << ") = " << gt(x, y) << std::endl;
    std::cout << "ge(" << x << ", " << y << ") = " << ge(x, y) << std::endl;
    std::cout << "eq(" << x << ", " << y << ") = " << eq(x, y) << std::endl;
    std::cout << "ne(" << x << ", " << y << ") = " << ne(x, y) << std::endl;

    return 0;
}