#ifndef lora_h
#define lora_h

#define DEBUG 1

#include <SPI.h>
#include "RH_RF95.h"
#include "DH.h"
#include "Encryption.h"
#include <EEPROM.h>

#define RECEIVE_TIMEOUT 3000
#define MAX_TX_POWER 14
#define INIT_TX_POWER 13
#define DEFAULT_BW 125
#define DEFAULT_SF 9
#define DEFAULT_CR 5
#define FREQ_ARRAY_SIZE 10
#define SEQ_DIFF 10

// Hardcoded device ID value
/*
#define DEVICE_ID1 0x41
#define DEVICE_ID2 0x41
#define DEVICE_ID3 0x41
*/

// Registration channels
#define REG_CHANNEL_1  866.1 // 866.10MHz
#define REG_CHANNEL_2  863.8 // 866.40MHz
#define REG_CHANNEL_3 866.5 // 866.70MHz
#define  REG_CHANNEL_4  866.7 // 867.00MHz
#define  REG_CHANNEL_5  866.9 // 868.00MHz

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
#define Number_Of_SF 0x06//total number of spreading factors used in LoRa@FIIT protocol

#define Max_Confidence_Bound 0x0A//maximal confidence bound for UCB to ensure exploration 
#define SF_indexer 0x07//number of lowest spreading factor used in LoRa@FIIT protocol
#define Default_SF 0x07//number of lowest spreading factor used in LoRa@FIIT protocol

// Coding rate
const byte CR_5 = B00000010; // 4/5
const byte CR_6 = B00000100; // 4/6
const byte CR_7 = B00000110; // 4/7
const byte CR_8 = B00001000; // 4/8

// Bandwidth
const byte BW_7_8 = B00000000; // 7.8 kHz
const byte BW_10_4 = B00010000; // 10.4 kHz
const byte BW_15_6 = B00100000; // 15.6 kHz
const byte BW_20_8 = B00110000; // 20.8 kHz
const byte BW_31_25 = B01000000; // 31.25 kHz
const byte BW_41_7 = B01010000; // 41.7 kHz
const byte BW_62_5 = B01100000; // 62.5 kHz
const byte BW_125 = B01110000; // 125 kHz
const byte BW_250 = B10000000; // 250 kHz
const byte BW_500 = B10010000; // 500 kHz

// Spreading factor
const byte SF_6 = B01100000; // 2^6 = 64 chips / symbol
const byte SF_7 = B01110000; // 2^7 = 128 chips / symbol
const byte SF_8 = B10000000; // 2^8 = 256 chips / symbol
const byte SF_9 = B10010000; // 2^9 = 512 chips / symbol
const byte SF_10 = B10100000; // 2^10 = 1024 chips / symbol
const byte SF_11 = B10110000; // 2^11 = 2048 chips / symbol
const byte SF_12 = B11000000; // 2^12 = 4096 chips / symbol

/** Structure: Actual global configuration */
struct netconfig {
	uint8_t freqData[FREQ_ARRAY_SIZE];
	uint8_t freqReg[FREQ_ARRAY_SIZE];
	uint8_t freqEmer[FREQ_ARRAY_SIZE];
	
	uint8_t freqDataSize;
	uint8_t freqRegSize;
	uint8_t freqEmerSize;
	
	uint8_t bwData;
	uint8_t crData;
	uint8_t sfData;
	uint8_t pwData;
	
	uint8_t bwReg;
	uint8_t crReg;
	uint8_t sfReg;
	uint8_t pwReg;
	
	uint8_t bwEmer;
	uint8_t crEmer;
	uint8_t sfEmer;
	uint8_t pwEmer;
};

class lora : private RH_RF95
{

  public:
    /** Parameters are Slave-Select pin, Interrupt pin for Tx and Rx done, Reset pin */
    lora(uint8_t slaveSelectPin, uint8_t interruptPin, uint8_t resetPin);

    /** Reset and Initialization of the chip */
    void On();

    /** Set the default settings (bw=125,  sf=7, cr=4/5) */
    void SetDefault();

    /** Set the bandwidth */
    bool SetBW(float bandwidth);

    /** Set the coding rate - 4/parameter */
    bool SetCR(uint8_t codingrate);

    /** Set the spreading factor */
    bool SetSF(uint8_t spreadingfactor);

    /** Set the frequency */
    bool SetFrequency(float frequency);

    /** Set the transciever power */
    bool SetPW(uint8_t power, bool useRF0);

    void Awake();

    void Sleep();
    void SetManual(bool value);

    /** Send basic data message with ack */
    bool Send(uint8_t* data, uint8_t &len);

