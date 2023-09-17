#include "Bandit.h"

Bandit::Bandit() 
{

}

Bandit::~Bandit() 
{

}

uint8_t Bandit::idxToFreq(uint8_t idx) {
  switch (idx) {
    case 0: return 31;
    case 1: return 33;
    case 2: return 35;
    case 3: return 37;
    case 4: return 39;
    default: return 31;
  }
}

uint8_t Bandit::freqToIdx(uint8_t freq) {
  switch (freq) {
    case 31: return 0;
    case 33: return 1;
    case 35: return 2;
    case 37: return 3;
    case 39: return 4;
    default: return 0;
  }
}