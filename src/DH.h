#ifndef DH_h
#define DH_h

#define PRIME0 	0xFB
#define PRIME1 	0xFF 
#define PRIME2 	0xFF 
#define PRIME3 	0xFF 

#define PRIME4 	0xFB
#define PRIME5 	0xFF 
#define PRIME6 	0xFF 
#define PRIME7 	0xFF 

#define PRIME8 	0xFB
#define PRIME9 	0xFF 
#define PRIME10 0xFF 
#define PRIME11 0xFF 

#define PRIME12 0xFB 
#define PRIME13 0xFF 
#define PRIME14 0xFF 
#define PRIME15 0xFF 

#define GENERATOR0 0x02
#define GENERATOR1 0x00
#define GENERATOR2 0x00
#define GENERATOR3 0x00

#define KEY_SIZE 16
#define PRESHAREDKEY_SIZE 24
#define UNUSED_PIN 0 // For random seed

#include <Arduino.h>

#ifndef SERIAL_DEBUG
  #define SERIAL_DEBUG 1
#endif

class DH {
	public:
	  DH();
	  uint32_t mul_mod_p(uint32_t a, uint32_t b, uint32_t P);
	  uint32_t pow_mod_p(uint32_t a, uint32_t b, uint32_t P);
	  uint32_t pow_mod_p2(uint32_t a, uint32_t b, uint32_t P);
	  uint32_t randomInt32();
    void getSessionKey(uint8_t* neighborPublic);
	  void sendDHA(uint8_t* publicKey);
	  uint8_t session_private_key[KEY_SIZE];

    #if SERIAL_DEBUG
	    void printSessionKey();
    #endif
};

#endif