#ifndef Bandit_h
#define Bandit_h

#include <stdint.h> 

#define MIN_SF 0x07
#define MAX_SF 0x12
#define NUM_SF 0x06

class Bandit
{

  public:
    Bandit();
    virtual ~Bandit();

    virtual uint8_t pull();
    virtual void update(uint8_t idx, uint8_t reward);

  private:
    uint32_t rewards[NUM_SF];
    uint32_t pulls[NUM_SF];

};

#endif