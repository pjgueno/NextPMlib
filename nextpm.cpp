#include <WString.h>
#include <nextpm.h>

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

template <typename T, std::size_t N>
constexpr std::size_t array_num_elements(const T (&)[N])
{
	return N;
}

/*****************************************************************
 * NPM functions     *
 *****************************************************************/

#ifndef ESP32
void NextPM::begin(uint8_t pin_rx, uint8_t pin_tx)
{
	_pin_rx = pin_rx;
	_pin_tx = pin_tx;

	SoftwareSerial *softSerial = new SoftwareSerial(_pin_rx, _pin_tx);

	softSerial->begin(9600);
	npm_data = softSerial;
}

#endif

#ifdef ESP32
void NextPM::begin(HardwareSerial *serial, int8_t rxPin, int8_t txPin)
{
	serial->begin(115200, SERIAL_8E1, rxPin, txPin);
	npm_data = serial;
}
#endif

int8_t NextPM::get_state()
{
	int8_t result = -1;
	NPM_waiting_for_4 = NPM_REPLY_HEADER_4;
	Serial.println("State NPM...");
	command(PmSensorCmd::State);

	unsigned long timeout = millis();

	do
	{
		Serial.println("Wait for Serial...");
	} while (!npm_data->available() && millis() - timeout < 3000);

	while (npm_data->available() >= NPM_waiting_for_4)
	{
		const uint8_t constexpr header[2] = {0x81, 0x16};
		uint8_t state[1];
		uint8_t checksum[1];
		uint8_t test[4];

		switch (NPM_waiting_for_4)
		{
		case NPM_REPLY_HEADER_4:
			if (npm_data->find(header, sizeof(header)))
				NPM_waiting_for_4 = NPM_REPLY_STATE_4;
			break;
		case NPM_REPLY_STATE_4:
			npm_data->readBytes(state, sizeof(state));
			state_npm(state[0]);
			result = state[0];
			NPM_waiting_for_4 = NPM_REPLY_CHECKSUM_4;
			break;
		case NPM_REPLY_CHECKSUM_4:
			npm_data->readBytes(checksum, sizeof(checksum));
			memcpy(test, header, sizeof(header));
			memcpy(&test[sizeof(header)], state, sizeof(state));
			memcpy(&test[sizeof(header) + sizeof(state)], checksum, sizeof(checksum));
			data_reader(test, 4);
			NPM_waiting_for_4 = NPM_REPLY_HEADER_4;
			if (checksum_valid_4(test))
			{
				Serial.println("Checksum OK...");
			}
			else
			{
				Serial.println("Checksum invalid...");
			}
			break;
		}
	}
	return result;
}

bool NextPM::start_stop()
{
	bool result;
	NPM_waiting_for_4 = NPM_REPLY_HEADER_4;
	Serial.println("Switch start/stop NPM...");
	command(PmSensorCmd::Change);

	unsigned long timeout = millis();

	do
	{
		Serial.println("Wait for Serial...");
	} while (!npm_data->available() && millis() - timeout < 3000);

	while (npm_data->available() >= NPM_waiting_for_4)
	{
		const uint8_t constexpr header[2] = {0x81, 0x15};
		uint8_t state[1];
		uint8_t checksum[1];
		uint8_t test[4];

		switch (NPM_waiting_for_4)
		{
		case NPM_REPLY_HEADER_4:
			if (npm_data->find(header, sizeof(header)))
				NPM_waiting_for_4 = NPM_REPLY_STATE_4;
			break;
		case NPM_REPLY_STATE_4:
			npm_data->readBytes(state, sizeof(state));
			state_npm(state[0]);

			if (bitRead(state[0], 0) == 0)
			{
				Serial.println("NPM start...");
				result = true;
			}
			else if (bitRead(state[0], 0) == 1)
			{
				Serial.println("NPM stop...");
				result = false;
			}

			NPM_waiting_for_4 = NPM_REPLY_CHECKSUM_4;
			break;
		case NPM_REPLY_CHECKSUM_4:
			npm_data->readBytes(checksum, sizeof(checksum));
			memcpy(test, header, sizeof(header));
			memcpy(&test[sizeof(header)], state, sizeof(state));
			memcpy(&test[sizeof(header) + sizeof(state)], checksum, sizeof(checksum));
			data_reader(test, 4);
			NPM_waiting_for_4 = NPM_REPLY_HEADER_4;
			if (checksum_valid_4(test))
			{
				Serial.println("Checksum OK...");
			}
			break;
		}
	}
	return result;
}

