#include <iostream>

using namespace std;

extern "C" {
    double arr();
}

int main(int argc, char** argv)
{
    cout << "arr() = " << arr() << endl;

    return 0;
}