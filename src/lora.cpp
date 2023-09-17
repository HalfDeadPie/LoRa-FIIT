#include "lora.h"

/**
 * Constructor: Parameters are Slave-select pin, interrupt pin, reset pin
 * @param slaveSelectPin
 * @param interruptPin
 * @param resetPin
 */
lora::lora(uint8_t slaveSelectPin, uint8_t interruptPin, uint8_t resetPin)
:
	RH_RF95(slaveSelectPin, interruptPin)
{
	_resetPin = resetPin;

  // Temporary to clear EEPROM when switching between ADR and MAB approach
  /*
  for (int i = 0; i < EEPROM.length(); i++) {
    EEPROM.write(i, 0);
  }
  */
}

unsigned long lora::Getsendtime() {
  return _sendtime;
}

/**
 * Reset and initialization
 */
void lora::On() {
  pinMode(_resetPin, OUTPUT);
  digitalWrite(_resetPin, HIGH);

  // manual reset
  digitalWrite(_resetPin, LOW);
  delay(10);
  digitalWrite(_resetPin, HIGH);
  delay(10);

  while (!init()) {
	  #if SERIAL_DEBUG
    	Serial.println(F("LoRa radio init failed"));
    #endif
	  while (1);
  }

  #if SERIAL_DEBUG
    Serial.println(F("LoRa radio init OK!"));
  #endif

  // If you set manual to true, you are resposible for setting the frequency, coding rate, bandwidth, spreading factor and respecting the duty cycle!
  _manual = false;
  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
  // you can set transmitter powers from 5 to 23 dBm:
  setTxPower(INIT_TX_POWER, false);	
  
  #if CAD_ENABLED
	// This line enables CAD functionality but setting timer different than zero
	// In default state, CAD is set to 0
    setCADTimeout(CAD_TIMEOUT);
  #endif

  // Initiate UCB arrays
  #if MAB_UCB_ENABLED 
    mab = UpperConfidenceBound();
  #elif MAB_TS_ENABLED 
    mab = ThompsonSampling();
  #endif
}

/**
 * Setting the default communication parameters
 */
void lora::SetDefault() {
	SetFrequency(REG_CHANNEL_1);
	SetBW(DEFAULT_BW);
	SetSF(DEFAULT_SF);	
	SetCR(DEFAULT_CR);	
}

/**
 * LoRa module is put to sleep
 */
void lora::Sleep() {
	spiWrite(RH_RF95_REG_01_OP_MODE, RH_RF95_MODE_SLEEP);
}

/**
 * LoRa module is awaken
 */
void lora::Awake() {
  spiWrite(RH_RF95_REG_01_OP_MODE, RH_RF95_MODE_STDBY);
}

/**
 * Set the bandwidth
 * @param bandwidth
 */
bool lora::SetBW(float bandwidth) {
  #if SERIAL_DEBUG
    Serial.print(F("Setting BW: "));
    Serial.println(bandwidth);
  #endif

  bool state = false;
  byte value = spiRead(RH_RF95_REG_1D_MODEM_CONFIG1);
  byte bw;
  
	if (bandwidth == 7.80) {
	  bw = BW_7_8;
	} else if (bandwidth == 10.40) {
	  bw = BW_10_4;
	} else if (bandwidth == 15.60)	{
	  bw = BW_15_6;
	} else if (bandwidth == 20.80) {
    bw = BW_20_8;
  } else if (bandwidth == 31.25) {
	  bw = BW_31_25;
	} else if (bandwidth == 41.70) {
	  bw = BW_41_7;
	} else if (bandwidth == 62.50)	{
	  bw = BW_62_5;
	} else if (bandwidth == 125.00)	{
	  bw = BW_125;
	} else if (bandwidth == 500.00)	{
	  bw = BW_500;
	} else {
	  return false;
	}

  value &= B00001111;
  value |= bw;
  spiWrite(RH_RF95_REG_1D_MODEM_CONFIG1, value);
  value = spiRead(RH_RF95_REG_1D_MODEM_CONFIG1);

  // Getting the bandwidth bimab only
  value &= B11110000;

  if (value == bw) {
    return true;
  }
  return state;
}

/**
 * Set the coding rate
 * @param codingRate
 */
bool lora::SetCR(uint8_t codingRate) {
	#if SERIAL_DEBUG
	  Serial.print(F("Setting CR0: "));	
	  Serial.println(codingRate);
  #endif

  bool state = false;
  byte value = spiRead(RH_RF95_REG_1D_MODEM_CONFIG1);
  byte cr;

	switch (codingRate) {
		case 5: cr = CR_5; break;
		case 6: cr = CR_6; break;
		case 7: cr = CR_7; break;
		case 8: cr = CR_8; break;
		default: return false;
	}
  
  // Clear bimab for coding rate
  value &= B11110001;
  // Set coding rate bits
  value |= cr;
  spiWrite(RH_RF95_REG_1D_MODEM_CONFIG1, value);
  value = spiRead(RH_RF95_REG_1D_MODEM_CONFIG1);
  // Getting only coding rate bits
  value &= B00001110;

  if (value == cr) {
    return true;
  }

  return state;
}

/**
 * Setting the transmission power
 * @param power
 * @param useRF0
 */
