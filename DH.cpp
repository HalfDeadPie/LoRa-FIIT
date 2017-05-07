#include "DH.h"

//public variable representing the shared secret key.
uint32_t Alice_k, Bob_k;

//Constructor
DH::DH(){
}

uint32_t DH::mul_mod_p(uint32_t a, uint32_t b, uint32_t P) {
  uint32_t m = 0;

  while (b) {
    if (b & 1) {
      uint32_t t = P - a;
      if ( m >= t) {
        m -= t;
      } else {
        m += a;
      }
    }
    if (a >= P - a) {
      a = a * 2 - P;
    } else {
      a = a * 2;
    }
    b >>= 1;
  }
  return m;
}

uint32_t DH::pow_mod_p(uint32_t a, uint32_t b, uint32_t P) {
  /*if(a>P){
	  a %= P;
  }*/
  if (b == 1) {
    return a;
  }
  uint32_t t = pow_mod_p(a, b >> 1, P);
  t = mul_mod_p(t, t, P);
  if (b % 2) {
    t = mul_mod_p(t, a, P);
  }
  return t;
}

uint32_t DH::randomint32() {
  return random() << 16 | random();
}

void DH::sendDHA(uint8_t* publickey){
	randomSeed(analogRead(UNUSED_PIN));
	uint8_t generator[4];
	generator[0] = GENERATOR0;
	generator[1] = GENERATOR1;
	generator[2] = GENERATOR2;
	generator[3] = GENERATOR3;
	uint32_t* generatorpointer = (uint32_t*) &generator[0];
	
	//Round1
	uint8_t presharedkey[4];
	presharedkey[0] = PRIME0;
	presharedkey[1] = PRIME1;
	presharedkey[2] = PRIME2;
	presharedkey[3] = PRIME3;
	uint32_t* privatekey = (uint32_t*) &session_private_key[0];
	*privatekey = randomint32();	
	uint32_t* publicpointer = (uint32_t*) publickey;
	
	uint32_t* presharedpointer = (uint32_t*) &presharedkey[0];

	
	*publicpointer = pow_mod_p(*generatorpointer, *privatekey, *presharedpointer);
	
	//Round2
	presharedkey[0] = PRIME4;
	presharedkey[1] = PRIME5;
	presharedkey[2] = PRIME6;
	presharedkey[3] = PRIME7;
	privatekey++;
	*privatekey = randomint32();
	publicpointer++;
	*publicpointer = pow_mod_p(*generatorpointer, *privatekey, *presharedpointer);
	
	//Round3
	presharedkey[0] = PRIME8;
	presharedkey[1] = PRIME9;
	presharedkey[2] = PRIME10;
	presharedkey[3] = PRIME11;
	privatekey++;
	*privatekey = randomint32();
	publicpointer++;
	*publicpointer = pow_mod_p(*generatorpointer, *privatekey, *presharedpointer);
	
	//Round4
	presharedkey[0] = PRIME12;
	presharedkey[1] = PRIME13;
	presharedkey[2] = PRIME14;
	presharedkey[3] = PRIME15;
	privatekey++;
	*privatekey = randomint32();
	publicpointer++;
	*publicpointer = pow_mod_p(*generatorpointer, *privatekey, *presharedpointer);
}

void DH::getSessionKey(uint8_t* neighborpublic){
	
	//Round1
	uint32_t* sessionkey = (uint32_t*) &session_private_key[0];
	uint32_t privatekey = *sessionkey;
	uint32_t* publicpointer = (uint32_t*) neighborpublic;
	uint8_t presharedkey[8];
	presharedkey[0] = PRIME0;
	presharedkey[1] = PRIME1;
	presharedkey[2] = PRIME2;
	presharedkey[3] = PRIME3;
	
	uint32_t* presharedpointer = (uint32_t*) &presharedkey[0];
	*sessionkey = pow_mod_p(*publicpointer, privatekey, *presharedpointer);
	
	//Round2
	sessionkey++;
	privatekey = *sessionkey;
	publicpointer++;
	presharedkey[0] = PRIME4;
	presharedkey[1] = PRIME5;
	presharedkey[2] = PRIME6;
	presharedkey[3] = PRIME7;
	*sessionkey = pow_mod_p(*publicpointer, privatekey, *presharedpointer);
	
	//Round3
	sessionkey++;
	privatekey = *sessionkey;
	publicpointer++;
	presharedkey[0] = PRIME8;
	presharedkey[1] = PRIME9;
	presharedkey[2] = PRIME10;
	presharedkey[3] = PRIME11;
	*sessionkey = pow_mod_p(*publicpointer, privatekey, *presharedpointer);
	
	//Round4
	sessionkey++;
	privatekey = *sessionkey;
	publicpointer++;
	presharedkey[0] = PRIME12;
	presharedkey[1] = PRIME13;
	presharedkey[2] = PRIME14;
	presharedkey[3] = PRIME15;	
	*sessionkey = pow_mod_p(*publicpointer, privatekey, *presharedpointer);
	/*
	Serial.print("**Session key: ");
	for(int i = 0; i<16; i++){
		Serial.print(session_private_key[i],HEX);
		Serial.print(" ");
	}
	Serial.println();*/
}