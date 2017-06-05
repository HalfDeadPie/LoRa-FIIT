#include "lora.h"
#define DEBUG
//Constructor: Parameters are Slave-select pin, interrupt pin, reset pin
lora::lora(uint8_t slaveSelectPin, uint8_t interruptPin, uint8_t resetPin)
	:
	RH_RF95(slaveSelectPin, interruptPin)
{
	_resetPin = resetPin;
}
//************************************************
//Function: Reset and initialization
 void lora::On(){
  
  pinMode(_resetPin, OUTPUT);
  digitalWrite(_resetPin, HIGH);

  // manual reset
  digitalWrite(_resetPin, LOW);
  delay(10);
  digitalWrite(_resetPin, HIGH);
  delay(10);

  while (!init()) {
    Serial.println("LoRa radio init failed");
    while (1);
  }
  Serial.println("LoRa radio init OK!");
  //if you set manual to true, you are resposible for setting the frequency, coding rate, bandwidth, spreading factor and respecting the duty cycle!
  	_manual = false;
  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
  // you can set transmitter powers from 5 to 23 dBm:
  setTxPower(INIT_TX_POWER, false);	
}
//************************************************
//Function: Setting the default communication parameters
void lora::SetDefault(){
	SetFrequency(REG_CHANNEL_1);
    SetBW(DEFAULT_BW);
    SetSF(DEFAULT_SF);
    SetCR(DEFAULT_CR);
}

void lora::Sleep(){
	spiWrite(RH_RF95_REG_01_OP_MODE, RH_RF95_MODE_SLEEP);
}

void lora::Awake(){
		spiWrite(RH_RF95_REG_01_OP_MODE, RH_RF95_MODE_STDBY);
}


//************************************************
//Function: Set the bandwidth
bool lora::SetBW(float bandwidth) {
#ifdef DEBUG
  Serial.print("Setting BW: ");
  Serial.println(bandwidth);
 #endif
  bool state = false;
  byte value = spiRead(RH_RF95_REG_1D_MODEM_CONFIG1);
  byte bw;
  
	if	    (bandwidth ==  7.80)		bw = BW_7_8;
	else if(bandwidth ==  10.40) 	bw = BW_10_4;
	else if(bandwidth ==  15.60)		bw = BW_15_6; 
	else if(bandwidth ==  20.80)		bw = BW_20_8; 
	else if(bandwidth ==  31.25)		bw = BW_31_25;
	else if(bandwidth ==  41.70)		bw = BW_41_7;
	else if(bandwidth ==  62.50)		bw = BW_62_5;
	else if(bandwidth ==  125.00)	bw = BW_125;
	else if(bandwidth ==  500.00)	bw = BW_500;
	else return false;

  value &= B00001111;
  value |= bw;
  spiWrite(RH_RF95_REG_1D_MODEM_CONFIG1, value);
  value = spiRead(RH_RF95_REG_1D_MODEM_CONFIG1);
  //getting the bandwidth bits only
  value &= B11110000;
  if (value == bw) {
    state = true;
  }
  return state;
}
//************************************************
//Function: Set the coding rate
bool lora::SetCR(uint8_t codingrate) {
#ifdef DEBUG
  Serial.print("Setting CR: ");
  Serial.println(codingrate);
#endif
  bool state = false;
  byte value = spiRead(RH_RF95_REG_1D_MODEM_CONFIG1);
  byte cr;

	switch(codingrate){
		case 5: cr = CR_5; break;
		case 6: cr = CR_6; break;
		case 7: cr = CR_7; break;
		case 8: cr = CR_8; break;
		default: return false;
	}
  
  //clear bits for coding rate
  value &= B11110001;
  //set coding rate bits
  value |= cr;
  spiWrite(RH_RF95_REG_1D_MODEM_CONFIG1, value);
  value = spiRead(RH_RF95_REG_1D_MODEM_CONFIG1);
  //getting only coding rate bits
  value &= B00001110;
  if (value == cr) {
    state = true;
  }
  return state;
}

