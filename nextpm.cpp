#include <WString.h>
#include "nextpm.h"


enum
{
	NPM_REPLY_HEADER_16 = 16,
	NPM_REPLY_STATE_16 = 14,
	NPM_REPLY_BODY_16 = 13,
	NPM_REPLY_CHECKSUM_16 = 1
} NPM_waiting_for_16; // for concentration

enum
{
	NPM_REPLY_HEADER_4 = 4,
	NPM_REPLY_STATE_4 = 2,
	NPM_REPLY_CHECKSUM_4 = 1
} NPM_waiting_for_4; // for change

enum
{
	NPM_REPLY_HEADER_5 = 5,
	NPM_REPLY_STATE_5 = 3,
	NPM_REPLY_DATA_5 = 2,
	NPM_REPLY_CHECKSUM_5 = 1
} NPM_waiting_for_5; // for fan speed

enum
{
	NPM_REPLY_HEADER_6 = 6,
	NPM_REPLY_STATE_6 = 4,
	NPM_REPLY_DATA_6 = 3,
	NPM_REPLY_CHECKSUM_6 = 1
} NPM_waiting_for_6; // for version

enum
{
	NPM_REPLY_HEADER_8 = 8,
	NPM_REPLY_STATE_8 = 6,
	NPM_REPLY_BODY_8 = 5,
	NPM_REPLY_CHECKSUM_8 = 1
} NPM_waiting_for_8; // for temperature/humidity

String current_state_npm;
String current_th_npm;


/*****************************************************************
 * NPM functions     *
 *****************************************************************/

static int8_t NPM_get_state()
{
	int8_t result = -1;
	NPM_waiting_for_4 = NPM_REPLY_HEADER_4;
	debug_outln_info(F("State NPM..."));
	NPM_cmd(PmSensorCmd::State);

	unsigned long timeout = millis();

	do
	{
		debug_outln("Wait for Serial...", DEBUG_MAX_INFO);
	} while (!serialNPM.available() && millis() - timeout < 3000);

	while (!serialNPM.available())
	{
		debug_outln("Wait for Serial...", DEBUG_MAX_INFO);
	}

	while (serialNPM.available() >= NPM_waiting_for_4)
	{
		const uint8_t constexpr header[2] = {0x81, 0x16};
		uint8_t state[1];
		uint8_t checksum[1];
		uint8_t test[4];

		switch (NPM_waiting_for_4)
		{
		case NPM_REPLY_HEADER_4:
			if (serialNPM.find(header, sizeof(header)))
				NPM_waiting_for_4 = NPM_REPLY_STATE_4;
			break;
		case NPM_REPLY_STATE_4:
			serialNPM.readBytes(state, sizeof(state));
			NPM_state(state[0]);
			result = state[0];
			NPM_waiting_for_4 = NPM_REPLY_CHECKSUM_4;
			break;
		case NPM_REPLY_CHECKSUM_4:
			serialNPM.readBytes(checksum, sizeof(checksum));
			memcpy(test, header, sizeof(header));
			memcpy(&test[sizeof(header)], state, sizeof(state));
			memcpy(&test[sizeof(header) + sizeof(state)], checksum, sizeof(checksum));
			NPM_data_reader(test, 4);
			NPM_waiting_for_4 = NPM_REPLY_HEADER_4;
			if (NPM_checksum_valid_4(test))
			{
				debug_outln_info(F("Checksum OK..."));
			}
			break;
		}
	}
	return result;
}

