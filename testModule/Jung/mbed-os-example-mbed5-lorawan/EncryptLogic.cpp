#include <cstdint>
#include <iostream>
#include <cstring>

#include "BUILD/NUCLEO_F446RE/ARMC6/mbed_config.h"
#include "fhss_api.h"
#include "mbedtls/sha256.h"
#include "mbedtls/aes.h"

using namespace std;

class Security{
    public:
    mbedtls_sha256_context IVs;
    mbedtls_sha256_context Akeys;
    mbedtls_aes_context Ac;
    Security()
    {
        // IV와 AesKey에 대한 hash값들을 위한 초기화.
        // AES(CTR) 모드 init.
        mbedtls_sha256_init(&IVs);
        mbedtls_sha256_init(&Akeys);
        mbedtls_sha256_starts(&Akeys, 0);
        mbedtls_sha256_starts(&IVs, 0);
        mbedtls_aes_init(&Ac);
    }

    uint32_t Read()
    {
        // SHA256에 넣을 값들을 buffer에 삽입.
        for(int i =0; i < 32 ; i++)
        {
            this->buffer[i] = (unsigned char)appskey[i];
            this->buffer[i+16] = (unsigned char)nwkskey[i];
        }
       
        // SHA256에 buffer의 값을 삽입.
        mbedtls_sha256_update(&IVs, this->buffer, 32);

        // Buffer로 통해 만들어진 해쉬값을 IV에 복사
        mbedtls_sha256_finish(&IVs, IV);
        mbedtls_sha256_free(&IVs);

        //AES를 위한 IV생성완료, Key를 생산하기 위해 동일 동작 반복
        for(int i = 0; i < 7; i++)
        {
            this->buffer[i] = (unsigned char)deveui[i];
        }

        mbedtls_sha256_update(&Akeys,this->buffer, 7);

        mbedtls_sha256_finish(&Akeys, this->Akey);
        mbedtls_sha256_free(&Akeys);


        return Ci;
    }

    private:
    unsigned char buffer[32];
    unsigned char Akey[32];
    uint8_t nwkskey[16] = MBED_CONF_LORA_NWKSKEY;
    uint8_t appskey[16] = MBED_CONF_LORA_APPSKEY;
    uint8_t deveui[7] = MBED_CONF_LORA_DEVICE_EUI;
    unsigned char IV[16];
    uint32_t Ci;
};
