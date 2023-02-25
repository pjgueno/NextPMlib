#ifndef __nextpm_H
#define __nextpm_H

#if ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#ifndef ESP32 
#include <SoftwareSerial.h>
#endif

class NextPM {
	public:
		NextPM(void);
#ifndef ESP32 
		void begin(SoftwareSerial* serial);
		void begin(uint8_t pin_rx, uint8_t pin_tx);
#endif
#ifdef ESP32
		void begin(HardwareSerial* serial);
		void begin(HardwareSerial* serial, int8_t pin_rx, int8_t pin_tx);
#endif
		int read(float *p1, float *p25, float *p10);
		void sleep();
		void wakeup();
		void continuous_mode();
	private:
		uint8_t _pin_rx, _pin_tx;
		Stream *npm_data;
};

#endif