#ifndef Encryption_h
#define Encryption_h

//#include <algorithm>
#include <Arduino.h>

#define DELTA 0x9e3779b9
#define MX (((z>>5^y<<2) + (y>>3^z<<4)) ^ ((sum^y) + (key[(p&3)^e] ^ z)))

class Encryption{
	
	public:
	Encryption();
	
	static void encrypt(uint8_t *indata, uint8_t size, uint8_t *keyByte);
	
	static void decrypt(uint8_t *indata, uint8_t size, uint8_t *keyByte);
	
	static uint32_t createCheck(uint8_t *data, uint8_t size);
	
	static bool isLoraPacketCorrect(uint8_t *in, uint8_t size,uint32_t compare);
};

#endif
