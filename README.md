# Next PM Library 
  
Arduino library for the TERA Sensor [NextPM](https://tera-sensor.com/markets-products-services/nextpm/?portfolioCats=58%2C57%2C59) PM Sensor.
  
## Usage

| Function                 | Usage   |
| ------------------------ | ------- |
| get_state()              |         |
| start_stop()             |         |
| version_date()           |         |
| fan_speed()              |         |
| powerOnTest(data)        |         |
| fetchDataPM(data,option) |         |
| fetchDataTH(data)        |         |

The data variables are some struct containing the values:

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