static bool NPM_start_stop()
{
	bool result;
	NPM_waiting_for_4 = NPM_REPLY_HEADER_4;
	debug_outln_info(F("Switch start/stop NPM..."));
	NPM_cmd(PmSensorCmd::Change);

	unsigned long timeout = millis();

	do
	{
		debug_outln("Wait for Serial...", DEBUG_MAX_INFO);
	} while (!serialNPM.available() && millis() - timeout < 3000);

	while (serialNPM.available() >= NPM_waiting_for_4)
	{
		const uint8_t constexpr header[2] = {0x81, 0x15};
		uint8_t state[1];
		uint8_t checksum[1];
		uint8_t test[4];

		switch (NPM_waiting_for_4)
		{
		case NPM_REPLY_HEADER_4:
			if (serialNPM.find(header, sizeof(header)))
				NPM_waiting_for_4 = NPM_REPLY_STATE_4;
			break;
		case NPM_REPLY_STATE_4:
			serialNPM.readBytes(state, sizeof(state));
			NPM_state(state[0]);

			if (bitRead(state[0], 0) == 0)
			{
				debug_outln_info(F("NPM start..."));
				result = true;
			}
			else if (bitRead(state[0], 0) == 1)
			{
				debug_outln_info(F("NPM stop..."));
				result = false;
			}
			else
			{
				result = !is_NPM_running; //DANGER BECAUSE NON INITIALISED
			}

			NPM_waiting_for_4 = NPM_REPLY_CHECKSUM_4;
			break;
		case NPM_REPLY_CHECKSUM_4:
			serialNPM.readBytes(checksum, sizeof(checksum));
			memcpy(test, header, sizeof(header));
			memcpy(&test[sizeof(header)], state, sizeof(state));
			memcpy(&test[sizeof(header) + sizeof(state)], checksum, sizeof(checksum));
			NPM_data_reader(test, 4);
			NPM_waiting_for_4 = NPM_REPLY_HEADER_4;
			if (NPM_checksum_valid_4(test))
			{
				debug_outln_info(F("Checksum OK..."));
			}
			break;
		}
	}
	return result;
}

static String NPM_version_date()
{
	// debug_outln_verbose(FPSTR(DBG_TXT_START_READING), FPSTR(DBG_TXT_NPM_VERSION_DATE));
	delay(250);
	NPM_waiting_for_6 = NPM_REPLY_HEADER_6;
	debug_outln_info(F("Version NPM..."));
	NPM_cmd(PmSensorCmd::Version);

	unsigned long timeout = millis();

	do
	{
		debug_outln("Wait for Serial...", DEBUG_MAX_INFO);
	} while (!serialNPM.available() && millis() - timeout < 3000);

	while (serialNPM.available() >= NPM_waiting_for_6)
	{
		const uint8_t constexpr header[2] = {0x81, 0x17};
		uint8_t state[1];
		uint8_t data[2];
		uint8_t checksum[1];
		uint8_t test[6];

		switch (NPM_waiting_for_6)
		{
		case NPM_REPLY_HEADER_6:
			if (serialNPM.find(header, sizeof(header)))
				NPM_waiting_for_6 = NPM_REPLY_STATE_6;
			break;
		case NPM_REPLY_STATE_6:
			serialNPM.readBytes(state, sizeof(state));
			NPM_state(state[0]);
			NPM_waiting_for_6 = NPM_REPLY_DATA_6;
			break;
		case NPM_REPLY_DATA_6:
			if (serialNPM.readBytes(data, sizeof(data)) == sizeof(data))
			{
				NPM_data_reader(data, 2);
				uint16_t NPMversion = word(data[0], data[1]);
				last_value_NPM_version = String(NPMversion);
				// debug_outln_verbose(FPSTR(DBG_TXT_END_READING), FPSTR(DBG_TXT_NPM_VERSION_DATE));
				debug_outln_info(F("Next PM Firmware: "), last_value_NPM_version);
			}
			NPM_waiting_for_6 = NPM_REPLY_CHECKSUM_6;
			break;
		case NPM_REPLY_CHECKSUM_6:
			serialNPM.readBytes(checksum, sizeof(checksum));
			memcpy(test, header, sizeof(header));
			memcpy(&test[sizeof(header)], state, sizeof(state));
			memcpy(&test[sizeof(header) + sizeof(state)], data, sizeof(data));
			memcpy(&test[sizeof(header) + sizeof(state) + sizeof(data)], checksum, sizeof(checksum));
			NPM_data_reader(test, 6);
			NPM_waiting_for_6 = NPM_REPLY_HEADER_6;
			if (NPM_checksum_valid_6(test))
			{
				debug_outln_info(F("Checksum OK..."));
			}
			break;
		}
	}
	return last_value_NPM_version;
}

