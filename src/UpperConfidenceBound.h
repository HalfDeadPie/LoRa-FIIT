#ifndef UpperConfidenceBound_h
#define UpperConfidenceBound_h

#include "Bandit.h"
#include <math.h>

#define UCB_SCALE 10000

class UpperConfidenceBound: private Bandit
{
  public:
    UpperConfidenceBound();
    ~UpperConfidenceBound();

    uint8_t pull();
    void update(uint8_t sf, int32_t reward);

  private:
    volatile uint32_t pulls[NUM_SF];
    volatile int32_t rewards[NUM_SF];
    volatile uint32_t totalPulls;

    uint32_t integerSqrt(uint32_t x);
    int32_t ucbScore(uint8_t sf);
};

#endif