#include <WString.h>
#include "./utils.h"

/*********************************************************************************
 * send Tera Sensor Next PM sensor command state, change, concentration, version *
 *********************************************************************************/

bool NPM_checksum_valid_4(const uint8_t (&data)[4]) {
	uint8_t sum = data[0] + data[1] + data[2] + data[3];
	uint8_t checksum = sum % 0x100;
	return (checksum == 0);
}

bool NPM_checksum_valid_5(const uint8_t (&data)[5]) {
	uint8_t sum = data[0] + data[1] + data[2] + data[3] + data[4];
	uint8_t checksum = sum % 0x100;
	return (checksum == 0);
}

bool NPM_checksum_valid_6(const uint8_t (&data)[6])
{
	uint8_t sum = data[0] + data[1] + data[2] + data[3] + data[4] + data[5];
	uint8_t checksum = sum % 0x100;
	return (checksum == 0);
}

bool NPM_checksum_valid_8(const uint8_t (&data)[8])
{
	uint8_t sum = data[0] + data[1] + data[2] + data[3] + data[4] + data[5] + data[6] + data[7];
	uint8_t checksum = sum % 0x100;
	return (checksum == 0);
}

bool NPM_checksum_valid_16(const uint8_t (&data)[16]) {
	uint8_t sum = data[0] + data[1] + data[2] + data[3] + data[4] + data[5] + data[6] + data[7] + data[8] + data[9] + data[10] + data[11] + data[12] + data[13] + data[14] + data[15];
	uint8_t checksum = sum % 0x100;
	return (checksum == 0);
}



void NPM_cmd(PmSensorCmd cmd) {

	static constexpr uint8_t state_cmd[] PROGMEM = { //read the current state
		0x81, 0x16, 0x69
	};
	static constexpr uint8_t change_cmd[] PROGMEM = { //change the sate alternatively start/stop
		0x81, 0x15, 0x6A
	};
	static constexpr uint8_t concentration_cmd[] PROGMEM = { //No continous mode => repeat call
		// 0x81, 0x11, 0x6E    //Concentrations reading’s averaged over 10 seconds and updated every 1 second
			0x81, 0x12, 0x6D    //Concentrations reading’s averaged over 60 seconds and updated every 10 seconds
	};



	static constexpr uint8_t version_cmd[] PROGMEM = {
		0x81, 0x17, 0x68 
	};

	static constexpr uint8_t speed_cmd[] PROGMEM = {
		//0x81, 0x21, 0x00, 0x5E //0% to get current value
		0x81, 0x21, 0x32, 0x2C //50% 
	};

	static constexpr uint8_t temphumi_cmd[] PROGMEM = {
		0x81, 0x14, 0x6B
	};

//0x81 + 0x21 + 0x55 + 0x09 = 0x100

	constexpr uint8_t cmd_len = array_num_elements(change_cmd);
	uint8_t buf[cmd_len];

	switch (cmd) {
	case PmSensorCmd::State:
		memcpy_P(buf, state_cmd, cmd_len);
		break;
	case PmSensorCmd::Change:
		memcpy_P(buf, change_cmd, cmd_len);
		break;
	case PmSensorCmd::Concentration:
		memcpy_P(buf, concentration_cmd, cmd_len);
		break;
	case PmSensorCmd::Version:
		memcpy_P(buf, version_cmd, cmd_len);
		break;
	case PmSensorCmd::Speed:
		memcpy_P(buf, speed_cmd, cmd_len);
		break;
	case PmSensorCmd::Temphumi:
		memcpy_P(buf, temphumi_cmd, cmd_len);
		break;
	}
	serialNPM.write(buf, cmd_len);
}

/*****************************************************************
 * Helpers                                                       *
 *****************************************************************/






/*****************************************************************
 * Helpers                                                       *
 *****************************************************************/


void NPM_data_reader(uint8_t data[], size_t size)
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

String NPM_state(uint8_t bytedata)
	{
		String state = "State: ";

		for (int b = 7; b >= 0; b--)
		{
			state += String(bitRead(bytedata, b));
		}
		Serial.println(state);
		return state;
	}