//Function: Setting the 
bool lora::SetPW(uint8_t power, bool useRF0){
#ifdef DEBUG
	Serial.print("Setting PW: ");
	Serial.println(power);
#endif
	setTxPower(power, useRF0);
}

//Function: Set the spreading factor
bool lora::SetSF(uint8_t spreadingfactor) {
#ifdef DEBUG
  Serial.print("Setting SF: ");
  Serial.println(spreadingfactor);
#endif
  bool state = false;
  byte value = spiRead(RH_RF95_REG_1E_MODEM_CONFIG2);
  byte value_mobile = spiRead(RH_RF95_REG_26_MODEM_CONFIG3);
  byte sf;
  bool mobile;

	switch(spreadingfactor){
		case 6: 	sf = SF_6; 	mobile = false; break;
		case 7:	sf = SF_7; 	mobile = false; break;
		case 8: 	sf = SF_8; 	mobile = false; break;
		case 9: 	sf = SF_9; 	mobile = false; break;
		case 10:	sf = SF_10; mobile = false; break;
		case 11:	sf = SF_11; mobile = true; break;
		case 12:	sf = SF_12; mobile = true; break;
		default: return false;
	}
	
	if (mobile == true) {
		value_mobile &= B11110011;
		value_mobile |= B00001100;
		spiWrite(RH_RF95_REG_26_MODEM_CONFIG3, value_mobile);
	}	 
	else {
		value_mobile &= B11110011;
		value_mobile |= B00000100;
		spiWrite(RH_RF95_REG_26_MODEM_CONFIG3, value_mobile);
	}
	
  value &= B00001111;
  value |= sf;
  spiWrite(RH_RF95_REG_1E_MODEM_CONFIG2, value);
  value = spiRead(RH_RF95_REG_1E_MODEM_CONFIG2);
  value &= B11110000;
  if (value = sf) {
    state = true;
  }
  return state;
}

//Function: Set the frequency
bool lora::SetFrequency(float frequency){
#ifdef DEBUG	
	Serial.print("Setting FREQ: ");
	Serial.println(frequency);
#endif	
	return setFrequency(frequency);
}

//Function: Sendi the common message
void lora::SendMessage(uint8_t type, uint8_t ack, uint8_t* data, uint8_t &len){
	uint8_t payload[RH_RF95_MAX_PAYLOAD_LEN];
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
	*seqpointer = _sequence_number;
	
	datapointer += sizeof(uint16_t);
	uint32_t* micpointer = (uint32_t*) datapointer;
	*micpointer = Encryption::createCheck(&payload[4], len + 3);//applen + payloadlen + seq
	
	uint8_t actuallen = len + 7; //applen + payloadlen + seq + mic
	int8_t paddinglen;
	if(actuallen < 8){
		paddinglen = 8 - actuallen;
	}else{
		paddinglen = actuallen % 4;
		if(paddinglen != 0){
			paddinglen = 4 - paddinglen;
		}
	}

	datapointer += sizeof(uint32_t); 
	for(int i = 0; i< paddinglen; i++){
		*datapointer = random();
		datapointer++;
	}
	
	Encryption::encrypt(&payload[4], actuallen + paddinglen, dhkey1.session_private_key);
	Serial.println("Waiting for packet to complete..."); delay(10);
	send(payload,4 + actuallen + paddinglen);
	waitPacketSent();
}

uint8_t lora::GetMessageLength(uint8_t len){	
	uint8_t actuallen = len + 7;
	int8_t paddinglen;
	if(actuallen < 8){
		paddinglen = 8 - actuallen;
	}else{
		paddinglen = actuallen % 4;
		if(paddinglen != 0){
			paddinglen = 4 - paddinglen;
		}
	}
	return 3 + 1 + actuallen + paddinglen;
	
}

