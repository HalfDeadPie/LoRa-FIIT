#ifndef ThompsonSampling_h
#define ThompsonSampling_h

#include "Bandit.h"
#include <stdlib.h>
#include <Arduino.h>

#define TS_SCALE 1000

class ThompsonSampling: private Bandit
{

  public:
    ThompsonSampling();
    ~ThompsonSampling();

    uint8_t pull(uint8_t currentSQ);
    void update(uint8_t currentSQ, uint8_t reward);

    #if FREQ_ENABLE
      uint8_t freqPull(uint8_t currentFREQ);
      void freqUpdate(uint8_t currentFREQ, uint8_t reward);
    #endif

  private:
    volatile uint32_t rewards[NUM_SF];
    volatile uint32_t pulls[NUM_SF];
    volatile uint16_t estimatedMeans[NUM_SF];

    #if FREQ_ENABLE
      volatile uint32_t freqRewards[NUM_SF];
      volatile uint32_t freqPulls[NUM_SF];
      volatile uint16_t freqEstimatedMeans[NUM_SF];
    #endif
};

#endif