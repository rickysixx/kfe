#include <iostream>

using namespace std;

extern "C"
{
    double forexpr(double);
    double forexpr2(double);
}

int main(int argc, char** argv)
{
    cout << "forexpr(5) = " << forexpr(5.0) << endl;
    cout << "forexpr(7) = " << forexpr(7.0) << endl;

    return 0;
}