//Function: Returns the time, after which will be device able to send data (duty cycle)
unsigned long lora::GetDutyWait(){
	if(	_sendtime > millis() ){
		return 10 + _sendtime - millis();
	}else{
		return 0;
	}
}

//Function: Automatic send
bool lora::Send(uint8_t* data, uint8_t &len){
	if(_sendtime < millis()){
		Serial.println();
		Serial.println("Sending message...");
		uint32_t time = LoadNetworkData(TYPE_DATA_UP, GetMessageLength(len));
		_sendtime = millis() +  time;
		SendMessage(TYPE_DATA_UP, ACK_OPT, data, len);
		Receive(data,len);
		return true;
	}
	else{
		Serial.print("Duty cycle: ");
		Serial.println(GetDutyWait());
		return false;
	}
}

//Function: Specific send
bool lora::Send(uint8_t type, uint8_t ack, uint8_t* data, uint8_t &len){
		if(_sendtime < millis()){
		uint32_t time = LoadNetworkData(type, GetMessageLength(len));
		_sendtime = millis() + time;
		SendMessage(type,ack,data,len);		
		if(ack == ACK_MAN){
			uint8_t temp = len;
			if(Receive(data,len) == false){
				len = temp;				
				return SendEmergency(data,len);
			}
		}
		else if(ack == ACK_OPT){
			Receive(data,len) ;
		}
		else{
			len = 0;
			return true;
		}
		return true;
	}
	else{
		Serial.print("Duty cycle: ");
		Serial.print(GetDutyWait());
		Serial.println(" -with Mandatory ACK");
		return false;
	}
}

//Function: Hello function
bool lora::SendHello(uint8_t* data, uint8_t &len){
	if(_sendtime < millis()){
		uint32_t time = LoadNetworkData(TYPE_HELLO_UP,len);
		SendMessage(TYPE_HELLO_UP, ACK_OPT, data, len);
		_sendtime = millis() + time;
		Receive(data,len);
		return true;
	}
	else{
		return false;
	}
}

bool lora::SendEmergency(uint8_t* data, uint8_t &len){
	uint8_t iteration = 0;
	uint8_t temp = len;
	Serial.print("Delay for duty cycle before emergency: ");
	Serial.println( GetDutyWait()  );
	delay( GetDutyWait() );
	do{
			if(_sendtime < millis()){
			uint32_t time = LoadNetworkData(TYPE_EMER_UP,GetMessageLength(temp) );
			SendMessage(TYPE_EMER_UP, ACK_MAN, data, temp);
			_sendtime = millis() + time;
			Serial.println("Sending emergency and receiving...");
			if(Receive(data,len)) {
				return true;
			}
			iteration++;
		}else{
			Serial.print("Duty  cycle: ");
			Serial.print(GetDutyWait());
			Serial.println("  -Emergency");
			delay( GetDutyWait() );
		}
	}while(iteration < 3);
	
	//Emergency messages weren't acknoledged. Starting the registration process again.
	LoadNetworkData(TYPE_REG_UP,20);
	len = 20;
	Register(data, len);
	return false;
}

//Function: Register the device
bool lora::Register(uint8_t* buffer, uint8_t &len){
	SetDefault();
	uint8_t payload[20];
	setDEVID(payload);
	setType(payload, TYPE_REG_UP);
	setType(payload, ACK_MAN);
	dhkey1.sendDHA(&payload[4]);
	#ifdef DEBUG
	Serial.println("Sending register..."); delay(10);
	#endif
	send(payload, sizeof(payload));
	waitPacketSent();
	_sendtime = 0;
	_sequence_number = 0;
	int regiterator = 0;
	bool recValue = Receive(buffer, len);
	
	while(!recValue)
	{
		if(regiterator < 3){
			send(payload, sizeof(payload));
			waitPacketSent();
			Serial.println("ITERATOR < 3");
			recValue = Receive(buffer, len);
		}
		else{
			//pseudo-random sequence
			switch(regiterator){
				case 3: setFrequency(REG_CHANNEL_2); break;
				case 4: setFrequency(REG_CHANNEL_3); break;
				case 5: setFrequency(REG_CHANNEL_4); break;
				case 6: setFrequency(REG_CHANNEL_5); break;
			}
			send(payload, sizeof(payload));
			waitPacketSent();
			recValue = Receive(buffer, len);
		}
		regiterator++;
		delay(1000);
		if(regiterator == 7){
			return false;
		}
	}
	return true;
}

