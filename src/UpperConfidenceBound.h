#ifndef UpperConfidenceBound_h
#define UpperConfidenceBound_h

#include "Bandit.h"
#include <math.h>
#include <Arduino.h>

#define UCB_SCALE 10000

class UpperConfidenceBound: private Bandit
{
  public:
    UpperConfidenceBound();
    ~UpperConfidenceBound();

    uint8_t pull(uint8_t currentSF);
    void update(uint8_t sf, int32_t reward);

    #if FREQ_ENABLE
      uint8_t freqPull(uint8_t currentFreq);
      void freqUpdate(uint8_t freq, int32_t reward);
    #endif

  private:
    volatile uint32_t pulls[NUM_SF];
    volatile int32_t rewards[NUM_SF];
    volatile uint32_t totalPulls;

    uint32_t integerSqrt(uint32_t x);
    int32_t ucbScore(uint8_t sf);

    #if FREQ_ENABLE
      volatile uint32_t freqPulls[NUM_FREQ];
      volatile int32_t freqRewards[NUM_FREQ];
      volatile uint32_t freqTotalPulls;

      int32_t freqUcbScore(uint8_t freq);
    #endif
};

#endif