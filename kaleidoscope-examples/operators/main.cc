#include <iostream>

extern "C" {
    double add(double, double);
    double sub(double, double);
    double mul(double, double);
    double divide(double, double);
    double lt(double, double);
    double le(double, double);
    double gt(double, double);
    double ge(double, double);
    double eq(double, double);
    double ne(double, double);
    double neg(double);
    double testneg();
    double compound();
}

int main(int argc, char** argv)
{
    using namespace std;

    double x, y;

    cout << "Inserisci il valore di x: ";
    cin >> x;

    cout << "Inserisci il valore di y: ";
    cin >> y;

    cout << "add(" << x << ", " << y << ") = " << add(x, y) << endl;
    cout << "sub(" << x << ", " << y << ") = " << sub(x, y) << endl;
    cout << "mul(" << x << ", " << y << ") = " << mul(x, y) << endl;
    cout << "divide(" << x << ", " << y << ") = " << divide(x, y) << endl;
    cout << "lt(" << x << ", " << y << ") = " << lt(x, y) << endl;
    cout << "le(" << x << ", " << y << ") = " << le(x, y) << endl;
    cout << "gt(" << x << ", " << y << ") = " << gt(x, y) << endl;
    cout << "ge(" << x << ", " << y << ") = " << ge(x, y) << endl;
    cout << "eq(" << x << ", " << y << ") = " << eq(x, y) << endl;
    cout << "ne(" << x << ", " << y << ") = " << ne(x, y) << endl;
    cout << "neg(" << x << ") = " << neg(x) << endl;
    cout << "testneg() = " << testneg() << endl;
    cout << "compound() = " << compound() << endl;
}