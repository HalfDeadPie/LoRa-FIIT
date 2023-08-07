#include "ThompsonSampling.h"

void ThompsonSampling::init(uint8_t bestSF)
{

}

/**
 * Picks the best SF for sending the next message. Utilizes machine-learning Upper Confidence Bound (UCB) algorithm.
 * @return uint8_t
 */
uint8_t ThompsonSampling::pull() {

}

void ThompsonSampling::update(uint8_t currentSF, bool sfMsgDelivered)
{

}

/**
 * Sets the success rate for each SF based on number of messages sent succesfuly to the number of all messages sent
 * @param idx
 * @return
 */
void ThompsonSampling::updateSuccessRate(uint8_t idx)
{

}
