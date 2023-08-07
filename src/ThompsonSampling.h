#ifndef ThompsonSampling_h
#define ThompsonSampling_h

#include "Bandit.h"

#define RECEIVE_TIMEOUT 3000
#define MAX_TX_POWER 14
#define INIT_TX_POWER 13
#define DEFAULT_BW 125
#define DEFAULT_SF 9
#define DEFAULT_CR 5
#define FREQ_ARRAY_SIZE 10
#define SEQ_DIFF 10

#define CAD_ENABLED 0
#define MAB_UCB_ENABLED 0

// Registration channels
#define REG_CHANNEL_1  866.1 // 866.10MHz
#define REG_CHANNEL_2  863.8 // 866.40MHz
#define REG_CHANNEL_3 866.5 // 866.70MHz
#define REG_CHANNEL_4  866.7 // 867.00MHz
#define REG_CHANNEL_5  866.9 // 868.00MHz

#define TYPE_REG_UP 0x00
#define TYPE_DATA_UP 0x20
#define TYPE_HELLO_UP 0x40
#define TYPE_EMER_UP 0x60
#define TYPE_REG_DOWN 0x80
#define TYPE_DATA_DOWN 0xA0

#define MASK_TYPE 0xE0
#define MASK_ACK 0x07
#define MASK_MSB B11110000
#define MASK_LSB B00001111

#define ACK_NO 0x00
#define ACK_OPT 0x02
#define ACK_MAN 0x06

#define REC_D_BW 0x10
#define REC_D_CR 0x20
#define REC_D_FRQ 0x30
#define REC_D_PW 0x40
#define REC_D_SF 0x50

#define REC_R_BW 0x60
#define REC_R_CR 0x70
#define REC_R_FRQ 0x80
#define REC_R_PW 0x90
#define REC_R_SF 0xA0

#define REC_E_BW 0xB0
#define REC_E_CR 0xC0
#define REC_E_FRQ 0xD0
#define REC_E_PW 0xE0
#define REC_E_SF 0xF0

#define SF7 0x00
#define SF8 0x01
#define SF9 0x02
#define SF10 0x03
#define SF11 0x04
#define SF12 0x05
#define SF_ARRAY_SIZE 0x06 

#define PW_ARRAY_SIZE 15

class ThompsonSampling: private Bandit
{

  public:
    ThompsonSampling();
    ~ThompsonSampling();

    void init(uint8_t bestSF);
    uint8_t pull();
    void update(uint8_t currentSF, bool sfMsgDelivered);

  private:
    float sfSuccess[SF_ARRAY_SIZE];
    float freqSuccess[FREQ_ARRAY_SIZE];
    float pwSuccess[PW_ARRAY_SIZE];

    void updateSuccessRate(uint8_t idx);
};

#endif