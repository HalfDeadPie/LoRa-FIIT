#include "UpperConfidenceBound.h"

void UpperConfidenceBound::init(uint8_t defaultSF)
{
  this->bestSF = defaultSF;
}

/**
 * Picks the best SF for sending the next message. Utilizes machine-learning Upper Confidence Bound (UCB) algorithm.
 * @return uint8_t
 */
uint8_t UpperConfidenceBound::pull() {
  float SFConfidenceBound;
  float confidenceInterval;
  float maxSFConfidenceBound = 0;

  if (allUplinkMsgs == 0) {
    return bestSF;
  }

  for (uint8_t idx = 0; idx < SF_ARRAY_SIZE; idx++)
  {
    if (sfSentMsgs[idx] > 0) {
      confidenceInterval = sqrt(3/2 * log(allUplinkMsgs + 1) / sfSentMsgs[idx]);
      SFConfidenceBound = sfSuccessRate[idx] + confidenceInterval;
    } else {
      SFConfidenceBound = MAX_CONFIDENCE_BOUND;
    }

    if (SFConfidenceBound > maxSFConfidenceBound) {
      maxSFConfidenceBound = SFConfidenceBound;
      bestSF = idx + LOWEST_SF;
    }
  }

  return bestSF;
}

void UpperConfidenceBound::update(uint8_t currentSF, bool sfMsgDelivered)
{
  uint8_t idx = currentSF - LOWEST_SF;
  
  if (sfMsgDelivered) {
    sfDeliveredMsgs[idx] += 1;
  }		
  
  sfSentMsgs[idx] += 1;	
  allUplinkMsgs += 1;
  
  updateSuccessRate(idx);
  return;
}

/**
 * Sets the success rate for each SF based on number of messages sent succesfuly to the number of all messages sent
 * @param index
 * @return
 */
void UpperConfidenceBound::updateSuccessRate(uint8_t idx)
{
  if (sfSentMsgs[idx] == 0) {
		return;
  }
	
	sfSuccessRate[idx] = (float) sfDeliveredMsgs[idx] / (float) sfSentMsgs[idx];
}
