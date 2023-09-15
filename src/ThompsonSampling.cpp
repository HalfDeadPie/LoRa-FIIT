#include "ThompsonSampling.h"

/**
 * @brief Construct a new Thompson Sampling:: Thompson Sampling object
 * 
 */
ThompsonSampling::ThompsonSampling() 
{
  for (uint8_t idx = 0; idx < NUM_SF; idx++) {
    rewards[idx] = 0;
    pulls[idx] = 0;
    estimatedMeans[idx] = TS_SCALE / 2;  // Initial scaled estimated mean for each SF
  }
}

ThompsonSampling::~ThompsonSampling() 
{
}

/**
 * @brief Picks the best SF according to Thompson Sampling algorithm
 * 
 * @return uint8_t 
 */
uint8_t ThompsonSampling::pull() {
  uint8_t bestSF = MIN_SF;
  uint16_t maxMean = estimatedMeans[0];

  for (uint8_t sf = MIN_SF; sf <= MAX_SF; sf++) {
    uint8_t idx = sf - MIN_SF;
    uint16_t sample = rand() % TS_SCALE; // Scaled random sample between 0 and TS_SCALE
    if (estimatedMeans[idx] > maxMean && sample > maxMean) {
      maxMean = estimatedMeans[idx];
      bestSF = sf;
    }
  }

  return bestSF;
}

/**
 * @brief Updates the estimated means for each SF
 * 
 * @param sf 
 * @param reward 
 */
void ThompsonSampling::update(uint8_t sf, uint8_t reward)
{
  uint8_t idx = sf - MIN_SF;
  
  // Check againts index overflows
  if (idx < MIN_SF && idx > MAX_SF) {
    return;
  } 
  
  pulls[idx]++;
  rewards[idx] += reward * TS_SCALE;
  
  if (pulls[idx] > 0) {
    estimatedMeans[idx] = rewards[idx] / pulls[idx];
  }
}

