//
// Created by root on 5.4.2017.
//
#include "Encryption.h"

void Encryption::encrypt(uint8_t *indata, uint8_t size, uint8_t *keyByte) {
    uint32_t *v = (uint32_t *) indata;
    int n = size/ sizeof(uint32_t);
    uint32_t *key = (uint32_t *) keyByte;

    uint32_t y, z, sum;
    unsigned p, rounds, e;
        rounds = 6 + 52/n;
        sum = 0;
        z = v[n-1];
        do {
            sum += DELTA;
            e = (sum >> 2) & 3;
            for (p=0; p<n-1; p++) {
                y = v[p+1];
                z = v[p] += MX;
            }
            y = v[0];
            z = v[n-1] += MX;
        } while (--rounds);
}

void Encryption::decrypt(uint8_t *indata, uint8_t size, uint8_t *keyByte) {
    uint32_t *v = (uint32_t *) indata;
    int n = size/ sizeof(uint32_t);
    uint32_t *key = (uint32_t *) keyByte;

    uint32_t y, z, sum;
    unsigned p, rounds, e;
    rounds = 6 + 52/n;
    sum = rounds*DELTA;
    y = v[0];
    do {
        e = (sum >> 2) & 3;
        for (p=n-1; p>0; p--) {
            z = v[p-1];
            y = v[p] -= MX;
        }
        z = v[n-1];
        y = v[0] -= MX;
        sum -= DELTA;
    } while (--rounds);
}


uint32_t Encryption::createCheck(uint8_t *data, uint8_t size) {
    //One-at-a-Time hash
    uint32_t hash = 0;
    for (int i = 0; i < size; i++)
    {
        hash += data[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
	
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}

bool Encryption::isLoraPacketCorrect(uint8_t *in, uint8_t size, uint32_t compare) {
    uint32_t result = createCheck(in,size);
	if (result == compare){
        return true;
    }
    return false;
}
