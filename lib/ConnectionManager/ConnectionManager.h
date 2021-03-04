#ifndef ConnectionManager_h
#define ConnectionManager_h

// #include <FS.h>          // this needs to be first, or it all crashes and burns...
#include <LittleFS.h>
#include <Arduino.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <Arduino_JSON.h>

class ConnectionManager {
public:
  struct Configuration {
    char      mqtt_server[32];
    uint16_t  mqtt_port;
    char      mqtt_user[16];
    char      mqtt_password[32];
    float     actual_counter;
    uint8_t   rotations_per_kwh;
  } config;

  ConnectionManager();
  void resetConfiguration();
  void init();

  void saveConfigCallback();

private:
  WiFiManager wifiManager;
  bool _shouldSaveConfig         = false;
  const char *configFile         = "/config.json";
  const char *apSSIDPrefix       = "MRT-PowerMeter";
  
  void readConfiguration();
};

#endif