bool lora::SetPW(uint8_t power, bool useRF0) {
  #ifdef SERIAL_DEBUG
    Serial.print(F("Setting PW: "));
    Serial.println(power);
  #endif
	setTxPower(power, useRF0);
	return false;
}

/**
 * Set the spreading factor
 * @param spreadingFactor
 * @return
 */
bool lora::SetSF(uint8_t spreadingFactor) {
  #ifdef SERIAL_DEBUG
    Serial.print(F("Setting SF: "));
    Serial.println(spreadingFactor);
  #endif

  byte value = spiRead(RH_RF95_REG_1E_MODEM_CONFIG2);
  byte value_mobile = spiRead(RH_RF95_REG_26_MODEM_CONFIG3);
  byte sf;
  bool mobile;

	switch (spreadingFactor) {
		case 6:
		  sf = SF_6;
		  mobile = false;
		  break;
		case 7:
		  sf = SF_7;
		  mobile = false;
		  break;
		case 8:
		  sf = SF_8;
		  mobile = false;
		  break;
		case 9:
		  sf = SF_9;
		  mobile = false;
		  break;
		case 10:
		  sf = SF_10;
		  mobile = false;
		  break;
		case 11:
		  sf = SF_11;
		  mobile = true;
		  break;
		case 12:
		  sf = SF_12;
		  mobile = true;
		  break;
		default:
		  return false;
	}

  value_mobile &= B11110011;

  // Mobile end nodes are those with SF 11 or 12
	if (mobile) {
		value_mobile |= B00001100;
	} else {
		value_mobile |= B00000100;
	}

  spiWrite(RH_RF95_REG_26_MODEM_CONFIG3, value_mobile);
	
  value &= B00001111;
  value |= sf;

  spiWrite(RH_RF95_REG_1E_MODEM_CONFIG2, value);
  value = RHSPIDriver::spiRead(RH_RF95_REG_1E_MODEM_CONFIG2);
  value &= B11110000;

  #if CAD_ENABLED
    SetCADDuration(spreadingFactor);
  #endif

  if (value == sf) {
	  currentSF = spreadingFactor;
    return true;
  }

  return false;
}

#if CAD_ENABLED
  /**
   * Semab minimal CAD duration based on given Spreading Factor
   * @param spreadingFactor
   * @return
   */
  void lora::SetCADDuration(uint8_t spreadingFactor) {
    unsigned long timeout = CalculateCadDuration(spreadingFactor, bwDC);
    
    #if SERIAL_DEBUG
      Serial.print(F("CAD timeout: "));
      Serial.println(timeout);
    #endif
    
    setCADTimeout(timeout);
    // setCADTimeout(CAD_TIMEOUT);
  }

  /**
   * Calculates minimal CAD duration based on given Spreading Factor and Bandwidth settings
   * @return
   */
  unsigned long lora::CalculateCadDuration(uint8_t spreadingFactor, float bw) {
    uint32_t minimal_duration = (1UL << spreadingFactor) + 32;
    minimal_duration = (minimal_duration + (uint32_t)(bw * 0.5)) / (uint32_t) bw;
    return minimal_duration + 5;
  }
#endif


/**
 * Set the frequency
 * @param frequency
 * @return
 */
bool lora::SetFrequency(float frequency) {
  #ifdef SERIAL_DEBUG
    Serial.print(F("Setting FREQ: "));
    Serial.println(frequency);
  #endif
	return setFrequency(frequency);
}

/**
 * Sending the common message
 * @param type
 * @param ack
 * @param data
 * @param len
 */
bool lora::SendMessage(uint8_t type, uint8_t ack, uint8_t* data, uint8_t &len) {
	uint8_t payload[RH_RF95_MAX_PAYLOAD_LEN];
	bool sendOk = false;
	setDEVID(payload);
	setType(payload, type);
	setACK(payload, ack);
	
	uint8_t* datapointer = &payload[4];
	*datapointer = len;
	datapointer++;
	memcpy(datapointer, data, len);
	
	datapointer += len;//jump to seq
	uint16_t* seqpointer = (uint16_t*) datapointer;
	_sequence_number++;
	
  #if SERIAL_DEBUG
    Serial.print(F("Sequence number: "));
	  Serial.println(_sequence_number);
	#endif

  *seqpointer = _sequence_number;
	
	datapointer += sizeof(uint16_t);
	uint32_t* micpointer = (uint32_t*) datapointer;
	*micpointer = Encryption::generateMIC(&payload[4], len + 3);//applen + payloadlen + seq
	
	uint8_t actuallen = len + 7; //applen + payloadlen + seq + mic
	int8_t paddinglen;

	if (actuallen < 8) {
		paddinglen = 8 - actuallen;
	} else {
		paddinglen = actuallen % 4;
		if (paddinglen != 0) {
			paddinglen = 4 - paddinglen;
		}
	}

	datapointer += sizeof(uint32_t);

	for (int i = 0; i< paddinglen; i++) {
		*datapointer = random();
		datapointer++;
	}
	
	Encryption::encrypt(&payload[4], actuallen + paddinglen, dhkey1.session_private_key);
  
  #if SERIAL_DEBUG
	  Serial.println(F("Waiting for packet to complete..."));
	#endif
  
  delay(10);

  #if CSV_OUTPUT
    Serial.print(_sequence_number);
    Serial.print(",");
    Serial.print(freqDataDC);
    Serial.print(",");
    Serial.print(pwDC);
    Serial.print(",");
    Serial.println(sfDC);
  #endif

	sendOk = send(payload, 4 + actuallen + paddinglen);
	waitPacketSent();
	
	return sendOk;
}

