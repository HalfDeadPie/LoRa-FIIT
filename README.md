# LoRa@FIIT Arduino Library

This is an Arduino library for LoRa@FIIT communication using the LoRa Radio Node 1.0 at 868 MHz.

## Installation

To use this library, you'll need PlatformIO IDE installed. Add the following line to your `platformio.ini` dependencies:

```ini
lib_deps =
   LoRa@FIIT
```

## Usage

### Initializing LoRa

```cpp
#include <lora.h>

// Define your pins
#define RFM95_CS 10
#define RFM95_INT 2
#define RFM95_RST 9

lora LoRa(RFM95_CS, RFM95_INT, RFM95_RST);

void setup() {
  LoRa.On();       // Reset and initialize the chip
  LoRa.SetDefault(); // Set default settings
}

void loop() {
  // Your LoRa communication code here
}
```

## API Reference

### Class `lora`

#### Constructor

```cpp
lora(uint8_t slaveSelectPin, uint8_t interruptPin, uint8_t resetPin);
```

Initialize LoRa module with the specified pins.

#### Methods

- `void On()`: Reset and initialize the chip.
- `void SetDefault()`: Set default LoRa settings.
- `bool SetBW(float bandwidth)`: Set the bandwidth.
- `bool SetCR(uint8_t codingRate)`: Set the coding rate.
- `bool SetSF(uint8_t spreadingFactor)`: Set the spreading factor.
- `bool SetFrequency(float frequency)`: Set the operating frequency.
- `bool SetPW(uint8_t power, bool useRF0)`: Set the transceiver power.
- `void Awake()`: Turn on the LoRa module.
- `void Sleep()`: Put the LoRa module to sleep.
- `bool Send(uint8_t* data, uint8_t &len)`: Send basic data message with ACK.
- `bool Send(uint8_t type, uint8_t ack, uint8_t* data, uint8_t &len)`: Send a specific message.
- `bool SendHello(uint8_t* data, uint8_t &len)`: Send a "Hello" message.
- `bool SendEmergency(uint8_t* data, uint8_t &len)`: Send an emergency message.
- `unsigned long Getsendtime()`: Get the time when the message was sent.
- `uint8_t Receive(uint8_t* buf, uint8_t &len)`: Turn on receiving mode.
- `bool Register(uint8_t* buffer, uint8_t &len)`: Register the device.
- `unsigned long GetDutyWait()`: Get the wait time due to duty cycle.
- `uint32_t WaitDutyCycle(uint8_t len, float bw, uint8_t sf, uint8_t cr, uint8_t type)`: Handle duty cycle.

#### Private Members (selected)

- `bool _manual`: Flag for manual mode.
- `uint8_t currentSF`: Current spreading factor.
- `float bwDC`: Current bandwidth.
- `uint8_t percentageDC`: Current duty cycle percentage.
- `uint8_t crDC`: Current coding rate.
- `uint8_t sfDC`: Current spreading factor.
- `uint8_t pwDC`: Current power.
- `DH dhkey1`: Diffie-Hellman key instance.
- `uint16_t _sequence_number`: Current sequence number.
- `unsigned long _sendtime`: Time when the message was sent.

## License

See the [LICENSE](LICENSE) file for details.

## Credits

- **Author**: Simon Štefunko, Alexander Valach, Michal Greguš
  