//Function: Receiving the packet
bool lora::Receive(uint8_t* buf, uint8_t &len){
  spiWrite(RH_RF95_REG_33_INVERT_IQ, spiRead(RH_RF95_REG_33_INVERT_IQ)|(1<<6));
  Serial.println("Waiting for reply..."); delay(10);
  if (waitAvailableTimeout(RECEIVE_TIMEOUT))
  { 
	if (!available()){
		len = 0;
		return false;
	}
	ATOMIC_BLOCK_START;
	if(_bufLen < 4){
		len = 0;
		return false;
	}
	
	if(_buf[0] == DEVICE_ID1
	&& _buf[1] == DEVICE_ID2
	&& _buf[2] == DEVICE_ID3){

		uint8_t type = _buf[3] & MASK_TYPE;
		switch(type){
			case TYPE_REG_DOWN:
				if(ProcessMessage(buf, len, true)){
					Serial.println("Processing the register down message...");
					ProcessNetworkData(&_buf[21], _buf[20], true);
					clearRxBuf();
					return true;
				}
				break;
			case TYPE_DATA_DOWN:
				if(ProcessMessage(buf, len, false)){
					ProcessNetworkData(&_buf[5], _buf[4], false);
					clearRxBuf();
					return true;
				}
				break;
			default: break;
		}
	}
	ATOMIC_BLOCK_END;
    clearRxBuf(); // This message accepted and cleared
    return false;
  }
  else
  {
	len = 0;
    Serial.println("No reply, is there a listener around?");
	return false;
  }
}

//Function: Processing the received message
bool lora::ProcessMessage(uint8_t* dataout, uint8_t &len, bool reg){
	uint8_t* payload = &_buf[4];
	
	if(reg){
		dhkey1.getSessionKey(&_buf[4]);
		payload = &_buf[20];
		Encryption::decrypt(payload, _bufLen - 20, dhkey1.session_private_key);
	}
	else{
		Encryption::decrypt(payload, _bufLen - 4, dhkey1.session_private_key);
	}
	
	//data are decrypted now, even it's called encrypted
	uint8_t* decryptedpointer = payload;
	//get network length
	uint8_t networklen = payload[0];
	payload += networklen + 1; //jump to aplication length
	
	uint8_t applen = payload[0];
	if(applen > len)
		return false;
	memcpy(dataout, payload+1, applen);
	len = applen;
	
	payload += applen + 1;//jump to sequence number
	uint16_t* sequencepointer = (uint16_t*) payload;
	if(!reg){
		if(!CheckSequence(*sequencepointer)){
			Serial.println("**ERROR: Bad seq");
			return false;
		}
		else{
			_sequence_number = *sequencepointer;
		}
	}else{
		_sequence_number = *sequencepointer;
	}
	payload += sizeof(uint16_t);
	
	uint32_t* micpointer = (uint32_t*) payload;
	if(Encryption::isLoraPacketCorrect(decryptedpointer, networklen + applen + 4, *micpointer) == false) // 4 bytes for netlen appLen and seq
	{
		Serial.println("**ERROR: Bad MIC");
		return false;
	}
	
	Serial.println("Received message is ok!");
	return true;
}



