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
        mbedtls_sha256_starts(&Sc, 0);
    }

    void SetIV(uint8_t Rand[])
    {
        memcpy(this->IV, Rand, 16);
    }

    void Setbuffer(uint8_t* appskey, uint8_t* nwkskey)
    {
        for(int i =0; i < 32 ; i++)
        {
            this->buffer[i] = (unsigned char)appskey[i];
            this->buffer[i+16] = (unsigned char)nwkskey[i];
        }
    }

    void UpdateBuf()
    {
        mbedtls_sha256_update(&Sc, this->buffer, 32);
    }

    void CloseBuf()
    {
        
    }

    private:
    unsigned char buffer[32];
    uint8_t nwkskey[16] = MBED_CONF_LORA_NWKSKEY;
    uint8_t appskey[16] = MBED_CONF_LORA_APPSKEY;
    uint8_t IV[16];
};