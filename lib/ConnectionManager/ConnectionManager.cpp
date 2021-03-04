#include <ConnectionManager.h>

#define DEBUG false
#define USE_SERIAL Serial

ConnectionManager *currentObject;

//callback notifying us of the need to save config
void ConnectionManager::saveConfigCallback() {
  if(DEBUG)
    USE_SERIAL.println("save config callback raised.");
  _shouldSaveConfig = true;
}

// static wrapper to call the callback method within a library.
static void callbackWrapper() {
  currentObject->saveConfigCallback();
}

/*
Constructor
*/
ConnectionManager::ConnectionManager() {
  currentObject = this;
}

void ConnectionManager::init() {
  wifiManager.setDebugOutput(DEBUG);
  //set config save notify callback
  wifiManager.setSaveConfigCallback(callbackWrapper);
  //set minimum quality of signal so it ignores AP's under that quality
  //defaults to 8%
  wifiManager.setMinimumSignalQuality(10);
  //sets timeout until configuration portal gets turned off useful to make
  //it all retry or go to sleep. In seconds:
  //wifiManager.setTimeout(120);
  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length

  char _mqtt_server[32] = "";
  char _mqtt_port[5] = "1883";
  char _mqtt_user[16] = "";
  char _mqtt_password[32] = "";
  char _actual_counter[6] = "";
  char _rotations_per_kwh[3] = "75";

  WiFiManagerParameter custom_mqtt_server("server", "mqtt server", _mqtt_server, 32);
  WiFiManagerParameter custom_mqtt_port("port", "mqtt port", _mqtt_port, 5);
  WiFiManagerParameter custom_mqtt_user("user", "mqtt user", _mqtt_user, 16);
  WiFiManagerParameter custom_mqtt_password("password", "mqtt password", _mqtt_password, 32);
  WiFiManagerParameter custom_rotations_per_kwh("rotations_per_kwh", "Rotations per kWh", _rotations_per_kwh, 3);
  WiFiManagerParameter custom_actual_counter("actual_counter", "Actual Counter in kWh", _actual_counter, 6);

  //add all your parameters here
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_mqtt_user);
  wifiManager.addParameter(&custom_mqtt_password);
  wifiManager.addParameter(&custom_rotations_per_kwh);
  wifiManager.addParameter(&custom_actual_counter);

  readConfiguration();

  String ssid = apSSIDPrefix + String(ESP.getChipId()).substring(0,22);
  if (!wifiManager.autoConnect(ssid.c_str())) {
    if(DEBUG)
      USE_SERIAL.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  if(DEBUG)
    USE_SERIAL.println("connected.");

  //save the custom parameters to FS
  if (_shouldSaveConfig) {
    
    //read updated parameters 
    strcpy(config.mqtt_server, custom_mqtt_server.getValue());
    config.mqtt_port         = atoi(custom_mqtt_port.getValue());
    strcpy(config.mqtt_user, custom_mqtt_user.getValue());
    strcpy(config.mqtt_password, custom_mqtt_password.getValue());
    config.rotations_per_kwh = atoi(custom_rotations_per_kwh.getValue());
    config.actual_counter    = atof(custom_actual_counter.getValue());
    
    writeConfiguration();
  }
}

void ConnectionManager::readConfiguration() {
  if (LittleFS.begin()) {
  if(DEBUG)
    USE_SERIAL.println("file system mounted.");

  if (LittleFS.exists(configFile)) {
    //file exists, reading and loading
    if(DEBUG)
      USE_SERIAL.println("reading config file");
    File fileHandle = LittleFS.open(configFile, "r");

    if (fileHandle) {
      if(DEBUG)
        USE_SERIAL.println("opened config file");
      size_t size = fileHandle.size();
      // Allocate a buffer to store contents of the file.
      std::unique_ptr<char[]> buf(new char[size]);

      fileHandle.readBytes(buf.get(), size);
      JSONVar jsonBuffer = JSON.parse(buf.get());

      if(DEBUG)
        USE_SERIAL.println(jsonBuffer);

      strcpy(config.mqtt_server, jsonBuffer["mqtt_server"]);
      config.mqtt_port         = (int) jsonBuffer["mqtt_port"];
      strcpy(config.mqtt_user, jsonBuffer["mqtt_user"]);
      strcpy(config.mqtt_password, jsonBuffer["mqtt_password"]);
      config.rotations_per_kwh = (int) jsonBuffer["rotations_per_kwh"];
      config.actual_counter    = (double) jsonBuffer["actual_counter"];
    }
    fileHandle.close();
  }
  LittleFS.end();
  } else {
   if(DEBUG)
       USE_SERIAL.println("failed to mount FS");
  }
}

void ConnectionManager::writeConfiguration() {
  JSONVar json;
   json["mqtt_server"] = config.mqtt_server;
   json["mqtt_port"] = config.mqtt_port;
   json["mqtt_user"] = config.mqtt_user;
   json["mqtt_password"] = config.mqtt_password;
   json["rotations_per_kwh"] = config.rotations_per_kwh;
   json["actual_counter"] = config.actual_counter;

   if (LittleFS.begin()) {
     if(DEBUG)
       USE_SERIAL.println(json);
     File fileHandle = LittleFS.open(configFile, "w");
     if (!fileHandle) {
       if(DEBUG)
         USE_SERIAL.println("failed to open config file for writing");
     }
     if(DEBUG) {
       USE_SERIAL.println("Writing JSON String:");
       json.printTo(USE_SERIAL);
       USE_SERIAL.println();
     }
     json.printTo(fileHandle);
     fileHandle.close();
     LittleFS.end();
   } else {
     if(DEBUG)
       USE_SERIAL.println("failed to mount FS");
   }
}

void ConnectionManager::resetConfiguration() {
  // format FileSystem to delete custom config
  LittleFS.format();
  // reset wifi settings
  wifiManager.resetSettings();
}