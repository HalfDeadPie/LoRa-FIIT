/***
 * From Geeks for Geeks
 * Step 1: Alice and Bob get public numbers P = 23, G = 9
 * Step 2: Alice selected a private key a = 4 and Bob selected a private key b = 3
 * Step 3: Alice and Bob compute public values
 *         Alice: x =(9^4 mod 23) = (6561 mod 23) = 6
 *         Bob: y = (9^3 mod 23) = (729 mod 23)  = 16
 * Step 4: Alice and Bob exchange public numbers
 * Step 5: Alice receives public key y =16 and Bob receives public key x = 6
 * Step 6: Alice and Bob compute symmetric keys
 *         Alice: ka = y^a mod p = 65536 mod 23 = 9
 *         Bob: kb = x^b mod p = 216 mod 23 = 9
 * Step 7: 9 is the shared secret.
 */

#include "DH.h"

/** Public variable representing the shared secret key. */
uint32_t Alice_k, Bob_k;

/**
 * Constructor
 */
DH::DH() {}

/**
 *
 * @param a
 * @param b
 * @param P
 * @return
 */
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

/**
 *
 * @param a
 * @param b
 * @param P
 * @return
 */
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

uint32_t DH::pow_mod_p2(uint32_t a, uint32_t b, uint32_t P) {
    uint32_t t = a;
    uint32_t arr[100];
    int index  = 0;

    while(b > 0){
        arr[index] = b;
        index++;
        b /= 2;
    }

    for(int i = index - 2; i >= 0; i--){
       t = mul_mod_p(t, t, P);

       if(arr[i] % 2){
           t = mul_mod_p(t,a,P);
       }
    }

    return t;
}


/**
 * Generates random 32 unsigned integer
 * @return
 */
uint32_t DH::randomInt32() {
  return random() << 16 | random();
}

/**
 * Generating a DH message and key
 * @param publicKey
 */
void DH::sendDHA(uint8_t* publicKey) {
	randomSeed(analogRead(UNUSED_PIN));
	uint8_t generator[4];
	generator[0] = GENERATOR0;
	generator[1] = GENERATOR1;
	generator[2] = GENERATOR2;
	generator[3] = GENERATOR3;
	uint32_t* generatorpointer = (uint32_t*) &generator[0];
	
	// Round1
	uint8_t presharedkey[4];
	presharedkey[0] = PRIME0;
	presharedkey[1] = PRIME1;
	presharedkey[2] = PRIME2;
	presharedkey[3] = PRIME3;
	uint32_t* privatekey = (uint32_t*) &session_private_key[0];
	*privatekey = randomInt32();
	uint32_t* publicpointer = (uint32_t*) publicKey;
	uint32_t* presharedpointer = (uint32_t*) &presharedkey[0];
	*publicpointer = pow_mod_p2(*generatorpointer, *privatekey, *presharedpointer);
	
	// Round2
	presharedkey[0] = PRIME4;
	presharedkey[1] = PRIME5;
	presharedkey[2] = PRIME6;
	presharedkey[3] = PRIME7;
	privatekey++;
	*privatekey = randomInt32();
	publicpointer++;
	*publicpointer = pow_mod_p2(*generatorpointer, *privatekey, *presharedpointer);
	
	// Round3
	presharedkey[0] = PRIME8;
	presharedkey[1] = PRIME9;
	presharedkey[2] = PRIME10;
	presharedkey[3] = PRIME11;
	privatekey++;
	*privatekey = randomInt32();
	publicpointer++;
	*publicpointer = pow_mod_p2(*generatorpointer, *privatekey, *presharedpointer);
	
	// Round4
	presharedkey[0] = PRIME12;
	presharedkey[1] = PRIME13;
	presharedkey[2] = PRIME14;
	presharedkey[3] = PRIME15;
	privatekey++;
	*privatekey = randomInt32();
	publicpointer++;
	*publicpointer = pow_mod_p2(*generatorpointer, *privatekey, *presharedpointer);
}

/**
 * Generates a session key
 * @param neighborpublic
 */
void DH::getSessionKey(uint8_t* neighborPublic) {
	// Round1
	uint32_t* sessionkey = (uint32_t*) &session_private_key[0];
	uint32_t privatekey = *sessionkey;
	uint32_t* publicpointer = (uint32_t*) neighborPublic;
	uint8_t presharedkey[8];
	presharedkey[0] = PRIME0;
	presharedkey[1] = PRIME1;
	presharedkey[2] = PRIME2;
	presharedkey[3] = PRIME3;
	
	uint32_t* presharedpointer = (uint32_t*) &presharedkey[0];
	*sessionkey = pow_mod_p2(*publicpointer, privatekey, *presharedpointer);
	
	// Round2
	sessionkey++;
	privatekey = *sessionkey;
	publicpointer++;
	presharedkey[0] = PRIME4;
	presharedkey[1] = PRIME5;
	presharedkey[2] = PRIME6;
	presharedkey[3] = PRIME7;
	*sessionkey = pow_mod_p2(*publicpointer, privatekey, *presharedpointer);
	
	// Round3
	sessionkey++;
	privatekey = *sessionkey;
	publicpointer++;
	presharedkey[0] = PRIME8;
	presharedkey[1] = PRIME9;
	presharedkey[2] = PRIME10;
	presharedkey[3] = PRIME11;
	*sessionkey = pow_mod_p2(*publicpointer, privatekey, *presharedpointer);
	
	// Round4
	sessionkey++;
	privatekey = *sessionkey;
	publicpointer++;
	presharedkey[0] = PRIME12;
	presharedkey[1] = PRIME13;
	presharedkey[2] = PRIME14;
	presharedkey[3] = PRIME15;	
	*sessionkey = pow_mod_p2(*publicpointer, privatekey, *presharedpointer);
}

#if SERIAL_DEBUG
/**
 * Prints a session key in HEX format
 */
void DH::printSessionKey() {
  Serial.print(F("Session key: "));

  for (int i = 0; i < 16; i++) {
    Serial.print(session_private_key[i], HEX);
    Serial.print(" ");
  }

  Serial.println();
}
#endif