/**
 * Calculate the length of message
 * @param len
 * @return
 */
uint8_t lora::GetMessageLength(uint8_t len) {
	uint8_t actuallen = len + 7;
	int8_t paddinglen;

	if (actuallen < 8) {
		paddinglen = 8 - actuallen;
	} else {
		paddinglen = actuallen % 4;
		if (paddinglen != 0) {
			paddinglen = 4 - paddinglen;
		}
	}

	return 3 + 1 + actuallen + paddinglen;
}


#if MAB_UCB_ENABLED || MAB_TS_ENABLED || CAD_ENABLED
/**
 * Returns maximum transmission time of a packet on the channel based on communications parameters
 * @param bw
 * @param sf
 * @return
 */
uint8_t lora::getMaximumTransmissionTime(float bw, uint8_t sf) {
	uint8_t len = getMaxLen(bw, sf);

	float tsymbol = integerPow(2, sf) / (bw * 1000);
	tsymbol *= 1000;
	uint8_t optimalization = sf > 10 ? 1 : 0;
	
	float tpreamble = (8 + 4.25) * tsymbol;
	float payloadSymbNb = 8*len - 4*sf + 28 +16;
	payloadSymbNb /= (4* (sf - 2*optimalization));
	
	float temp = round(payloadSymbNb);

	if (temp < payloadSymbNb) {
		payloadSymbNb = temp + 1;
	} else {
		payloadSymbNb = temp;
	}
	
	float payload = 8 + max(payloadSymbNb*maxCR_DC, 0);
	payload *= tsymbol;

	return (payload + tpreamble);
}
#endif

#if CAD_ENABLED || MAB_TS_ENABLED || MAB_UCB_ENABLED
/**
 * Returns maximal packet length based on communications parameters
 * @param bw
 * @param sf
 * @return
 */
uint8_t lora::getMaxLen(float bw, uint8_t sf) {
	uint8_t maxLen = 255;

	if (bw == 125.0) {
		switch (sf) {
      case 7:
        maxLen = 222;
        break;
      case 8:
        maxLen = 222;
        break;
      case 9:
        maxLen = 115;
        break;
      case 10:
        maxLen = 51;
        break;
      case 11:
        maxLen = 51;
        break;
      case 12:
        maxLen = 51;
        break;
      default:
        break;
		}
	} else if (bw == 250.0) {
		switch (sf) {
      case 7:
        maxLen = 222;
        break;
      default:
        break;
		}
	}

	return maxLen;
}
#endif

/**
 * Returns the time, after which will be device able to send data (duty cycle)
 * @return
 */
unsigned long lora::GetDutyWait() {
	if (_sendtime > millis()) {
		return 10 + _sendtime - millis();
	}
	return 0;
}

/**
 * Automatic send
 * @param data
 * @param len
 * @return
 */
bool lora::Send(uint8_t* data, uint8_t &len) {
	uint32_t time = 0;
	bool message_sent = false;

	if (_sendtime < millis()) {
    #if SERIAL_DEBUG
		  Serial.println();
		  Serial.println(F("Sending message..."));
		#endif

    #if MAB_UCB_ENABLED || MAB_TS_ENABLED
      LoadNetworkData();
      if (SendMessage(TYPE_DATA_UP, ACK_OPT, data, len)) { // No CAD was detected, sending message
        time = WaitDutyCycle(GetMessageLength(len), bwDC, sfDC, crDC, TYPE_DATA_UP);
        message_sent = true;
      } else { // A CAD was detected, switching to another channel
        time = getTimeForBestSF(bwDC, sfDC);
      }
    #else
      LoadNetworkData(TYPE_DATA_UP, GetMessageLength(len));
      SendMessage(TYPE_DATA_UP, ACK_OPT, data, len);
      time = WaitDutyCycle(GetMessageLength(len), bwDC, sfDC, crDC, TYPE_DATA_UP);
      message_sent = true;
    #endif

		_sendtime = millis() +  time;

		if (message_sent) {
      #if MAB_UCB_ENABLED || MAB_TS_ENABLED
        if (Receive(data, len)) {
          mab.update(sfDC, 1);
          mab.freqUpdate(freqIdxDC, 1);
        } else {
          mab.update(sfDC, 0);
          mab.freqUpdate(freqIdxDC, 0);
        }
      #else
        Receive(data, len);
      #endif
		}
		
		return message_sent;
	}

  #if SERIAL_DEBUG
	  Serial.print(F("Duty cycle: "));
	  Serial.println(GetDutyWait());
	#endif

  return message_sent;
}

/**
 * Specific send
 * @param type
 * @param ack
 * @param data
 * @param len
 * @return
 */
