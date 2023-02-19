#include <iostream>

using namespace std;

extern "C"
{
    double simpleif(double);
    double fib(double);
}

int main(int argc, char** argv)
{
    double x, y;

    cout << "Inserisci il valore di x: ";
    cin >> x;

    cout << "simpleif(" << x << ") = " << simpleif(x) << endl;
    cout << "fib(" << x << ") = " << fib(x) << endl;

    return 0;
}