#ifndef ThompsonSampling_h
#define ThompsonSampling_h

#include "Bandit.h"
#include <stdlib.h>

#define DEFAULT_SF 9
#define CAD_ENABLED 0
#define MAB_TS_ENABLED 0
#define TS_SCALE 1000

class ThompsonSampling: private Bandit
{

  public:
    ThompsonSampling();
    ~ThompsonSampling();

    uint8_t pull();
    void update(uint8_t sf, uint8_t reward);

  private:
    volatile uint32_t rewards[NUM_SF];
    volatile uint32_t pulls[NUM_SF];
    volatile uint16_t estimatedMeans[NUM_SF];
};

#endif