bool lora::Send(uint8_t type, uint8_t ack, uint8_t* data, uint8_t &len) {
  uint32_t time = 0;
  bool message_sent = false;

  // if current time is more than next available sendtime
  if (_sendtime < millis()) { 
    // calculates next available send time based on time it will take to send the message and what is the frequency range duty cycle

    #if MAB_UCB_ENABLED || MAB_TS_ENABLED
      LoadNetworkData();

      if (SendMessage(TYPE_DATA_UP, ACK_OPT, data, len)) { 
        time = WaitDutyCycle(GetMessageLength(len), bwDC, sfDC, crDC, TYPE_DATA_UP);
        message_sent = true;
      } else {
        time = getTimeForBestSF(bwDC, sfDC);
      }
    #else
      LoadNetworkData(type, GetMessageLength(len));
      SendMessage(TYPE_DATA_UP, ACK_OPT, data, len);
      time = WaitDutyCycle(GetMessageLength(len), bwDC, sfDC, crDC, TYPE_DATA_UP);
      message_sent = true;
    #endif

		_sendtime = millis() + time;

		if (ack == ACK_MAN) {
			uint8_t temp = len;
			if (message_sent == false || Receive(data, len) == false) {
				len = temp;

        #if MAB_UCB_ENABLED || MAB_TS_ENABLED
				  mab.update(sfDC, 0);
          mab.freqUpdate(freqIdxDC, 0);
        #endif

        return SendEmergency(data, len);
			}

      #if MAB_UCB_ENABLED || MAB_TS_ENABLED
			  mab.update(sfDC, 1);
        mab.freqUpdate(freqIdxDC, 1);
      #endif 
		} else if (ack == ACK_OPT && message_sent == true) {
			Receive(data, len);
		} else {
      #if CAD_ENABLED
        len = 0;
        #if SERIAL_DEBUG
          Serial.println(F("CAD detected message not sent"));
          Serial.print(F("Message sent: "));
          Serial.println(message_sent);
        #endif

        return message_sent;
      #endif
		}
    
		return message_sent;
	}

  #if SERIAL_DEBUG
    Serial.print(F("Next uplink message in less than "));
    Serial.print(GetDutyWait());
    Serial.println(F(" ms"));
  #endif

  return message_sent;
}

/**
 * Hello function
 * @param data
 * @param len
 * @return
 */
bool lora::SendHello(uint8_t* data, uint8_t &len) {
	uint32_t time = 0;
	bool message_sent = false;

	if (_sendtime < millis()) {
    #if MAB_UCB_ENABLED || MAB_TS_ENABLED
      LoadNetworkData();
      if (SendMessage(TYPE_HELLO_UP, ACK_OPT, data, len)) {
        time = WaitDutyCycle(GetMessageLength(len), bwDC, sfDC, crDC, TYPE_HELLO_UP);
        message_sent = true;
      } else {
        time = getTimeForBestSF(bwDC, sfDC);
      }
    #else
      LoadNetworkData(TYPE_HELLO_UP,len);
      SendMessage(TYPE_HELLO_UP, ACK_OPT, data, len);
      time = WaitDutyCycle(GetMessageLength(len), bwDC, sfDC, crDC, TYPE_HELLO_UP);
      message_sent = true;
    #endif

		_sendtime = millis() +  time;

		if (message_sent) {
			Receive(data,len);
		}
		
		return message_sent;
	}
	return message_sent;
}

/**
 * Send emergency message
 * @param data
 * @param len
 * @return
 */
bool lora::SendEmergency(uint8_t* data, uint8_t &len) {
	uint8_t iteration = 0;
	uint32_t time = 0;
	uint8_t temp = len;
	bool message_sent = false;

  #if SERIAL_DEBUG
	  Serial.print(F("Emergency, DC delay: "));
	  Serial.println(GetDutyWait());
	#endif
  
  delay(GetDutyWait());

	do {
	  if (_sendtime < millis()) {
      #if MAB_UCB_ENABLED || MAB_TS_ENABLED
        LoadNetworkData();

        if (iteration > 0) {
          time = getTimeForBestSF(bwDC, sfDC);
        }

        if (SendMessage(TYPE_EMER_UP, ACK_MAN, data, temp)) {
          time = WaitDutyCycle(GetMessageLength(temp), bwDC, sfDC, crDC, TYPE_EMER_UP);
          message_sent = true;
        } else {
          time = getMaximumTransmissionTime(bwDC, sfDC);
        }
      #else 
        LoadNetworkData(TYPE_EMER_UP, GetMessageLength(temp));
        SendMessage(TYPE_EMER_UP, ACK_MAN, data, temp);
        time = WaitDutyCycle(GetMessageLength(temp), bwDC, sfDC, crDC, TYPE_EMER_UP);
        message_sent = true;
      #endif

			_sendtime = millis() + time;

      #if SERIAL_DEBUG
			  Serial.println(F("Emergency sent and receiving"));
      #endif

			if (message_sent == true && Receive(data,len)) {
        #if MAB_UCB_ENABLED || MAB_TS_ENABLED
				  mab.update(sfDC, 1);
          mab.freqUpdate(freqIdxDC, 1);
        #endif

				return message_sent;
			}

      #if MAB_UCB_ENABLED || MAB_TS_ENABLED
			  mab.update(sfDC, 0);	
        mab.freqUpdate(freqIdxDC, 0);
			#endif

      iteration++;
		} else {
      #if SERIAL_DEBUG
			  Serial.print(F("Emergency DC: "));
			  Serial.println(GetDutyWait());
			#endif
      
      delay(GetDutyWait());
		}
    message_sent = false;
	} while(iteration < 3);
	
	//Emergency messages weren't acknowledged. Starting the registration process again.
  #if MAB_UCB_ENABLED || MAB_TS_ENABLED
	  LoadNetworkData();
  #else
    LoadNetworkData(TYPE_REG_UP, 20);
  #endif
  
  len = 20;
	Register(data, len);
	return message_sent;
}

