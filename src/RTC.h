#ifndef RTC_h
#define RTC_h

#include <stdint.h> 
#include <avr/io.h>
#include <util/delay.h>

// Define DS1302 control pins
#define RTC_DAT_PIN 6
#define RTC_CLK_PIN 5
#define RTC_RST_PIN 7

#define RTC_WRITE_ENABLE  0x8E
#define RTC_WRITE_DISABLE 0x8F
#define RTC_READ_ENABLE   0xBF
#define RTC_READ_DISABLE  0xBE

class RTC
{
  public:
    RTC();
    ~RTC();

    void setup();
    void readTime(uint8_t *timeData);
    void setTime(uint8_t seconds, uint8_t minutes, uint8_t hours, uint8_t dayOfWeek, uint8_t dayOfMonth, uint8_t month, uint8_t year);
    unsigned long millis();
    uint8_t bcdToDec(uint8_t val);

  private:
    void writeByte(uint8_t data);
    uint8_t readByte();

};

#endif