
#include "MIDIUSB.h"
#include "SoftPWM.h"

#define SERIAL_DEBUG // will print debug messages on serial port when a motor is used @115200b
#define PRINT_ALL_MIDI_MESSAGES // will log all incoming MIDI on serial port
#define FADE_UP 0 // fade up time in ms
#define FADE_DOWN 0 // fade down time in ms
#define MIN_PWM 30 // minimal PWM value that will start the motor (0~255)
#define CC_CH 0xB0// = 176, control change on CH0

#ifdef SERIAL_DEBUG
  #define debugPrint(x)  Serial.print (x)
  #define debugPrintln(x)  Serial.println (x)
#else
  #define debugPrint(x)
  #define debugPrintln(x)
#endif
#ifdef PRINT_ALL_MIDI_MESSAGES
#define SERIAL_DEBUG
#endif

struct motor {
  byte ccNumber;
  unsigned int pin;
};

motor motors[] = { {1, 2}, // CC#1 will control pin 2
                   {2, 3}  // and CC#2 -> pin 3
};

uint8_t motorsCount = sizeof(motors)/sizeof(motor);

void setup() {
  #ifdef SERIAL_DEBUG
  Serial.begin(115200);
  #endif
  SoftPWMBegin();
  for (uint8_t i=0; i<motorsCount; i++) SoftPWMSet(motors[i].pin, 0);
  SoftPWMSetFadeTime(ALL, FADE_UP, FADE_DOWN);
}

void loop() {
  midiEventPacket_t incomingPacket;
  do {
    incomingPacket = MidiUSB.read();
    if (incomingPacket.header != 0) {
      byte type = incomingPacket.byte1;
      byte ccNumber = incomingPacket.byte2;
      byte value = incomingPacket.byte3;
      #ifdef PRINT_ALL_MIDI_MESSAGES
      debugPrint("Received MIDI: header "); debugPrint(incomingPacket.header);
      debugPrint(" type: "); debugPrint(type);
      debugPrint(" number: ");debugPrint(ccNumber);
      debugPrint(" value: "); debugPrintln(value);
      #endif
      if (type ==  CC_CH) {
        for (uint8_t i=0; i<motorsCount; i++) {
          if (motors[i].ccNumber == ccNumber) {
            (value != 0) ? value = map(value, 0, 127, MIN_PWM, 255) : 0;
            debugPrint("setting motor #"); debugPrint(i);
            debugPrint(" (CC#"); debugPrint(ccNumber);
            debugPrint(") to "); debugPrintln(value);
            SoftPWMSet(motors[i].pin, value);
          }
        }
      }
    }
   } while (incomingPacket.header != 0);

}