/**
 * Register the device
 * @param buffer
 * @param len
 * @return
 */
bool lora::Register(uint8_t* buffer, uint8_t &len) {
  SetDefault();		
	bool message_sent = false;
	uint8_t payload[20];
	setDEVID(payload);

	setType(payload, TYPE_REG_UP);
	setType(payload, ACK_MAN);
	
	dhkey1.sendDHA(&payload[4]);
	bool recValue = false;

	#ifdef SERIAL_DEBUG
	  Serial.println(F("Sending register."));
	#endif

  message_sent = send(payload, sizeof(payload));
	waitPacketSent();

	_sendtime = 0;
	_sequence_number = 0;
	int regiterator = 0;

	if (message_sent == true) {
		recValue = Receive(buffer, len);
	}	else {
		recValue = false;
  }

	while (!recValue)
	{
		if (regiterator < 3) {
			message_sent = send(payload, sizeof(payload));
			waitPacketSent();
      
      #if SERIAL_DEBUG
			  Serial.println(F("ITERATOR < 3"));
      #endif 

			if (message_sent == true) {
				recValue = Receive(buffer, len);
        
        #if MAB_UCB_ENABLED || MAB_TS_ENABLED
				  mab.update(sfDC, recValue);
          mab.freqUpdate(freqIdxDC, recValue);
        #endif
			}	else { 
				recValue = false;
      }
		} else {
			// Pseudo-random sequence
			switch (regiterator) {
				case 3: setFrequency(REG_CHANNEL_2); break;
				case 4: setFrequency(REG_CHANNEL_3); break;
				case 5: setFrequency(REG_CHANNEL_4); break;
				case 6: setFrequency(REG_CHANNEL_5); break;
			}

			message_sent = send(payload, sizeof(payload));
			waitPacketSent();
			
      if (message_sent == true) {
				recValue = Receive(buffer, len);

        #if MAB_UCB_ENABLED || MAB_TS_ENABLED
				  mab.update(sfDC, recValue);
          mab.freqUpdate(freqIdxDC, recValue);
        #endif

			}	else {
        recValue = false;
      }
		}

		regiterator++;

    #if MAB_UCB_ENABLED || MAB_TS_ENABLED
		  getTimeForBestSF(bwDC, sfDC);
    #endif
		
    // delay(1000);

		if (regiterator == 7) {
			return false;
		}
	}
	
	return true;
}

/**
 * Receiving the packet
 * @param buf
 * @param len
 * @return
 */
uint8_t lora::Receive(uint8_t* buf, uint8_t &len) {
  spiWrite(RH_RF95_REG_33_INVERT_IQ, spiRead(RH_RF95_REG_33_INVERT_IQ)|(1<<6));
  
  #if SERIAL_DEBUG
    Serial.println(F("Waiting for reply")); 
  #endif
  
  delay(10);

  if (waitAvailableTimeout(RECEIVE_TIMEOUT))
  {
    if (!available()) {
		  len = 0;
		  return 0;
	  }

    ATOMIC_BLOCK_START;
	  if (_bufLen < 4) {
		  len = 0;
		  return 0;
	  }
	
    if (_buf[0] == DEVICE_ID1 && _buf[1] == DEVICE_ID2 && _buf[2] == DEVICE_ID3) {
      uint8_t type = _buf[3] & MASK_TYPE;

      switch (type) {
        case TYPE_REG_DOWN:
          if (ProcessMessage(buf, len, true)) {
            #if SERIAL_DEBUG
              Serial.println(F("Processing REGA"));
            #endif

            #if !(MAB_UCB_ENABLED || MAB_TS_ENABLED)
              ProcessNetworkData(&_buf[21], _buf[20], true);
            #else
              SetDefaultNetworkData();
            #endif
            
            clearRxBuf();
            return 1;
          }
          break;
        case TYPE_DATA_DOWN:
          if (ProcessMessage(buf, len, false)) {

            #if !(MAB_UCB_ENABLED || MAB_TS_ENABLED)
              ProcessNetworkData(&_buf[5], _buf[4], false);
            #endif

            clearRxBuf();
            return 1;
          }
          break;
        default: break;
      }
    }
    ATOMIC_BLOCK_END;

    // This message accepted and cleared
    #if SERIAL_DEBUG
      Serial.println(F("Response received"));
    #endif
    clearRxBuf();
    return 0;
  } else {
	  len = 0;
    #if SERIAL_DEBUG
      Serial.println(F("No reply, is there listener?"));
	  #endif
    return 0;
  }
}

/**
 * Processing the received message
 * @param dataout
 * @param len
 * @param reg
 * @return
 */
