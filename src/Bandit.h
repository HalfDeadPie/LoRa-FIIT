#ifndef Bandit_h
#define Bandit_h

#include <stdint.h> 

#define MIN_SF 7
#define MAX_SF 12
#define NUM_SF 6

class Bandit
{

  public:
    Bandit();
    virtual ~Bandit();
    
    virtual uint8_t pull(uint8_t bestSF);

  private:
    uint32_t rewards[NUM_SF];
    uint32_t pulls[NUM_SF];

};

#endif