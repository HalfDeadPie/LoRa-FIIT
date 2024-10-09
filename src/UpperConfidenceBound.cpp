#include "UpperConfidenceBound.h"

UpperConfidenceBound::UpperConfidenceBound()
{
  totalPulls = 0;

  for (uint8_t idx = 0; idx < NUM_SF; idx++) {
    rewards[idx] = 0;
    pulls[idx] = 0;
  }

  #if FREQ_ENABLE
    freqTotalPulls = 0;

    for (uint8_t idx = 0; idx < NUM_FREQ; idx++) {
      freqRewards[idx] = 0;
      freqPulls[idx] = 0;
    }
  #endif
}

UpperConfidenceBound::~UpperConfidenceBound()
{
}


/**
 * Picks the best SF for sending the next message. Utilizes machine-learning Upper Confidence Bound (UCB) algorithm.
 * @return uint8_t
 */
uint8_t UpperConfidenceBound::pull(uint8_t currentSF) {
  uint8_t bestSF = currentSF;
  int32_t maxUcbScore = ucbScore(currentSF);

  for (uint8_t sf = MIN_SF; sf <= MAX_SF; sf++) {
    int32_t score = ucbScore(sf);

    /*
    Serial.print("UCB score for SF ");
    Serial.print(sf);
    Serial.print(" , ");
    Serial.println(score);
    */

    if (score > maxUcbScore) {
      maxUcbScore = score;
      bestSF = sf;
    }
  }

  return bestSF;
}

int32_t UpperConfidenceBound::ucbScore(uint8_t sf) {
  uint8_t idx = sf - MIN_SF;

  if (pulls[idx] == 0) {
    return INT32_MAX;  // A large value for unexplored SFs
  }
  
  int32_t meanReward = (rewards[idx] * UCB_SCALE) / pulls[idx];
  int32_t explorationTerm = (int32_t) (integerSqrt((uint32_t)(log(totalPulls * UCB_SCALE) * UCB_SCALE)) / pulls[idx]);
  return meanReward + explorationTerm;
}

void UpperConfidenceBound::update(uint8_t sf, int32_t reward) {
  uint8_t idx = sf - MIN_SF; // Convert SF to index
  pulls[idx]++;
  rewards[idx] += reward;
  totalPulls++;
}

#if FREQ_ENABLE
  uint8_t UpperConfidenceBound::freqPull(uint8_t currentFreq) {
    uint8_t bestFreq = currentFreq;
    int32_t maxUcbScore = freqUcbScore(currentFreq);

    for (uint8_t idx = 0; idx < NUM_FREQ; idx++) {
      int32_t score = freqUcbScore(idxToFreq(idx));

      /*
      Serial.print("UCB score for FREQ ");
      Serial.print(idx);
      Serial.print(" , ");
      Serial.println(score);
      */

      if (score > maxUcbScore) {
        maxUcbScore = score;
        bestFreq = idx;
      }
    }

    return idxToFreq(bestFreq);
  }

  int32_t UpperConfidenceBound::freqUcbScore(uint8_t freq) {
    uint8_t idx = freqToIdx(freq);

    if (freqPulls[idx] == 0) {
      return INT32_MAX;  // A large value for unexplored SFs
    }
    
    int32_t meanReward = (freqRewards[idx] * UCB_SCALE) / freqPulls[idx];
    int32_t explorationTerm = (int32_t) (integerSqrt((uint32_t)(log(freqTotalPulls * UCB_SCALE) * UCB_SCALE)) / freqPulls[idx]);
    return meanReward + explorationTerm;
  }

  void UpperConfidenceBound::freqUpdate(uint8_t freq, int32_t reward) {
    uint8_t idx = freqToIdx(freq);
    freqPulls[idx]++;
    freqRewards[idx] += reward;
    freqTotalPulls++;
  }
#endif

uint32_t UpperConfidenceBound::integerSqrt(uint32_t x) {
  uint32_t y = x;
  while (y > (x / y)) {
    y = (y + (x / y)) / 2;
  }
  return y;
}