bool lora::ProcessMessage(uint8_t* dataout, uint8_t &len, bool reg) {
	uint8_t* payload = &_buf[4];
	
	if (reg) {
		dhkey1.getSessionKey(&_buf[4]);
		payload = &_buf[20];
		Encryption::decrypt(payload, _bufLen - 20, dhkey1.session_private_key);
	} else {
		Encryption::decrypt(payload, _bufLen - 4, dhkey1.session_private_key);
	}
	
	// Data are decrypted now, even it's called encrypted
	uint8_t* decryptedpointer = payload;

	// Get network length
	uint8_t networklen = payload[0];
	payload += networklen + 1; //jump to aplication length
	
	uint8_t applen = payload[0];
	if (applen > len) {
    return false;
  }

	memcpy(dataout, payload+1, applen);
	len = applen;

  // Jump to the sequence number
	payload += applen + 1;
	uint16_t* sequencepointer = (uint16_t*) payload;

	if (!reg) {
		if (!CheckSequence(*sequencepointer)) {
      #if SERIAL_DEBUG
			  Serial.println(F("ERROR: Bad seq"));
			#endif
      return false;
		}
		_sequence_number = *sequencepointer;
	} else {
	  _sequence_number = *sequencepointer;
	}
  #if SERIAL_DEBUG
	  Serial.print(F("Received SEQ: "));
	  Serial.println(_sequence_number);
  #endif

	payload += sizeof(uint16_t);
	uint32_t* micpointer = (uint32_t*) payload;

  // 4 bytes for netlen appLen and seq
	if (Encryption::checkMIC(decryptedpointer, networklen + applen + 4, *micpointer) == false)
	{
    #if SERIAL_DEBUG
		  Serial.println(F("ERROR: Bad MIC"));
		#endif

    return false;
	}
	
  #if SERIAL_DEBUG
	  Serial.println(F("Received MSG is ok!"));
	#endif

  return true;
}

#if MAB_UCB_ENABLED || MAB_TS_ENABLED
  void lora::SetDefaultNetworkData() {
    netconfig global;
    EEPROM.get(0, global);

    global.bw = 2;
    global.cr = 2;
    global.sf = SF9;
    global.pw = 14;
    global.freqSize = 5;

    for (uint8_t idx = 0, freq = 31; idx < global.freqSize; idx++, freq += 2) {
      global.freq[idx] = freq;
    }

    // Begin with random frequency
    freqIdxDC = global.freq[random(0, global.freqSize)];

    EEPROM.put(0, global);
  }
#else
  /**
   * @brief Net Data processing
   * 
   * @param data
   * @param len
   * @param reg
   */
  void lora::ProcessNetworkData(uint8_t* data, uint8_t len, bool reg) {
    netconfig global;
    EEPROM.get(0, global);

    if (reg) {
      // Downlink reg must be full
      if (data[0] != 0) {
        return;
      }

      uint8_t i = 1;
      if (len == 0) {
        return;
      }

      while (i < len) {
        uint8_t j;

        switch ((data[i] & MASK_MSB)) {
          case REC_D_FRQ:
            global.freqDataSize = data[i] & MASK_LSB;
            for (j = 0; j<(data[i] & MASK_LSB); j++) {
              global.freqData[j] = data[i + 1 + j];
            }

            // Jump to BW from freq type and num
            i += j + 1;
            global.bwData = data[i] & MASK_MSB;
            global.bwData = global.bwData >> 4;
            global.crData = data[i] & MASK_LSB;
            i++;
            global.pwData = data[i] & MASK_MSB;
            global.pwData = global.pwData >> 4;
            global.sfData = data[i] & MASK_LSB;
            i++;
            break;
            
          case REC_R_FRQ:
            global.freqRegSize = data[i] & MASK_LSB;
            for (j = 0; j< (data[i] & MASK_LSB); j++) {
              global.freqReg[j] = data[i + 1 + j];
            }

            // Jump to BW from freq type and num
            i += j + 1;
            global.bwReg = data[i] & MASK_MSB;
            global.bwReg = global.bwReg >> 4;
            global.crReg = data[i] & MASK_LSB;
            i++;
            global.pwReg = data[i] & MASK_MSB;
            global.pwReg = global.pwReg >> 4;
            global.sfReg = data[i] & MASK_LSB;
            i++;
            break;
          
          case REC_E_FRQ:
            global.freqEmerSize = data[i] & MASK_LSB;

            for (j = 0; j<(data[i] & MASK_LSB); j++) {
              global.freqEmer[j] = data[i + 1 + j];
            }

            // Jump to BW from freq type and num
            i += j + 1;
            global.bwEmer = data[i] & MASK_MSB;
            global.bwEmer = global.bwEmer >> 4;
            global.crEmer = data[i] & MASK_LSB;
            i++;
            global.pwEmer = data[i] & MASK_MSB;
            global.pwEmer = global.pwEmer >> 4;
            global.sfEmer = data[i] & MASK_LSB;
            i++;
            break;
          }
        }
      } else {
        uint8_t i=0;

        if (data[i] == 0) {
          #if !(MAB_UCB_ENABLED || MAB_TS_ENABLED)
            ProcessNetworkData(data, len, true);
          #endif
          
          return;
        }

        if (len == 0) {
          return;
        }

        while (i < len) {
          uint8_t j;
          switch ((data[i] & MASK_MSB)) {
            case REC_D_FRQ:
              global.freqDataSize = data[i] & MASK_LSB;
              for (j = 0; j< (data[i] & MASK_LSB); j++) {
                global.freqData[j] = data[i + 1 + j];
              }
              i += j + 1; // jump to next
              break;
            case REC_R_FRQ:
              global.freqRegSize = data[i] & MASK_LSB;
              for (j = 0; j< (data[i] & MASK_LSB); j++) {
                global.freqReg[j] = data[i + 1 + j];
              }
              i += j + 1; // jump to next
              break;
            case REC_E_FRQ:
              global.freqEmerSize = data[i] & MASK_LSB;
              for (j = 0; j< (data[i] & MASK_LSB); j++) {
                global.freqEmer[j] = data[i + 1 + j];
              }
              i += j + 1; // jump to next
              break;
            case REC_D_BW:
              global.bwData = data[i] & MASK_LSB;
              i++;
              break;
            case REC_R_BW:
              global.bwReg = data[i] & MASK_LSB;
              i++;
              break;
            case REC_E_BW:
              global.bwEmer = data[i] & MASK_LSB;
              i++;
              break;
            case REC_D_CR:
              global.crData = data[i] & MASK_LSB;
              i++;
              break;
            case REC_R_CR:
              global.crReg = data[i] & MASK_LSB;
              i++;
              break;
            case REC_E_CR:
              global.crEmer = data[i] & MASK_LSB;
              i++;
              break;
            case REC_D_PW:
              global.pwData = data[i] & MASK_LSB;
              i++;
              break;
            case REC_R_PW:
              global.pwReg = data[i] & MASK_LSB;
              i++;
              break;
            case REC_E_PW:
              global.pwEmer = data[i] & MASK_LSB;
              i++;
              break;
            case REC_D_SF:
              global.sfData = data[i] & MASK_LSB;
              i++;
              break;
            case REC_R_SF:
              global.sfReg = data[i] & MASK_LSB;
              i++;
              break;
            case REC_E_SF:
              global.sfEmer = data[i] & MASK_LSB;
              i++;
              break;
          }
        }
      }
    EEPROM.put(0,global);
  }
