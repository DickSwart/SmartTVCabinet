#include <Arduino.h>
#include <FS.h>
#include <ESP8266WiFi.h> // https://github.com/esp8266/Arduino
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         // https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>         // https://github.com/bblanchon/ArduinoJson
#include <DoubleResetDetector.h> // https://github.com/datacute/DoubleResetDetector
#include <PubSubClient.h>        // https://github.com/knolleary/pubsubclient
#include <ArduinoOTA.h>          // https://github.com/esp8266/Arduino/tree/master/libraries/ArduinoOTA

#include "user_config.h" // Fixed user configurable options
#ifdef USE_CONFIG_OVERRIDE
#include "user_config_override.h" // Configuration overrides for my_user_config.h
#endif

// Display debug output
#ifdef DEBUG
bool enableDebug = true;
#else
bool enableDebug = false;
#endif

/* --------------------------------------------------------------------------------------------------
 * File System
 * -------------------------------------------------------------------------------------------------- */
// Methods
boolean
loadConfig();
boolean saveConfig();

/* --------------------------------------------------------------------------------------------------
 * Double Reset Detector
 * -------------------------------------------------------------------------------------------------- */
// Initialize
DoubleResetDetector drd(DRD_TIMEOUT, DRD_ADDRESS);

/* --------------------------------------------------------------------------------------------------
 * WiFiManager
 * -------------------------------------------------------------------------------------------------- */
// function declaration
void saveConfigCallback();
void configModeCallback(WiFiManager *myWiFiManager);
void forceConfigMode();

// define your default values here, if there are different values in config.json, they are overwritten.
char mqtt_server[40] = MQTT_SERVER;
char mqtt_port[6] = MQTT_SERVER_PORT;
char mqtt_username[50] = MQTT_USERNAME;
char mqtt_password[50] = MQTT_PASSWORD;

bool shouldSaveConfig = false; //flag for saving data

/* --------------------------------------------------------------------------------------------------
 * Main Setup & loop
 * -------------------------------------------------------------------------------------------------- */
void setup()
{
  Serial.begin(115200);
  Serial.println();

  if (!SPIFFS.begin())
  {
    Serial.println("Failed to mount File system");
    return;
  }

  pinMode(LED_BUILTIN, OUTPUT);   // Initialize the LED_BUILTIN pin as an output
  digitalWrite(LED_BUILTIN, LOW); // Turn the LED on (Note that LOW is the voltage level
  loadConfig();

  // Over the air
  ArduinoOTA.begin();

  WiFiManager wifiManager;
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.setSTAStaticIPConfig(
      IPAddress(10, 0, 1, 99),
      IPAddress(10, 0, 1, 1),
      IPAddress(255, 255, 255, 0));
  wifiManager.setMinimumSignalQuality(30);
  wifiManager.setDebugOutput(enableDebug);

  // Adding an additional config on the WIFI manager webpage for the MQTT Server.
  WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
  WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 6);
  WiFiManagerParameter custom_mqtt_username("username", "mqtt username", mqtt_username, 50);
  WiFiManagerParameter custom_mqtt_password("password", "mqtt password", mqtt_password, 50);
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_mqtt_username);
  wifiManager.addParameter(&custom_mqtt_password);

  String ssid = "SwartNinjaNoT" + String(ESP.getChipId());
  if (drd.detectDoubleReset())
  {
    Serial.println("Double Reset Detected");
    wifiManager.startConfigPortal(ssid.c_str(), "SwartNinja");
  }
  else
  {
    Serial.println("No Double Reset Detected");
    wifiManager.autoConnect(ssid.c_str(), "SwartNinja");
  }

  // read updated parameters
  strcpy(mqtt_server, custom_mqtt_server.getValue());
  strcpy(mqtt_port, custom_mqtt_port.getValue());
  strcpy(mqtt_username, custom_mqtt_username.getValue());
  strcpy(mqtt_password, custom_mqtt_password.getValue());

  if (shouldSaveConfig)
  {
    saveConfig();
  }

  digitalWrite(LED_BUILTIN, HIGH); // Turn the LED off by making the voltage HIGH

  if ((strcmp(mqtt_server, "") > 0) && (strcmp(mqtt_port, "") > 0))
  {
    Serial.println("Initialise MQTT Server");
    // TODO: ADD CODE
  }
  else
  {
    Serial.println("Forcing Config Mode");
    forceConfigMode();
  }

  // Print Network Info
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  drd.stop();
}

void loop()
{
  // Over the air
  ArduinoOTA.handle();

  // Call the double reset detector loop method every so often,
  // so that it can recognise when the timeout expires.
  // You can also call drd.stop() when you wish to no longer
  // consider the next reset as a double reset.
  drd.loop();
}

/* --------------------------------------------------------------------------------------------------
 * File System
 * -------------------------------------------------------------------------------------------------- */
boolean loadConfig()
{
  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile)
  {
    Serial.println("Failed to open config file");
    return false;
  }

  size_t size = configFile.size();
  if (size > 1024)
  {
    Serial.println("Config file size is too large");
    return false;
  }

  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  configFile.readBytes(buf.get(), size);

  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, buf.get());
  if (error)
  {
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(error.c_str());
    return false;
  }

  strcpy(mqtt_server, doc["mqtt_server"]);
  strcpy(mqtt_port, doc["mqtt_port"]);
  strcpy(mqtt_username, doc["mqtt_username"]);
  strcpy(mqtt_password, doc["mqtt_password"]);

  configFile.close();
  return true;
}

boolean saveConfig()
{
  Serial.println("Save config file to file system.");

  DynamicJsonDocument doc(1024);
  doc["mqtt_server"] = mqtt_server;
  doc["mqtt_port"] = mqtt_port;
  doc["mqtt_username"] = mqtt_username;
  doc["mqtt_password"] = mqtt_password;
  serializeJson(doc, Serial);

  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile)
  {
    Serial.println("Failed to open config file for writing.");
    return false;
  }

  serializeJson(doc, configFile);
  configFile.close();
  return true;
}

/* --------------------------------------------------------------------------------------------------
 * WiFi Manager
 * -------------------------------------------------------------------------------------------------- */

// Callback notifying us of the need to save config
void saveConfigCallback()
{
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

// You could indicate on your screen or by an LED you are in config mode here
// We don't want the next time the board resets to be considered a double reset
// so we remove the flag
void configModeCallback(WiFiManager *myWiFiManager)
{
  drd.stop();
}

void forceConfigMode()
{
  Serial.println("Reset");
  WiFi.disconnect();
  delay(500);
  ESP.restart();
  delay(5000);
}
