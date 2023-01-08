#include "Encryption.h"

/**
 * Encrypts data payload using XXTEA algorithm
 * @param payload
 * @param size
 * @param keyByte
 */
void Encryption::encrypt(uint8_t *payload, uint8_t size, uint8_t *keyByte) {
  uint32_t *v = (uint32_t *) payload;
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

/**
 * Decrypts data payload using XXTEA algorithm
 * @param payload
 * @param size
 * @param keyByte
 */
void Encryption::decrypt(uint8_t *payload, uint8_t size, uint8_t *keyByte) {
  uint32_t *v = (uint32_t *) payload;
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

/**
 * Generates a Message Integrity Code (MIC)
 * @param payload
 * @param size
 * @return
 */
uint32_t Encryption::generateMIC(uint8_t *payload, uint8_t size) {
  // One-at-a-Time hash
  uint32_t hash = 0;

  for (int i = 0; i < size; i++)
  {
    hash += payload[i];
    hash += (hash << 10);
    hash ^= (hash >> 6);
  }

  hash += (hash << 3);
  hash ^= (hash >> 11);
  hash += (hash << 15);
  return hash;
}

/**
 * Checking LoRa Message Integrity Code
 * @param payload
 * @param size
 * @param originalMIC
 * @return
 */
bool Encryption::checkMIC(uint8_t *payload, uint8_t size, uint32_t originalMIC) {
  return generateMIC(payload, size) == originalMIC;
}
