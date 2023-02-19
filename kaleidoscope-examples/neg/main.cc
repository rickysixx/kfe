#include <iostream>

extern "C"
{
    double neg();
    double neg2(double);
    double neg3(double);
    double neg4(double, double);
}

int main()
{
    double x, y;

    std::cout << "Inserisci il valore di x: ";
    std::cin >> x;

    std::cout << "Inserisci il valore di y: ";
    std::cin >> y;

    std::cout << "neg() = " << neg() << std::endl;
    std::cout << "neg2(" << x << ") = " << neg2(x) << std::endl;
    std::cout << "neg3(" << x << ") = " << neg3(x) << std::endl;
    std::cout << "neg4(" << x << ", " << y << ") = " << neg4(x, y) << std::endl;

    return 0;
}