String NextPM::version_date()
{
	uint16_t NPMversion;
	delay(250);
	NPM_waiting_for_6 = NPM_REPLY_HEADER_6;
	Serial.println("Version NPM...");
	command(PmSensorCmd::Version);

	unsigned long timeout = millis();

	do
	{
		Serial.println("Wait for Serial...");
	} while (!npm_data->available() && millis() - timeout < 3000);

	while (npm_data->available() >= NPM_waiting_for_6)
	{
		const uint8_t constexpr header[2] = {0x81, 0x17};
		uint8_t state[1];
		uint8_t data[2];
		uint8_t checksum[1];
		uint8_t test[6];

		switch (NPM_waiting_for_6)
		{
		case NPM_REPLY_HEADER_6:
			if (npm_data->find(header, sizeof(header)))
				NPM_waiting_for_6 = NPM_REPLY_STATE_6;
			break;
		case NPM_REPLY_STATE_6:
			npm_data->readBytes(state, sizeof(state));
			state_npm(state[0]);
			NPM_waiting_for_6 = NPM_REPLY_DATA_6;
			break;
		case NPM_REPLY_DATA_6:
			if (npm_data->readBytes(data, sizeof(data)) == sizeof(data))
			{
				data_reader(data, 2);
				NPMversion = word(data[0], data[1]);
			}
			NPM_waiting_for_6 = NPM_REPLY_CHECKSUM_6;
			break;
		case NPM_REPLY_CHECKSUM_6:
			npm_data->readBytes(checksum, sizeof(checksum));
			memcpy(test, header, sizeof(header));
			memcpy(&test[sizeof(header)], state, sizeof(state));
			memcpy(&test[sizeof(header) + sizeof(state)], data, sizeof(data));
			memcpy(&test[sizeof(header) + sizeof(state) + sizeof(data)], checksum, sizeof(checksum));
			data_reader(test, 6);
			NPM_waiting_for_6 = NPM_REPLY_HEADER_6;
			if (checksum_valid_6(test))
			{
				Serial.println("Checksum OK...");
				Serial.print("Next PM Firmware: ");
				Serial.println(NPMversion);
				return String(NPMversion);
			}
			else
			{
				Serial.println("Checksum invalid...");
				return String("Error");
			}
			break;
		}
	}
}

void NextPM::fan_speed()
{

	NPM_waiting_for_5 = NPM_REPLY_HEADER_5;
	Serial.println("Set fan speed to 50 %...");
	command(PmSensorCmd::Speed50);

	unsigned long timeout = millis();

	do
	{
		Serial.println("Wait for Serial...");
	} while (!npm_data->available() && millis() - timeout < 3000);

	while (npm_data->available() >= NPM_waiting_for_5)
	{
		const uint8_t constexpr header[2] = {0x81, 0x21};
		uint8_t state[1];
		uint8_t data[1];
		uint8_t checksum[1];
		uint8_t test[5];

		switch (NPM_waiting_for_5)
		{
		case NPM_REPLY_HEADER_5:
			if (npm_data->find(header, sizeof(header)))
				NPM_waiting_for_5 = NPM_REPLY_STATE_5;
			break;
		case NPM_REPLY_STATE_5:
			npm_data->readBytes(state, sizeof(state));
			state_npm(state[0]);
			NPM_waiting_for_5 = NPM_REPLY_DATA_5;
			break;
		case NPM_REPLY_DATA_5:
			if (npm_data->readBytes(data, sizeof(data)) == sizeof(data))
			{
				data_reader(data, 1);
			}
			NPM_waiting_for_5 = NPM_REPLY_CHECKSUM_5;
			break;
		case NPM_REPLY_CHECKSUM_5:
			npm_data->readBytes(checksum, sizeof(checksum));
			memcpy(test, header, sizeof(header));
			memcpy(&test[sizeof(header)], state, sizeof(state));
			memcpy(&test[sizeof(header) + sizeof(state)], data, sizeof(data));
			memcpy(&test[sizeof(header) + sizeof(state) + sizeof(data)], checksum, sizeof(checksum));
			data_reader(test, 5);
			NPM_waiting_for_5 = NPM_REPLY_HEADER_5;
			if (checksum_valid_5(test))
			{
				Serial.println("Checksum OK...");
			}
			break;
		}
	}
}

