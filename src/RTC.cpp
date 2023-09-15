#include "RTC.h"

/**
 * @brief Construct a new RTC:: RTC object
 * 
 */
RTC::RTC() 
{
  setup();
}

RTC::~RTC() 
{
  
}

void RTC::setup() 
{
  DDRD |= (1 << RTC_DAT_PIN) | (1 << RTC_CLK_PIN) | (1 << RTC_RST_PIN);
}

void RTC::readTime(uint8_t *timeData)
{
  PORTD &= ~(1 << RTC_RST_PIN);
  _delay_us(2);
  PORTD |= (1 << RTC_RST_PIN);
  _delay_us(2);
  
  writeByte(RTC_READ_ENABLE);
  
  for (uint8_t i = 0; i < 7; i++) {
    timeData[i] = readByte();
  }
  
  PORTD &= ~(1 << RTC_RST_PIN);
  _delay_us(2);
}

void RTC::setTime(uint8_t seconds, uint8_t minutes, uint8_t hours, uint8_t dayOfWeek, uint8_t dayOfMonth, uint8_t month, uint8_t year) 
{
  writeByte(RTC_WRITE_ENABLE);

  writeByte(seconds);
  writeByte(minutes);
  writeByte(hours);
  writeByte(dayOfWeek);
  writeByte(dayOfMonth);
  writeByte(month);
  writeByte(year);

  writeByte(RTC_WRITE_DISABLE);
}

unsigned long RTC::millis()
{
  // Array to store time data (seconds, minutes, hours, etc.)
  uint8_t timeData[7]; 
  readTime(timeData);
  
  // Extract time values from RTC clock
  uint8_t seconds = timeData[0];
  uint8_t minutes = timeData[1];
  uint8_t hours = timeData[2];
  
  // Calculate and return total milliseconds
  return (hours * 3600UL + minutes * 60UL + seconds) * 1000UL;
}

void RTC::writeByte(uint8_t data)
{
  for (uint8_t i = 0; i < 8; i++) {
    if (data & 0x01) {
      PORTD |= (1 << RTC_DAT_PIN);
    } else {
      PORTD &= ~(1 << RTC_DAT_PIN);
    }
  
    PORTD |= (1 << RTC_CLK_PIN);
    _delay_us(2);
    PORTD &= ~(1 << RTC_CLK_PIN);
    _delay_us(2);
    
    data >>= 1;
  }
}

uint8_t RTC::readByte()
{
  uint8_t data = 0;
  
  for (uint8_t i = 0; i < 8; i++) {
    data >>= 1;
    if (PIND & (1 << RTC_DAT_PIN)) {
      data |= RTC_READ_ENABLE;
    }
    
    PORTD |= (1 << RTC_CLK_PIN);
    _delay_us(2);
    PORTD &= ~(1 << RTC_CLK_PIN);
    _delay_us(2);
  }
  
  return data;
}

uint8_t RTC::bcdToDec(uint8_t val) {
  return (val / 16 * 10 + val % 16);
}

