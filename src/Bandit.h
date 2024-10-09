#ifndef Bandit_h
#define Bandit_h

#include <stdint.h> 

#define MIN_SF 7
#define MAX_SF 12
#define NUM_SF 6
#define NUM_FREQ 5
#define FREQ_ENABLE 1

class Bandit
{

  public:
    Bandit();
    virtual ~Bandit();

    virtual uint8_t pull(uint8_t bestSF);

  private:
    uint32_t rewards[NUM_SF];
    uint32_t pulls[NUM_SF];

    #if FREQ_ENABLE
        uint32_t freqRewards[NUM_FREQ];
        uint32_t freqPulls[NUM_FREQ];

      protected:
          uint8_t freqToIdx(uint8_t freq);
          uint8_t idxToFreq(uint8_t idx);
    #endif
};

#endif