void NextPM::fetchDataTH(NextPM_dataTH &datain)
{
	NPM_waiting_for_8 = NPM_REPLY_HEADER_8;

	Serial.println("Temperature/Humidity in Next PM...");
	command(PmSensorCmd::Temphumi);

	unsigned long timeout = millis();

	do
	{
		Serial.println("Wait for Serial...");
	} while (!npm_data->available() && millis() - timeout < 3000);

	while (npm_data->available() >= NPM_waiting_for_8)
	{
		const uint8_t constexpr header[2] = {0x81, 0x14};
		uint8_t state[1];
		uint8_t data[4];
		uint8_t checksum[1];
		uint8_t test[8];
		uint16_t NPM_temp;
		uint16_t NPM_humi;

		switch (NPM_waiting_for_8)
		{
		case NPM_REPLY_HEADER_8:
			if (npm_data->find(header, sizeof(header)))
				NPM_waiting_for_8 = NPM_REPLY_STATE_8;
			break;
		case NPM_REPLY_STATE_8:
			npm_data->readBytes(state, sizeof(state));
			state_npm(state[0]);
			NPM_waiting_for_8 = NPM_REPLY_BODY_8;
			break;
		case NPM_REPLY_BODY_8:
			if (npm_data->readBytes(data, sizeof(data)) == sizeof(data))
			{
				data_reader(data, 4);
				NPM_temp = word(data[0], data[1]);
				NPM_humi = word(data[2], data[3]);
			}
			NPM_waiting_for_8 = NPM_REPLY_CHECKSUM_8;
			break;
		case NPM_REPLY_CHECKSUM_16:
			npm_data->readBytes(checksum, sizeof(checksum));
			memcpy(test, header, sizeof(header));
			memcpy(&test[sizeof(header)], state, sizeof(state));
			memcpy(&test[sizeof(header) + sizeof(state)], data, sizeof(data));
			memcpy(&test[sizeof(header) + sizeof(state) + sizeof(data)], checksum, sizeof(checksum));
			data_reader(test, 8);
			if (checksum_valid_8(test))
			{
				Serial.println("Checksum OK...");
				Serial.print("Temperature (°C): ");
				Serial.println(String(NPM_temp / 100.0f));
				Serial.print("Relative humidity (%): ");
				Serial.println(String(NPM_humi / 100.0f));

				datain.temp = float(NPM_temp) / 100.0f;
				datain.humi = float(NPM_humi) / 100.0f;
			}
			else
			{
				Serial.println("Checksum invalid...");
				datain.temp = -1.0;
				datain.humi = -1.0;
			}
			NPM_waiting_for_8 = NPM_REPLY_HEADER_8;
			break;
		}
	}
}

