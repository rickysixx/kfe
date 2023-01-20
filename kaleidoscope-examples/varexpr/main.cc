#include <iostream>

using namespace std;

extern "C" {
    double fib(double);
}

int main(int argc, char** argv)
{
    cout << "fib(10) = " << fib(10) << endl;

    return 0;
}