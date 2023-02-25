#ifndef utils_h
#define utils_h

#include <WString.h>

//Answers Next PM
const uint8_t constexpr answer_stop[4] = {0x81, 0x15, 0x01, 0x69};
const uint8_t constexpr answer_start[4] = {0x81, 0x15, 0x00, 0x6A};
const uint8_t constexpr answer_sleep[4] = {0x81, 0x16, 0x01, 0x68};

enum class PmSensorCmd {
	State,
	Change,
	Concentration,
	Version,
	Speed,
	Temphumi
};

extern void NPM_cmd(PmSensorCmd cmd);
extern bool NPM_checksum_valid_4(const uint8_t (&data)[4]);
extern bool NPM_checksum_valid_5(const uint8_t (&data)[5]);
extern bool NPM_checksum_valid_6(const uint8_t (&data)[6]);
extern bool NPM_checksum_valid_8(const uint8_t (&data)[8]);
extern bool NPM_checksum_valid_16(const uint8_t (&data)[16]);
extern void NPM_data_reader(uint8_t data[], size_t size);
extern String NPM_state(uint8_t bytedata);
