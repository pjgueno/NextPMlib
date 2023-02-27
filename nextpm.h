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

struct NextPM_dataPM
{
	// unit : mg/m3
	float PM1;
	float PM2_5;
	float PM10;

	// unit : PC/l
	float PM1_NC;
	float PM2_5_NC;
	float PM10_NC;
};

struct NextPM_dataTH
{
	float temp;
	float humi;
};

struct NextPM_test
{
	bool connected;
	bool sleep;
	bool degraded;
	bool default_state;
	bool notready;
	bool heat_error;
	bool TH_error;
	bool fan_error;
	bool memory_error;
	bool laser_error;
};

const uint8_t constexpr answer_stop[4] = {0x81, 0x15, 0x01, 0x69};
const uint8_t constexpr answer_start[4] = {0x81, 0x15, 0x00, 0x6A};
const uint8_t constexpr answer_sleep[4] = {0x81, 0x16, 0x01, 0x68};

enum class PmSensorCmd
{
	State,
	Change,
	Concentration10s,
	Concentration60s,
	Concentration900s,
	Version,
	Speedcurrent,
	Speed25,
	Speed50,
	Speed75,
	Speed100,
	Temphumi
};

class NextPM
{
public:
	// NextPM(void);

#ifndef ESP32
	void begin(uint8_t pin_rx, uint8_t pin_tx);
#endif
#ifdef ESP32
	void begin(HardwareSerial *serial, int8_t pin_rx, int8_t pin_tx);
#endif

	int8_t get_state();
	bool start_stop();
	String version_date();
	void fan_speed();
	void powerOnTest(NextPM_test &);
	void fetchDataPM(NextPM_dataPM &, int sel);
	void fetchDataTH(NextPM_dataTH &);

private:
	uint8_t _pin_rx, _pin_tx;
	bool checksum_valid_4(const uint8_t (&data)[4]);
	bool checksum_valid_5(const uint8_t (&data)[5]);
	bool checksum_valid_6(const uint8_t (&data)[6]);
	bool checksum_valid_8(const uint8_t (&data)[8]);
	bool checksum_valid_16(const uint8_t (&data)[16]);
	void data_reader(uint8_t data[], size_t size);
	String state_npm(uint8_t bytedata);
	void command(PmSensorCmd cmd);

	Stream *npm_data;
};

#endif