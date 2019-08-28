#ifndef _USER_CONFIG_H_
#define _USER_CONFIG_H_

/* --------------------------------------------------------------------------------------------------
 * WiFi
 * -------------------------------------------------------------------------------------------------- */
#define WIFI_SSID "wifi_ssid"
#define WIFI_PASSWORD "wifi_password"
#define WIFI_QUALITY_OFFSET_VALUE 2
#define WIFI_QUALITY_INTERVAL 50000 // [ms]
#define WIFI_QUALITY_SENSOR_NAME "wifi"

/* --------------------------------------------------------------------------------------------------
 * MQTT
 * -------------------------------------------------------------------------------------------------- */
#define MQTT_SERVER "xxx.xxx.xxx.xxx"
#define MQTT_SERVER_PORT "1883"
#define MQTT_USERNAME "mqtt_user_name"
#define MQTT_PASSWORD "mqtt_password"

#define MQTT_AVAILABILITY_TOPIC_TEMPLATE "%s/status" // MQTT availability: online/offline
#define MQTT_SENSOR_TOPIC_TEMPLATE "%s/sensor/%s"

#define MQTT_PAYLOAD_ON "ON"
#define MQTT_PAYLOAD_OFF "OFF"


/* --------------------------------------------------------------------------------------------------
 * Double Reset Detector
 * -------------------------------------------------------------------------------------------------- */
// Number of seconds after reset during which a subseqent reset will be considered a double reset.
#define DRD_TIMEOUT 10

// RTC Memory Address for the DoubleResetDetector to use
#define DRD_ADDRESS 0


/* --------------------------------------------------------------------------------------------------
 * Over-the-Air update (OTA)
 * -------------------------------------------------------------------------------------------------- */
#define OTA_PORT 8266  // port 8266 by default

#endif  // _USER_CONFIG_H_
