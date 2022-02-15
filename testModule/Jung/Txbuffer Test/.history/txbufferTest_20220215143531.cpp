#include <iostream>
using namespace std;

int main(void)
{
    char txbuffer[] =  "mbed-os-example-mbed5-lorawan";
    cout << "char(ASCII): ";
    for(int i = 0; i< txbuffer.length(); i++)
    {
        printf("%d\t", txbuffer[i]);
    }
    return 0;
}