void lora::ProcessNetworkData(uint8_t* data, uint8_t len, bool reg){
	netconfig global;
	EEPROM.get(0,global);

	if(reg){		
		//downlink reg must be full

		if(data[0] != 0)
			return;

		uint8_t i = 1;
		if(len == 0)
			return;
		while(i < len){
			uint8_t j;
			
			switch((data[i] & MASK_MSB)){
				case REC_D_FRQ:
					global.freqDataSize = data[i] & MASK_LSB;
					for(j = 0; j<(data[i] & MASK_LSB); j++){
						global.freqData[j] = data[i + 1 + j];
					}
					i += j + 1; // jump to BW from freq type and num
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
					for(j = 0; j< (data[i] & MASK_LSB); j++){
						global.freqReg[j] = data[i + 1 + j];
					}
					i += j + 1; // jump to BW from freq type and num
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
					for(j = 0; j<(data[i] & MASK_LSB); j++){
						global.freqEmer[j] = data[i + 1 + j];
					}
					i += j + 1; // jump to BW from freq type and num
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
		
		}
		else{
		uint8_t i=0;
		if(data[i] == 0){
			ProcessNetworkData(data,len,true);
			return;
		}
		if(len == 0)
			return;
		while(i < len){
			uint8_t j;
			switch((data[i] & MASK_MSB)){
				case REC_D_FRQ:
					global.freqDataSize = data[i] & MASK_LSB;
					for(j = 0; j< (data[i] & MASK_LSB); j++){
						global.freqData[j] = data[i + 1 + j];
					}
					i += j + 1; // jump to next
					break;
					
				case REC_R_FRQ:
					global.freqRegSize = data[i] & MASK_LSB;
					for(j = 0; j< (data[i] & MASK_LSB); j++){
						global.freqReg[j] = data[i + 1 + j];
					}
					i += j + 1; // jump to next		
					break;
			
				case REC_E_FRQ:
					global.freqEmerSize = data[i] & MASK_LSB;
					for(j = 0; j< (data[i] & MASK_LSB); j++){
						global.freqEmer[j] = data[i + 1 + j];
					}
					i += j + 1; // jump to next
					break;
				
				//***********
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
				//***********
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
				//***********
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
				//***********
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

uint32_t lora::LoadNetworkData(uint8_t type, uint8_t len){
	if(_manual == false){
		netconfig global;
		uint8_t percentage;
		EEPROM.get(0,global);
		
		uint8_t bw;
		uint8_t freq;
		uint8_t cr;
		uint8_t sf;
		uint8_t pw; 
		
		float bwDC;
		float freqDC;
		uint8_t crDC;
		uint8_t sfDC;
			
		if(type == TYPE_DATA_UP || type == TYPE_HELLO_UP){
			bw = global.bwData;
			freq = global.freqData[random(0,global.freqDataSize)];
			cr = global.crData;
			sf = global.sfData;
			pw = global.pwData;
		}else if(type == TYPE_REG_UP){
			bw = global.bwReg;
			freq = global.freqReg[random(0,global.freqRegSize)];
			cr = global.crReg;
			sf = global.sfReg;
			pw = global.pwReg;
		}else if(type == TYPE_EMER_UP){
			bw = global.bwEmer;
			freq = global.freqEmer[random(0,global.freqEmerSize)];
			cr = global.crEmer;
			sf = global.sfEmer;
			pw = global.pwEmer;
		}
		switch(bw){
			case 0: SetBW(500.0); bwDC = 500.0; break;
			case 1: SetBW(250.0); bwDC = 250.0; break;
			case 2: SetBW(125.0); bwDC = 125.0; break;
			case 3: SetBW(62.5); bwDC = 62.5; break;
			case 4: SetBW(41.7); bwDC = 41.7; break;
			case 5: SetBW(31.25); bwDC = 31.25;  break;
			case 6: SetBW(20.8); bwDC = 20.8; break;
			case 7: SetBW(15.6); bwDC = 15.6; break;
			case 8: SetBW(10.4); bwDC = 10.4; break;
			case 9: SetBW(7.8); bwDC = 7.8; break;
			default: SetBW(125.0); bwDC = 125.0; break;
		}
		
		float freqdiff = freq / 10.0;
		freqdiff += 863;
		SetFrequency(freqdiff);
		if( (865.0 <= freqdiff && freqdiff <= 868.6) || (869.7 <= freqdiff && freqdiff <= 870.0) ){
			percentage = 1;
		}
		else if(868.7 <= freqdiff && freqdiff <= 869.2){
			percentage = 0;
		}
		else if(869.4 <= freqdiff && freqdiff <= 869.65){
			percentage = 10;
		}
		
		switch(cr){
			case 0: SetCR(5); crDC = 5; break;
			case 1: SetCR(6); crDC = 6; break;
			case 2: SetCR(7); crDC = 7;	break;
			case 3: SetCR(8); crDC = 8; break;
			default: SetCR(5); crDC = 5; break;
		}
		
		switch(sf){
			case 0: SetSF(7); sfDC = 7; break;
			case 1: SetSF(8); sfDC = 8; break;
			case 2: SetSF(9); sfDC = 9; break;
			case 3: SetSF(10); sfDC = 10; break;
			case 4: SetSF(11); sfDC = 11; break;
			case 5: SetSF(12); sfDC = 12; break;
			default: SetSF(7); sfDC = 7; break;
		}

		// 5 is minimum
		if(pw < 5){
			pw = 5;
		}
		if(pw > MAX_TX_POWER){
			pw = MAX_TX_POWER;
		}
		SetPW(pw,false);
		
		if(type != TYPE_REG_UP)
			return WaitDutyCycle(len, bwDC, sfDC, crDC, percentage);
		else
			return 0;
	}else{
		return 0;
	}
}

uint32_t lora::WaitDutyCycle(uint8_t len, float bw, uint8_t sf, uint8_t cr, uint8_t percentage) {
	float tsymbol = pow(2,sf) / (bw * 1000);
	tsymbol *= 1000;
	uint8_t optimalization;
	
	if(sf > 10)
	{
		optimalization = 1;
	}
	else{
		optimalization = 0;
	}
	
	float tpreamble = (8 + 4.25) * tsymbol;
	float payloadSymbNb = 8*len - 4*sf + 28 +16;
	payloadSymbNb /= (4* (sf - 2*optimalization));
	
	float temp = round(payloadSymbNb);
	if(temp < payloadSymbNb)
	{
		payloadSymbNb = temp + 1;
	}
	else
	{
		payloadSymbNb = temp;
	}
	
	float payload = 8 + max(payloadSymbNb*cr, 0);
	payload *= tsymbol;
	if(percentage == 0){
		return (payload + tpreamble) / 0.001;
	}
	else if(percentage == 1){
		return (payload + tpreamble) / 0.01;
	}else if(percentage == 10){
		return (payload + tpreamble) / 0.1;
	}
}

//Function: Set the manual settings mode
void lora::SetManual(bool value){
	if(value == true){
		Serial.println("**MANUALMODE: You are responsible for respecting the duty cycle**");
		_sendtime = millis();
	}
	_manual = value;
}

bool lora::CheckSequence(uint16_t seq){
	if(_sequence_number > 65535 - SEQ_DIFF){
		if(seq > _sequence_number || seq <= _sequence_number + SEQ_DIFF){
			return true;
		}else {
			return false;
		}
	}
	else{
		if(_sequence_number > seq)
			return false;
		else if(seq - _sequence_number <= SEQ_DIFF)
			return true;
		else
			return false;
		}
}
	
//Function: Set the device ID to the message
void lora::setDEVID(uint8_t* message){
	message[0] = DEVICE_ID1;
	message[1] = DEVICE_ID2;
	message[2] = DEVICE_ID3;
}

//Function: Set the type of the message
void lora::setType(uint8_t* message, uint8_t type){
	message[3] = type;
}

//Function: Set the ACK of the message
void lora::setACK(uint8_t* message, uint8_t ack){
	message[3] |= ack;
}