static void NPM_fan_speed()
{

	NPM_waiting_for_5 = NPM_REPLY_HEADER_5;
	debug_outln_info(F("Set fan speed to 50 %..."));
	NPM_cmd(PmSensorCmd::Speed);

	unsigned long timeout = millis();

	do
	{
		debug_outln("Wait for Serial...", DEBUG_MAX_INFO);
	} while (!serialNPM.available() && millis() - timeout < 3000);

	while (serialNPM.available() >= NPM_waiting_for_5)
	{
		const uint8_t constexpr header[2] = {0x81, 0x21};
		uint8_t state[1];
		uint8_t data[1];
		uint8_t checksum[1];
		uint8_t test[5];

		switch (NPM_waiting_for_5)
		{
		case NPM_REPLY_HEADER_5:
			if (serialNPM.find(header, sizeof(header)))
				NPM_waiting_for_5 = NPM_REPLY_STATE_5;
			break;
		case NPM_REPLY_STATE_5:
			serialNPM.readBytes(state, sizeof(state));
			NPM_state(state[0]);
			NPM_waiting_for_5 = NPM_REPLY_DATA_5;
			break;
		case NPM_REPLY_DATA_5:
			if (serialNPM.readBytes(data, sizeof(data)) == sizeof(data))
			{
				NPM_data_reader(data, 1);
			}
			NPM_waiting_for_5 = NPM_REPLY_CHECKSUM_5;
			break;
		case NPM_REPLY_CHECKSUM_5:
			serialNPM.readBytes(checksum, sizeof(checksum));
			memcpy(test, header, sizeof(header));
			memcpy(&test[sizeof(header)], state, sizeof(state));
			memcpy(&test[sizeof(header) + sizeof(state)], data, sizeof(data));
			memcpy(&test[sizeof(header) + sizeof(state) + sizeof(data)], checksum, sizeof(checksum));
			NPM_data_reader(test, 5);
			NPM_waiting_for_5 = NPM_REPLY_HEADER_5;
			if (NPM_checksum_valid_5(test))
			{
				debug_outln_info(F("Checksum OK..."));
			}
			break;
		}
	}
}

static String NPM_temp_humi()
{
	uint16_t NPM_temp;
	uint16_t NPM_humi;
	NPM_waiting_for_8 = NPM_REPLY_HEADER_8;
	debug_outln_info(F("Temperature/Humidity in Next PM..."));
	NPM_cmd(PmSensorCmd::Temphumi);

	unsigned long timeout = millis();

	do
	{
		debug_outln("Wait for Serial...", DEBUG_MAX_INFO);
	} while (!serialNPM.available() && millis() - timeout < 3000);

	while (serialNPM.available() >= NPM_waiting_for_8)
	{
		const uint8_t constexpr header[2] = {0x81, 0x14};
		uint8_t state[1];
		uint8_t data[4];
		uint8_t checksum[1];
		uint8_t test[8];

		switch (NPM_waiting_for_8)
		{
		case NPM_REPLY_HEADER_8:
			if (serialNPM.find(header, sizeof(header)))
				NPM_waiting_for_8 = NPM_REPLY_STATE_8;
			break;
		case NPM_REPLY_STATE_8:
			serialNPM.readBytes(state, sizeof(state));
			NPM_state(state[0]);
			NPM_waiting_for_8 = NPM_REPLY_BODY_8;
			break;
		case NPM_REPLY_BODY_8:
			if (serialNPM.readBytes(data, sizeof(data)) == sizeof(data))
			{
				NPM_data_reader(data, 4);
				NPM_temp = word(data[0], data[1]);
				NPM_humi = word(data[2], data[3]);
				debug_outln_verbose(F("Temperature (°C): "), String(NPM_temp / 100.0f));
				debug_outln_verbose(F("Relative humidity (%): "), String(NPM_humi / 100.0f));
			}
			NPM_waiting_for_8 = NPM_REPLY_CHECKSUM_8;
			break;
		case NPM_REPLY_CHECKSUM_16:
			serialNPM.readBytes(checksum, sizeof(checksum));
			memcpy(test, header, sizeof(header));
			memcpy(&test[sizeof(header)], state, sizeof(state));
			memcpy(&test[sizeof(header) + sizeof(state)], data, sizeof(data));
			memcpy(&test[sizeof(header) + sizeof(state) + sizeof(data)], checksum, sizeof(checksum));
			NPM_data_reader(test, 8);
			if (NPM_checksum_valid_8(test))
				debug_outln_info(F("Checksum OK..."));
			NPM_waiting_for_8 = NPM_REPLY_HEADER_8;
			break;
		}
	}
	return String(NPM_temp / 100.0f) + " / " + String(NPM_humi / 100.0f);
}


