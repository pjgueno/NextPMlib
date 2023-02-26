#include <WString.h>
#include <Arduino.h>
#include <nextpm.h>

// #ifdef ESP32
// #define serialNPM (Serial1) //OU SOFT?
// #endif

#ifdef ESP32
HardwareSerial port(1);
#endif

#ifndef ESP32

#endif

//Define the Serial pins explicitely

#define PM_SERIAL_RX 39
#define PM_SERIAL_TX 32

//Define the sensor

NextPM my_nextpm;

NextPM_dataPM data_pm;

NextPM_test data_test;
NextPM_dataTH data_th;

/*****************************************************************
 * The Setup                                                     *
 *****************************************************************/

void setup()
{

	Serial.begin(9600);
	Serial.println(F("Starting"));
	my_nextpm.begin(&port, PM_SERIAL_RX, PM_SERIAL_TX);
	Serial.println("Read Next PM... serialNPM 115200 8E1");
	my_nextpm.powerOnTest(data_test); //
	Serial.println(data_test.connected);
	Serial.println(data_test.sleep);
	Serial.println(data_test.degraded);
	Serial.println(data_test.default_state);
	Serial.println(data_test.notready);
	Serial.println(data_test.heat_error);
	Serial.println(data_test.TH_error);
	Serial.println(data_test.fan_error);
	Serial.println(data_test.memory_error);
	Serial.println(data_test.laser_error);
}

void loop()
{

	my_nextpm.fetchDataPM(data_pm, 10);

	delay(5000);
	my_nextpm.fetchDataTH(data_th);

	delay(5000);
}