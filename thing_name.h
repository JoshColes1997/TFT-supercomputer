/*
#define MQTT_BROKER "test.mosquitto.org"
#define THING_NAME "ASR"
#define LIGHT_LEVEL_TOPIC "ASR/light"
#define TEMPERATURE_TOPIC "ASR/temperature"
#define ANNOUNCE_TOPIC "mytopic/announce"
#define LIGHT_THRESH_TOPIC "ASR/lthresh"
#define TEMP_THRESH_TOPIC "ASR/tthresh"
*/
#ifndef THING_NAME_H
#define THING_NAME_H
#endif

#define THING_NAME "RFID_Reader_01"
#define MQTT_BROKER "192.168.2.207" // use this in college
//#define MQTT_BROKER "test.mosquitto.org" // use this when not in college
#define MQTT_PORT (1883)

#define RFID_TOPIC "rfid/card"
#define RFID_ACCESS_TOPIC "rfid/access"
#define ANNOUNCE_TOPIC "rfid/announce"
#define STATUS_TOPIC "rfid/status"