#endif

#if MAB_UCB_ENABLED || MAB_TS_ENABLED
  uint32_t lora::LoadNetworkData() {
    if (!_manual) {
      netconfig global;
      EEPROM.get(0, global);
      
      uint8_t bw = global.bw;
      uint8_t freq = mab.freqPull(freqIdxDC);
      uint8_t cr = global.cr;
      uint8_t sf = global.sf;
      uint8_t pw = global.pw; 

      bwDC = idxToBW(bw);
      SetBW(bwDC);
      
      float freqdiff = idxToFreq(freq);
      SetFrequency(freqdiff);
      freqDataDC = freqdiff;
      freqIdxDC = freq;

      crDC = cr + MIN_CR;
      SetCR(crDC);

      sfDC = sf + MIN_SF;
      sfDC = pickBestSF(sfDC);

      // 5 is minimum
      if (pw < 5) {
        pw = 5;
      }

      // Not more than maximum TX power
      if (pw > MAX_TX_POWER) {
        pw = MAX_TX_POWER;
      }

      SetPW(pw, false);
      pwDC = pw;
    }

    return 0;
  }
#else
  uint32_t lora::LoadNetworkData(uint8_t type, uint8_t len) {
    if (!_manual) {
      netconfig global;
      EEPROM.get(0, global);
      
      uint8_t bw, freq, cr, sf, pw; 

      if (type == TYPE_DATA_UP || type == TYPE_HELLO_UP) {
        bw = global.bwData;
        freq = global.freqData[random(0,global.freqDataSize)];
        cr = global.crData;
        sf = global.sfData;
        pw = global.pwData;
      } else if (type == TYPE_REG_UP) {
        bw = global.bwReg;
        freq = global.freqReg[random(0,global.freqRegSize)];
        cr = global.crReg;
        sf = global.sfReg;
        pw = global.pwReg;
      } else if (type == TYPE_EMER_UP) {
        bw = global.bwEmer;
        freq = global.freqEmer[random(0,global.freqEmerSize)];
        cr = global.crEmer;
        sf = global.sfEmer;
        pw = global.pwEmer;
      }

      float freqdiff = idxToFreq(freq);
      freqDataDC = freqdiff;
      SetFrequency(freqdiff);

      bwDC = idxToBW(bw);
      percentageDC = getPercentageDC(freqDataDC);
      crDC = cr + MIN_CR;
      SetCR(crDC);
      sfDC = sf + MIN_SF;
      SetSF(sfDC);

      // 5 is minimum
      if (pw < 5) {
        pw = 5;
      }

      // Not more than maximum TX power
      if (pw > MAX_TX_POWER) {
        pw = MAX_TX_POWER;
      }

      SetPW(pw, false);
      pwDC = pw;
    }

    return 0;
  }
#endif

/**
 * Waiting for duty cycle
 * @param len
 * @param bw
 * @param sf
 * @param cr
 * @param type
 * @return
 */
