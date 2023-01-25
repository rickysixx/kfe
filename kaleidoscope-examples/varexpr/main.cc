#include <iostream>

using namespace std;

extern "C"
{
    double fib(double, double);
}

int main(int argc, char** argv)
{
    double x, y;

    cout << "Inserisci il valore di x: ";
    cin >> x;

    cout << "Inserisci il valore di y: ";
    cin >> y;

    cout << "fib(" << x << ", " << y << ") = " << fib(x, y) << endl;

    return 0;
}