void NextPM::fetchDataPM(NextPM_dataPM &datain, int sel)
{

	NPM_waiting_for_16 = NPM_REPLY_HEADER_16;

	switch (sel)
	{
	case 10:
		Serial.println("Concentration NPM 10s...");
		command(PmSensorCmd::Concentration10s);
		break;
	case 60:
		Serial.println("Concentration NPM 60s...");
		command(PmSensorCmd::Concentration60s);
		break;
	case 900:
		Serial.println("Concentration NPM 900s...");
		command(PmSensorCmd::Concentration900s);
		break;
	}

	unsigned long timeout = millis();

	do
	{
		Serial.println("Wait for Serial...");
	} while (!npm_data->available() && millis() - timeout < 3000);

	while (npm_data->available() >= NPM_waiting_for_16)
	{

		const uint8_t constexpr header10[2] = {0x81, 0x11};
		const uint8_t constexpr header60[2] = {0x81, 0x12};
		const uint8_t constexpr header900[2] = {0x81, 0x13};

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
			if (npm_data->find(header10, sizeof(header10)) || npm_data->find(header60, sizeof(header60)) || npm_data->find(header900, sizeof(header900)))
				NPM_waiting_for_16 = NPM_REPLY_STATE_16;
			break;
		case NPM_REPLY_STATE_16:
			npm_data->readBytes(state, sizeof(state));
			Serial.print("Current state: ");
			Serial.println(state_npm(state[0]));
			NPM_waiting_for_16 = NPM_REPLY_BODY_16;
			break;
		case NPM_REPLY_BODY_16:
			if (npm_data->readBytes(data, sizeof(data)) == sizeof(data))
			{
				data_reader(data, 12);
				N1_serial = word(data[0], data[1]);
				N25_serial = word(data[2], data[3]);
				N10_serial = word(data[4], data[5]);

				pm1_serial = word(data[6], data[7]);
				pm25_serial = word(data[8], data[9]);
				pm10_serial = word(data[10], data[11]);
			}
			NPM_waiting_for_16 = NPM_REPLY_CHECKSUM_16;
			break;
		case NPM_REPLY_CHECKSUM_16:
			npm_data->readBytes(checksum, sizeof(checksum));

			if (sel == 10)
			{
				memcpy(test, header10, sizeof(header10));
				memcpy(&test[sizeof(header10)], state, sizeof(state));
				memcpy(&test[sizeof(header10) + sizeof(state)], data, sizeof(data));
				memcpy(&test[sizeof(header10) + sizeof(state) + sizeof(data)], checksum, sizeof(checksum));
			}
			if (sel == 60)
			{
				memcpy(test, header60, sizeof(header60));
				memcpy(&test[sizeof(header60)], state, sizeof(state));
				memcpy(&test[sizeof(header60) + sizeof(state)], data, sizeof(data));
				memcpy(&test[sizeof(header60) + sizeof(state) + sizeof(data)], checksum, sizeof(checksum));
			}
			if (sel == 900)
			{
				memcpy(test, header900, sizeof(header900));
				memcpy(&test[sizeof(header900)], state, sizeof(state));
				memcpy(&test[sizeof(header900) + sizeof(state)], data, sizeof(data));
				memcpy(&test[sizeof(header900) + sizeof(state) + sizeof(data)], checksum, sizeof(checksum));
			}

			data_reader(test, 16);
			if (checksum_valid_16(test))
			{
				Serial.println("Checksum OK...");

				Serial.print("PM1 (μg/m3) : ");
				Serial.println(String(pm1_serial / 10.0f));
				Serial.print("PM2.5 (μg/m3): ");
				Serial.println(String(pm25_serial / 10.0f));
				Serial.print("PM10 (μg/m3) : ");
				Serial.println(String(pm10_serial / 10.0f));

				Serial.print("PM1 (pcs/L) : ");
				Serial.println(String(N1_serial));
				Serial.print("PM2.5 (pcs/L): ");
				Serial.println(String(N25_serial));
				Serial.print("PM10 (pcs/L) : ");
				Serial.println(String(N10_serial));

				datain.PM1 = float(pm1_serial) / 10.0f;
				datain.PM2_5 = float(pm25_serial) / 10.0f;
				datain.PM10 = float(pm10_serial) / 10.0f;

				datain.PM1_NC = float(N1_serial);
				datain.PM2_5_NC = float(N25_serial);
				datain.PM10_NC = float(N10_serial);
			}
			else
			{
				Serial.println("Checksum invalid...");
				datain.PM1 = -1.0;
				datain.PM2_5 = -1.0;
				datain.PM10 = -1.0;

				datain.PM1_NC = -1.0;
				datain.PM2_5_NC = -1.0;
				datain.PM10_NC = -1.0;
			}
			NPM_waiting_for_16 = NPM_REPLY_HEADER_16;
			break;
		}
	}
	// debug_outln_info(F("Temperature and humidity in NPM after measure..."));
	// current_th_npm = NPM_temp_humi();
}

