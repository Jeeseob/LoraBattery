#include <iostream>
#include <string.h>
using namespace std;

int main(void)
{
    char txbuffer[] =  "mbed-os-example-mbed5-lorawan";
    cout << "char(ASCII): ";
    for(int i = 0; i< strlen(txbuffer); i++)
    {
        printf("%d\t", txbuffer[i]);
    }
    return 0;
}