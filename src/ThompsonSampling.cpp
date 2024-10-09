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

  #if FREQ_ENABLE
    for (uint8_t idx = 0; idx < NUM_FREQ; idx++) {
      freqRewards[idx] = 0;
      freqPulls[idx] = 0;
      freqEstimatedMeans[idx] = TS_SCALE / 2;  // Initial scaled estimated mean for each FREQ
    }
  #endif
}

ThompsonSampling::~ThompsonSampling() 
{
}

/**
 * @brief Picks the best SF according to Thompson Sampling algorithm
 * 
 * @return uint8_t 
 */
uint8_t ThompsonSampling::pull(uint8_t currentSF) {
  uint8_t bestSF = currentSF;
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
void ThompsonSampling::update(uint8_t currentSF, uint8_t reward)
{
  uint8_t idx = currentSF - MIN_SF;
  
  pulls[idx]++;
  rewards[idx] += reward * TS_SCALE;
  
  if (pulls[idx] > 0) {
    estimatedMeans[idx] = rewards[idx] / pulls[idx];
  }
}

#if FREQ_ENABLE
  uint8_t ThompsonSampling::freqPull(uint8_t currentFREQ) 
  {
    uint8_t bestFREQ = currentFREQ;
    uint16_t maxMean = freqEstimatedMeans[0];

    for (uint8_t idx = 0; idx < NUM_FREQ; idx++) {
      uint16_t sample = rand() % TS_SCALE; // Scaled random sample between 0 and TS_SCALE
      if (freqEstimatedMeans[idx] > maxMean && sample > maxMean) {
        maxMean = freqEstimatedMeans[idx];
        bestFREQ = idx;
      }

      /*
      Serial.print("TS sample for FREQ ");
      Serial.print(idx);
      Serial.print(" , ");
      Serial.println(sample);
      */
    }

    return bestFREQ;
  }

  void ThompsonSampling::freqUpdate(uint8_t currentFREQ, uint8_t reward)
  {
    uint8_t idx = idxToFreq(currentFREQ);

    /*
    Serial.print("TS reward for FREQ ");
    Serial.print(idx);
    Serial.print(" , ");
    Serial.println(reward);
    */

    freqPulls[idx]++;
    freqRewards[idx] += reward * TS_SCALE;
    
    if (freqPulls[idx] > 0) {
      freqEstimatedMeans[idx] = freqRewards[idx] / freqPulls[idx];
    }
  }
#endif