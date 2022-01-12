#include <iostream>
#include <cstdlib>
#include <ctime>
using namespace std;

int main(void)
{
    srand((unsigned int)time(NULL));
    int rnum;
    int result = 0;
    for (int i= 0; i < 10; i++)
    {
        rnum = (rand() % 10)-5;
        result += rnum;
        cout << result << endl;
    }
    return 0;
}