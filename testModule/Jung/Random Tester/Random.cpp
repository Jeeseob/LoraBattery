#include <iostream>
#include <random>
using namespace std;

int main(void)
{
    int result = 0;
    random_device rd;
    mt19937 mersenne(rd());
    uniform_int_distribution<> Sensor(-40, 40);


    for (int i= 0; i < 1000; i++)
    {
        cout << Sensor(mersenne) << endl;
    }
    return 0;
}