#include <iostream>

extern "C"
{
    double fib(double);
}

int main(int argc, char** argv)
{
    double n;

    std::cout << "Inserisci il valore di n: ";
    std::cin >> n;

    std::cout << "fib(" << n << ") = " << fib(n) << std::endl;

    return 0;
}