/*****************************************************************
 * read Tera Sensor Next PM sensor sensor values                 *
 *****************************************************************/
static void fetchSensorNPM(String &s)
{

	NPM_waiting_for_16 = NPM_REPLY_HEADER_16;

	debug_outln_info(F("Concentration NPM..."));
	NPM_cmd(PmSensorCmd::Concentration);

	unsigned long timeout = millis();

	do
	{
		debug_outln("Wait for Serial...", DEBUG_MAX_INFO);
	} while (!serialNPM.available() && millis() - timeout < 3000);

	while (serialNPM.available() >= NPM_waiting_for_16)
	{
		const uint8_t constexpr header[2] = {0x81, 0x12};
		uint8_t state[1];
		uint8_t data[12];
		uint8_t checksum[1];
		uint8_t test[16];
		uint16_t N1_serial;
		uint16_t N25_serial;
		uint16_t N10_serial;
		uint16_t pm1_serial;
		uint16_t pm25_serial;
		uint16_t pm10_serial;

		switch (NPM_waiting_for_16)
		{
		case NPM_REPLY_HEADER_16:
			if (serialNPM.find(header, sizeof(header)))
				NPM_waiting_for_16 = NPM_REPLY_STATE_16;
			break;
		case NPM_REPLY_STATE_16:
			serialNPM.readBytes(state, sizeof(state));
			current_state_npm = NPM_state(state[0]);
			NPM_waiting_for_16 = NPM_REPLY_BODY_16;
			break;
		case NPM_REPLY_BODY_16:
			if (serialNPM.readBytes(data, sizeof(data)) == sizeof(data))
			{
				NPM_data_reader(data, 12);
				N1_serial = word(data[0], data[1]);
				N25_serial = word(data[2], data[3]);
				N10_serial = word(data[4], data[5]);

				pm1_serial = word(data[6], data[7]);
				pm25_serial = word(data[8], data[9]);
				pm10_serial = word(data[10], data[11]);

				debug_outln_info(F("Next PM Measure..."));

				debug_outln_verbose(F("PM1 (μg/m3) : "), String(pm1_serial / 10.0f));
				debug_outln_verbose(F("PM2.5 (μg/m3): "), String(pm25_serial / 10.0f));
				debug_outln_verbose(F("PM10 (μg/m3) : "), String(pm10_serial / 10.0f));

				debug_outln_verbose(F("PM1 (pcs/L) : "), String(N1_serial));
				debug_outln_verbose(F("PM2.5 (pcs/L): "), String(N25_serial));
				debug_outln_verbose(F("PM10 (pcs/L) : "), String(N10_serial));
			}
			NPM_waiting_for_16 = NPM_REPLY_CHECKSUM_16;
			break;
		case NPM_REPLY_CHECKSUM_16:
			serialNPM.readBytes(checksum, sizeof(checksum));
			memcpy(test, header, sizeof(header));
			memcpy(&test[sizeof(header)], state, sizeof(state));
			memcpy(&test[sizeof(header) + sizeof(state)], data, sizeof(data));
			memcpy(&test[sizeof(header) + sizeof(state) + sizeof(data)], checksum, sizeof(checksum));
			NPM_data_reader(test, 16);
			if (NPM_checksum_valid_16(test))
			{
				debug_outln_info(F("Checksum OK..."));

				npm_pm1_sum += pm1_serial;
				npm_pm25_sum += pm25_serial;
				npm_pm10_sum += pm10_serial;

				npm_pm1_sum_pcs += N1_serial;
				npm_pm25_sum_pcs += N25_serial;
				npm_pm10_sum_pcs += N10_serial;
				npm_val_count++;
				debug_outln(String(npm_val_count), DEBUG_MAX_INFO);
			}
			NPM_waiting_for_16 = NPM_REPLY_HEADER_16;
			break;
		}
	}

	if (send_now && cfg::sending_intervall_ms >= 120000)
	{
		last_value_NPM_P0 = -1.0f;
		last_value_NPM_P1 = -1.0f;
		last_value_NPM_P2 = -1.0f;
		last_value_NPM_N1 = -1.0f;
		last_value_NPM_N10 = -1.0f;
		last_value_NPM_N25 = -1.0f;

		if (npm_val_count == 2)
		{
			last_value_NPM_P0 = float(npm_pm1_sum) / (npm_val_count * 10.0f);
			last_value_NPM_P1 = float(npm_pm10_sum) / (npm_val_count * 10.0f);
			last_value_NPM_P2 = float(npm_pm25_sum) / (npm_val_count * 10.0f);

			last_value_NPM_N1 = float(npm_pm1_sum_pcs) / (npm_val_count); //enlevé * 1000.0f pour Litre
			last_value_NPM_N10 = float(npm_pm10_sum_pcs) / (npm_val_count);
			last_value_NPM_N25 = float(npm_pm25_sum_pcs) / (npm_val_count);

			add_Value2Json(s, F("NPM_P0"), F("PM1: "), last_value_NPM_P0);
			add_Value2Json(s, F("NPM_P1"), F("PM10:  "), last_value_NPM_P1);
			add_Value2Json(s, F("NPM_P2"), F("PM2.5: "), last_value_NPM_P2);

			add_Value2Json(s, F("NPM_N1"), F("NC1.0: "), last_value_NPM_N1);
			add_Value2Json(s, F("NPM_N10"), F("NC10:  "), last_value_NPM_N10);
			add_Value2Json(s, F("NPM_N25"), F("NC2.5: "), last_value_NPM_N25);

			debug_outln_info(FPSTR(DBG_TXT_SEP));
		}
		else
		{
			NPM_error_count++;
		}

		npm_pm1_sum = 0;
		npm_pm10_sum = 0;
		npm_pm25_sum = 0;

		npm_val_count = 0;

		npm_pm1_sum_pcs = 0;
		npm_pm10_sum_pcs = 0;
		npm_pm25_sum_pcs = 0;

		debug_outln_info(F("Temperature and humidity in NPM after measure..."));
		current_th_npm = NPM_temp_humi();
	}
}