void NextPM::powerOnTest(NextPM_test &datain)
{
	datain.connected = false;
	datain.sleep = false;
	datain.degraded = false;
	datain.default_state = false;
	datain.notready = false;
	datain.heat_error = false;
	datain.TH_error = false;
	datain.fan_error = false;
	datain.memory_error = false;
	datain.laser_error = false;

	int8_t test_state;
	delay(15000); // wait a bit to be sure Next PM is ready to receive instructions.
	test_state = get_state();
	if (test_state == -1)
	{
		Serial.println("NPM not connected");
		datain.connected = false;
	}
	else
	{
		datain.connected = true;
		if (test_state == 0x00)
		{
			datain.sleep = false;
			datain.degraded = false;
			datain.default_state = false;
			datain.notready = false;
			datain.heat_error = false;
			datain.TH_error = false;
			datain.fan_error = false;
			datain.memory_error = false;
			datain.laser_error = false;
		}
		else if (test_state == 0x01 && bitRead(test_state, 1) == 0)
		{
			Serial.println("NPM is stopped..."); // to read the firmware version
			datain.sleep = true;
			datain.degraded = false;
			datain.default_state = false;
		}
		else
		{
			if (bitRead(test_state, 1) == 1)
			{
				Serial.println("Degraded state");
				datain.degraded = true;
				datain.default_state = false;
			}
			else
			{
				Serial.println("Default state");
				datain.sleep = true;
				datain.degraded = false;
				datain.default_state = true;
			}
			if (bitRead(test_state, 2) == 1)
			{
				Serial.println("Not ready");
				datain.notready = true;
			}
			if (bitRead(test_state, 3) == 1)
			{
				Serial.println("Heat error");
				datain.heat_error = true;
			}
			if (bitRead(test_state, 4) == 1)
			{
				Serial.println("T/RH error");
				datain.TH_error = true;
			}
			if (bitRead(test_state, 5) == 1)
			{
				Serial.println("Fan error");
				datain.fan_error = true;
			}
			if (bitRead(test_state, 6) == 1)
			{
				Serial.println("Memory error");
				datain.memory_error = true;
			}
			if (bitRead(test_state, 7) == 1)
			{
				Serial.println("Laser error");
				datain.laser_error = true;
			}
		}
	}

	// if (nextpmconnected)
	// {
	// 	delay(15000);
	// 	version_date();
	// 	delay(3000);
	// 	temp_humi();
	// 	delay(2000);
	// }
}

bool NextPM::checksum_valid_4(const uint8_t (&data)[4])
{
	uint8_t sum = data[0] + data[1] + data[2] + data[3];
	uint8_t checksum = sum % 0x100;
	return (checksum == 0);
}

bool NextPM::checksum_valid_5(const uint8_t (&data)[5])
{
	uint8_t sum = data[0] + data[1] + data[2] + data[3] + data[4];
	uint8_t checksum = sum % 0x100;
	return (checksum == 0);
}

bool NextPM::checksum_valid_6(const uint8_t (&data)[6])
{
	uint8_t sum = data[0] + data[1] + data[2] + data[3] + data[4] + data[5];
	uint8_t checksum = sum % 0x100;
	return (checksum == 0);
}

bool NextPM::checksum_valid_8(const uint8_t (&data)[8])
{
	uint8_t sum = data[0] + data[1] + data[2] + data[3] + data[4] + data[5] + data[6] + data[7];
	uint8_t checksum = sum % 0x100;
	return (checksum == 0);
}

