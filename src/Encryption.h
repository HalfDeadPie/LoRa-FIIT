#ifndef Encryption_h
#define Encryption_h

#include <Arduino.h>

#define DELTA 0x9e3779b9
#define MX (((z>>5^y<<2) + (y>>3^z<<4)) ^ ((sum^y) + (key[(p&3)^e] ^ z)))

class Encryption {
	public:
	  Encryption();
	  static void encrypt(uint8_t *payload, uint8_t size, uint8_t *keyByte);
	  static void decrypt(uint8_t *payload, uint8_t size, uint8_t *keyByte);
	  static uint32_t generateMIC(uint8_t *payload, uint8_t size);
	  static bool checkMIC(uint8_t *payload, uint8_t size, uint32_t originalMIC);
};

#endif
