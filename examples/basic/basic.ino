#include <WString.h>
#include <Arduino.h>

#define serialNPM (Serial1) //OU SOFT?

unsigned long starttime;
unsigned long starttime_NPM;
unsigned long act_micro;
unsigned long act_milli;
unsigned long last_micro = 0;
unsigned long min_micro = 1000000000;
unsigned long max_micro = 0;

/*****************************************************************
 * Data variables                                      *
 *****************************************************************/
uint32_t npm_pm1_sum = 0;
uint32_t npm_pm10_sum = 0;
uint32_t npm_pm25_sum = 0;
uint32_t npm_pm1_sum_pcs = 0;
uint32_t npm_pm10_sum_pcs = 0;
uint32_t npm_pm25_sum_pcs = 0;
uint16_t npm_val_count = 0;

float last_value_NPM_P0 = -1.0;
float last_value_NPM_P1 = -1.0;
float last_value_NPM_P2 = -1.0;
float last_value_NPM_N1 = -1.0;
float last_value_NPM_N10 = -1.0;
float last_value_NPM_N25 = -1.0;

/*****************************************************************
 * The Setup                                                     *
 *****************************************************************/

void setup()
{
	Serial.begin(115200); 
	Serial.println(F("Starting"));

		serialNPM.begin(115200, SERIAL_8E1, PM_SERIAL_RX, PM_SERIAL_TX);
		Serial.println("Read Next PM... serialNPM 115200 8E1");
		serialNPM.setTimeout(1000); //REVOIR LE TIMEOUT

	Serial.printf("End of void setup()\n");
}

void loop()
{
	String result_NPM;

	unsigned sum_send_time = 0;

	act_micro = micros();
	act_milli = millis();
	send_now = msSince(starttime) > cfg::sending_intervall_ms;

	// Wait at least 30s for each NTP server to sync

		if ((msSince(starttime_NPM) > SAMPLETIME_NPM_MS && npm_val_count == 0))
		{
			starttime_NPM = act_milli;
			fetchSensorNPM(result_NPM);
		}
}