static void powerOnTestNPM()
{
		int8_t test_state;
		delay(15000); // wait a bit to be sure Next PM is ready to receive instructions.
		test_state = NPM_get_state();
		if (test_state == -1)
		{
			debug_outln_info(F("NPM not connected"));
			nextpmconnected = false;
		}
		else
		{
			nextpmconnected = true;
			if (test_state == 0x00)
			{
				debug_outln_info(F("NPM already started..."));
				nextpmconnected = true;
			}
			else if (test_state == 0x01)
			{
				debug_outln_info(F("Force start NPM...")); // to read the firmware version
				is_NPM_running = NPM_start_stop();
			}
			else
			{
				if (bitRead(test_state, 1) == 1)
				{
					debug_outln_info(F("Degraded state"));
				}
				else
				{
					debug_outln_info(F("Default state"));
				}
				if (bitRead(test_state, 2) == 1)
				{
					debug_outln_info(F("Not ready"));
				}
				if (bitRead(test_state, 3) == 1)
				{
					debug_outln_info(F("Heat error"));
				}
				if (bitRead(test_state, 4) == 1)
				{
					debug_outln_info(F("T/RH error"));
				}
				if (bitRead(test_state, 5) == 1)
				{
					debug_outln_info(F("Fan error"));

					// if (bitRead(test_state, 0) == 1){
					// 	debug_outln_info(F("Force start NPM..."));
					// 	is_NPM_running = NPM_start_stop();
					// 	delay(5000);
					// }
					// NPM_fan_speed();
					// delay(5000);
				}
				if (bitRead(test_state, 6) == 1)
				{
					debug_outln_info(F("Memory error"));
				}
				if (bitRead(test_state, 7) == 1)
				{
					debug_outln_info(F("Laser error"));
				}
				if (bitRead(test_state, 0) == 0)
				{
					debug_outln_info(F("NPM already started..."));
					is_NPM_running = true;
				}
				else
				{
					debug_outln_info(F("Force start NPM..."));
					is_NPM_running = NPM_start_stop();
				}
			}
		}

		if (nextpmconnected)
		{
			delay(15000);
			NPM_version_date();
			delay(3000);
			NPM_temp_humi();
			delay(2000);
		}
	
}