uint32_t lora::WaitDutyCycle(uint8_t len, float bw, uint8_t sf, uint8_t cr, uint8_t type) {
	if (type == TYPE_REG_UP || _manual) {
		return 0;
  }

  // Calculate the symbol time
	float tsymbol = integerPow(2, sf) / (bw * 1000);
	tsymbol *= 1000;
	uint8_t optimalization = sf > 10 ? 1 : 0;
	
	float tpreamble = (8 + 4.25) * tsymbol;
	float payloadSymbNb = 8*len - 4*sf + 28 +16;
	payloadSymbNb /= (4* (sf - 2*optimalization));
	
	float temp = round(payloadSymbNb);
	if (temp < payloadSymbNb) {
		payloadSymbNb = temp + 1;
	} else {
		payloadSymbNb = temp;
	}
	
	float payload = 8 + max(payloadSymbNb*cr, 0);
	payload *= tsymbol;

	uint32_t timeOffAir = 0;
	if (percentageDC == 0) {
		timeOffAir = ((payload + tpreamble) / 0.001 ) - (payload + tpreamble);
	} else if (percentageDC == 1) {
		timeOffAir = ((payload + tpreamble) / 0.01 ) - (payload + tpreamble);
	} else if (percentageDC == 10) {
		timeOffAir = ((payload + tpreamble) / 0.1) - (payload + tpreamble);
	}

  #if SERIAL_DEBUG
	  Serial.print(F("Time of air from waitDC: "));
	  Serial.println(timeOffAir);
  #endif

	return timeOffAir;
}

#if MANUAL_ENABLED
/**
 * Set the manual settings mode
 * @param value
 */
void lora::SetManual(bool value) {
	if (value) {
    #if SERIAL_DEBUG
		  Serial.println(F("MANUALMODE: You are responsible for respecting the DC"));
		#endif
    _sendtime = millis();
	}
	_manual = value;
}
#endif

/**
 * Check message sequence number
 * @param seq
 * @return
 */
bool lora::CheckSequence(uint16_t seq) {
	if (_sequence_number > 65535 - SEQ_DIFF) {
		return (seq > _sequence_number || seq <= _sequence_number + SEQ_DIFF);
	} else {
    if (_sequence_number > seq) {
      return false;
    } else if (seq - _sequence_number <= SEQ_DIFF) {
      return true;
    } else {
      return false;
    }
  }
}
	
/**
 * Set the device ID to the message
 * @param message
 */
void lora::setDEVID(uint8_t* message) {
	message[0] = DEVICE_ID1;
	message[1] = DEVICE_ID2;
	message[2] = DEVICE_ID3;
}

/**
 * Set the type of the message
 * @param message
 * @param type
 */
void lora::setType(uint8_t* message, uint8_t type) {
	message[3] = type;
}

/**
 * Set the ACK of the message
 * @param message
 * @param ack
 */
void lora::setACK(uint8_t* message, uint8_t ack) {
	message[3] |= ack;
}

uint8_t lora::integerPow(uint8_t base, uint8_t exponent) {
  uint8_t result = 1;
  
  while (exponent > 0) {
    if (exponent % 2 == 1) {
      result *= base;
    }
    base *= base;
    exponent /= 2;
  }
  
  return result;
}

uint8_t lora::getPercentageDC(float freqdiff) 
{
  if ((865.0 <= freqdiff && freqdiff <= 868.6) || (869.7 <= freqdiff && freqdiff <= 870.0)) {
    return 1;
  } else if (868.7 <= freqdiff && freqdiff <= 869.2) {
    return 0;
  } else if (869.4 <= freqdiff && freqdiff <= 869.65) {
    return 10;
  }
  return 1;
}

float lora::idxToFreq(uint8_t idx) {
  return (idx / 10.0) + BASE_FREQ;
}

uint8_t lora::freqToIdx(float freq) {
  return (freq - BASE_FREQ) * 10.0;
}

float lora::idxToBW(uint8_t bw) { 
  switch (bw) {
    case 0: return 500.0;
    case 1: return 250.0;
    case 2: return 125.0;
    case 3: return 62.5;
    case 4: return 41.7;
    case 5: return 31.25;
    case 6: return 20.8;
    case 7: return 15.6;
    case 8: return 10.4;
    case 9: return 7.8;
    default: return 125.0;
  }
}

#if MAB_UCB_ENABLED || MAB_TS_ENABLED
/**
 * @brief Network Data processing
 * 
 * @param sf
 */
void lora::writeNetworkData(uint8_t sf) {
	netconfig global;
	EEPROM.get(0, global);

  switch (sf) {
    case 7: global.sf = SF7; break;
    case 8: global.sf = SF8; break;
    case 9: global.sf = SF9; break;
    case 10: global.sf = SF10; break;
    case 11: global.sf = SF11; break;
    case 12: global.sf = SF12; break;
  }

	EEPROM.put(0, global);
}

uint8_t lora::pickBestSF(uint8_t currentSF) {
  uint8_t bestSF = mab.pull(currentSF);
  SetSF(bestSF);
  writeNetworkData(bestSF);
  return bestSF;
}

uint8_t lora::pickBestFREQ(uint8_t currentFREQ) {
  uint8_t bestFREQ = mab.freqPull(currentFREQ);
  SetFrequency(idxToFreq(bestFREQ));
  return bestFREQ;
}

/**
 * @brief Calculate maximum transmission time for best SF
 * 
 * @param currentBW 
 * @param currentSF 
 * @return uint8_t 
 */
uint8_t lora::getTimeForBestSF(float currentBW, uint8_t currentSF) {
  uint8_t bestSF = pickBestSF(currentSF);
  
  if (bestSF == currentSF) {
    return getMaximumTransmissionTime(currentBW, currentSF);
  }
  
  return 0;
}
#endif