bool NextPM::checksum_valid_16(const uint8_t (&data)[16])
{
	uint8_t sum = data[0] + data[1] + data[2] + data[3] + data[4] + data[5] + data[6] + data[7] + data[8] + data[9] + data[10] + data[11] + data[12] + data[13] + data[14] + data[15];
	uint8_t checksum = sum % 0x100;
	return (checksum == 0);
}

void NextPM::command(PmSensorCmd cmd)
{

	constexpr uint8_t state_cmd[] PROGMEM = {
		0x81, 0x16, 0x69 //read the current state
	};

	constexpr uint8_t change_cmd[] PROGMEM = {
		0x81, 0x15, 0x6A //change the sate alternatively start/stop
	};

	constexpr uint8_t concentration_cmd_10s[] PROGMEM = {
		0x81, 0x11, 0x6E //Concentrations reading’s averaged over 10 seconds and updated every 1 second
	};

	constexpr uint8_t concentration_cmd_60s[] PROGMEM = {
		0x81, 0x12, 0x6D //Concentrations reading’s averaged over 60 seconds and updated every 10 seconds
	};

	constexpr uint8_t concentration_cmd_900s[] PROGMEM = {
		0x81, 0x13, 0x6C //Concentrations reading’s averaged over 900 seconds and updated every 60 seconds
	};

	constexpr uint8_t version_cmd[] PROGMEM = {
		0x81, 0x17, 0x68};

	constexpr uint8_t speed_cmd_current[] PROGMEM = {
		0x81, 0x21, 0x00, 0x5E //0% to get current value
	};

	constexpr uint8_t speed_cmd_set_25[] PROGMEM = {
		0x81, 0x21, 0x32, 0x2C //50%
	};

	constexpr uint8_t speed_cmd_set_50[] PROGMEM = {
		0x81, 0x21, 0x32, 0x2C //50%
	};

	constexpr uint8_t speed_cmd_set_75[] PROGMEM = {
		0x81, 0x21, 0x32, 0x2C //50%
	};

	constexpr uint8_t speed_cmd_set_100[] PROGMEM = {
		0x81, 0x21, 0x32, 0x2C //50%
	};

	constexpr uint8_t temphumi_cmd[] PROGMEM = {
		0x81, 0x14, 0x6B};

	//0x81 + 0x21 + 0x55 + 0x09 = 0x100

	constexpr uint8_t cmd_len = array_num_elements(change_cmd);
	uint8_t buf[cmd_len];

	switch (cmd)
	{
	case PmSensorCmd::State:
		memcpy_P(buf, state_cmd, cmd_len);
		break;
	case PmSensorCmd::Change:
		memcpy_P(buf, change_cmd, cmd_len);
		break;
	case PmSensorCmd::Concentration10s:
		memcpy_P(buf, concentration_cmd_10s, cmd_len);
		break;
	case PmSensorCmd::Version:
		memcpy_P(buf, version_cmd, cmd_len);
		break;
	case PmSensorCmd::Speed50:
		memcpy_P(buf, speed_cmd_set_50, cmd_len);
		break;
	case PmSensorCmd::Temphumi:
		memcpy_P(buf, temphumi_cmd, cmd_len);
		break;
	}
	npm_data->write(buf, cmd_len);
}

void NextPM::data_reader(uint8_t data[], size_t size)
{
	String reader = "Read: ";
	for (size_t i = 0; i < size; i++)
	{
		reader += "0x";
		if (data[i] < 0x10)
		{
			reader += "0";
		}
		reader += String(data[i], HEX);
		if (i != (size - 1))
		{
			reader += ", ";
		}
	}
	Serial.println(reader);
}

String NextPM::state_npm(uint8_t bytedata)
{
	String state = "State: ";

	for (int b = 7; b >= 0; b--)
	{
		state += String(bitRead(bytedata, b));
	}
	Serial.println(state);
	return state;
}