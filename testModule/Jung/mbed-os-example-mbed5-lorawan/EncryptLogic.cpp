#include <cstdint>
#include <iostream>
#include <cstring>

#include "mbedtls/sha256.h"

using namespace std;

class Security{
    public:
    mbedtls_sha256_context Sc;
    Security()
    {
        mbedtls_sha256_init(&Sc);

    }

    void setIV(uint8_t Rand[])
    {
        memcpy(this->IV, Rand, 16);
    }

    private:
    uint8_t nwkskey[16] = MBED_CONF_LORA_NWKSKEY;
    uint8_t appskey[16] = MBED_CONF_LORA_APPSKEY;
    uint8_t IV[16];

};