    /** Send the specific message */
    bool Send(uint8_t type, uint8_t ack, uint8_t* data, uint8_t &len);

    bool SendHello(uint8_t* data, uint8_t &len);
    bool SendEmergency(uint8_t* data, uint8_t &len);
    unsigned long Getsendtime();

    /** Turn on receiving */
    bool Receive(uint8_t* buf, uint8_t &len);

    /** Register the device */
    bool Register(uint8_t* buffer, uint8_t &len);

    /** Returns the time while the device should not be able to send anything because of duty cycle */
    unsigned long GetDutyWait();

    /** Duty cycle handler, returns time on air for the message. This should be used if _manual = true because of respecting the duty cycle */
    uint32_t WaitDutyCycle(uint8_t len, float bw, uint8_t sf, uint8_t cr, uint8_t type);

  private:
    /** Reset pin - used for reset in On() */
    uint8_t _resetPin;

    bool _manual;

    /** Stores number of all uplink messages send on each Spreading Factor*/
    unsigned int SF_allSentMsg[7];
    /** Stores number of all uplink messages successfully send and decoded by gateway on each Spreading Factor*/
    unsigned int SF_OkSentMsg[7];
    /** Stores the ration of successfully send messages to all send messages for each Spreading Factor*/
    float SF_successRate[7];
    /** Holds the number of all uplink messages with required ACK or the opening of recieve window send since modem was turned On*/
    unsigned int _allSentMessages;

    /** Holds the value of Spreading Factor currently used to send uplink message on*/
    uint8_t currentSF;

    /** Holds the current bandwidth on which uplink messages are being send*/
    float bwDC;
    /** Holds the current duty cycle percentage as defined by ETSI regulations on which uplink messages are being send*/
    uint8_t percentageDC;
    /** Holds the current coding rate on which uplink messages are being send*/
    uint8_t crDC;
    /** Holds the maximum possible value of Coding Rate*/
    uint8_t maxCR_DC = 8;
    /** Holds the current spreading factor on which uplink messages are being send*/
    uint8_t sfDC;

    /** Diffie-Hellman key */
    DH dhkey1;

    /** Actual sequence number */
    uint16_t _sequence_number;

    /** Available sendtime */
    unsigned long _sendtime;

    uint8_t testvar;

    /** Set the device ID in message */
    void setDEVID(uint8_t* message);

    /** Set the message type in message */
    void setType(uint8_t* message, uint8_t type);

    /** Set the ACK in message */
    void setACK(uint8_t* message, uint8_t ack);

    /** Message handler function */
    bool ProcessMessage(uint8_t* dataout, uint8_t &len, bool reg);

    /** Network reconfiguration handler */
    void ProcessNetworkData(uint8_t* data, uint8_t len, bool reg);

    /** Check the sequence number */
    bool CheckSequence(uint16_t seq);

    /** Loading network configuration from global configuration structure */
    uint32_t LoadNetworkData(uint8_t type, uint8_t len);

    /** Returns the message length */
    uint8_t GetMessageLength(uint8_t len);

    /** Returns the 0 if Spreading Factor does not change otherwise it returns maximum transmission time on a given SF */
    uint8_t pickBestSF(float bw);

    /** Sets minimal CAD duration based on given Spreading Factor */
    void SetCADDuration(uint8_t spreadingfactor);

    /** Returns minimal CAD duration based on given Spreading Factor and Bandwidth */
    unsigned long lora::CalculateCadDuration(uint8_t spreadingfactor, float bw);

    /** Returns the Spreading Factor success rate */
    void calculateSFsuccessRate();

    /** Returns the Spreading Factor success rate based on number of succesfully sent messages on that Spreading Factor to number of all messages sent on that Spreading Factor */
    float getSFsuccessRate(uint8_t okSentMessages, uint8_t allSentMessages);

    /** sets message rate for Spreading Factor, number of succesfully send messages and number of all messages sent */
    bool messageSuccesfullySendOnSF(bool successfullySent);

    /** Returns maximum transmission time for packet, maximum time for how long medium will be used by other device when transmission is detected */
    uint8_t getMaximumTransmissionTime(float bw, uint8_t sf);

    /** Returns maximum length of application data in a packet based on communication parameters */
    uint8_t getMaxLen(float bw, uint8_t sf);

    /** sets success rate for each Spreading Factor to zero */
    void clearSFsuccessRate();

    /** Common sending function */
    bool SendMessage(uint8_t type, uint8_t ack, uint8_t* data, uint8_t &len);

    /** Upper Confidence Bound algorithmic function to determine the best Spreading Factor to send the next message on */
    uint8_t UCB();
};

#endif