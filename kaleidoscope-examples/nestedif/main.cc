#include <iostream>

using namespace std;

extern "C" {
    double nested(double, double);
}

int main(int argc, char** argv)
{
    double x, y;

    cout << "Inserisci il valore di x: ";
    cin >> x;

    cout << "Inserisci il valore di y: ";
    cin >> y;

    cout << "Il valore di simple(" << x << ", " << y << ") è: " << nested(x, y) << "." << endl;

    return 0;
}