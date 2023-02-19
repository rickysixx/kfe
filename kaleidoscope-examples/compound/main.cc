#include <iostream>

extern "C"
{
    double compound(double);
}

int main()
{
    double x;

    std::cout << "Inserisci il valore di x: ";
    std::cin >> x;

    std::cout << "compound(" << x << ") = " << compound(x) << std::endl;

    return 0;
}