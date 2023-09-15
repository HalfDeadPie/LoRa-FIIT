#include "UpperConfidenceBound.h"

UpperConfidenceBound::UpperConfidenceBound()
{
  totalPulls = 0;

  for (uint8_t idx = 0; idx < NUM_SF; idx++) {
    rewards[idx] = 0;
    pulls[idx] = 0;
  }
}

UpperConfidenceBound::~UpperConfidenceBound()
{
}


/**
 * Picks the best SF for sending the next message. Utilizes machine-learning Upper Confidence Bound (UCB) algorithm.
 * @return uint8_t
 */
uint8_t UpperConfidenceBound::pull() {
  uint8_t bestSF = MIN_SF;
  int32_t maxUcbScore = ucbScore(MIN_SF);

  for (uint8_t sf = MIN_SF + 1; sf <= MAX_SF; sf++) {
    int32_t score = ucbScore(sf);

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

uint32_t UpperConfidenceBound::integerSqrt(uint32_t x) {
  uint32_t y = x;
  while (y > (x / y)) {
    y = (y + (x / y)) / 2;
  }
  return y;
}