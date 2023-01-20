#include <iostream>

using namespace std;

extern "C" {
    double whileexpr(double);
}

int main(int argc, char** argv)
{
    cout << "whileexpr(5) = " << whileexpr(5) << endl;

    return 0;
}