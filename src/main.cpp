#include <Arduino.h>
#include <ConnectionManager.h> 
#include <MqttClient.h>
#include <MessageBuilder.h>
#include <DoubleResetDetect.h>
#include <NTPTime.h>
#include <DisplayManager.h>

// maximum number of seconds between resets that
// counts as a double reset 
#define DRD_TIMEOUT 2.0

// address to the block in the RTC user memory
// change it if it collides with another usage 
// of the address block
#define DRD_ADDRESS 0x00
DoubleResetDetect drd(DRD_TIMEOUT, DRD_ADDRESS);

#define DIGITALPIN D5
#define DEBUG true
#define USE_SERIAL Serial

ConnectionManager connectionManager;
MqttClient        mqttClient;
MessageBuilder    msgBuilder;
NTPTime           ntpTime;
DisplayManager    displayManager;

uint32_t previousTimestamp     = 0;
uint8_t  previousDay           = 0;
float    total_energy_kwh      = 0.0;
float    today_energy_kwh      = 0.0;
float    powerInWatts          = 0.0;
boolean  previousImpulse       = false;
boolean  timeIsTicking         = false;

uint16_t maxPayloadBufferSize  = MqttClient::getMaxPayloadBufferSize();
uint8_t  maxTopicNameSize      = MqttClient::getMaxTopicNameSize();

char     hostname[32];

char     energyTopicName[128];
char     energyConfigPayload[512];
char     powerTopicName[128];
char     powerConfigPayload[512];
char     stateTopicName[128];

void setup () {
  pinMode (DIGITALPIN, INPUT_PULLUP);

  strcat(hostname, "MRT-Power-Meter-");
  strcat(hostname, String(ESP.getChipId()).c_str());

  USE_SERIAL.begin (9600);
  
  // Activate to reset all wifimanager settings
  // connectionManager.resetConfiguration();
  if (drd.detect() && ESP.getResetInfoPtr()->reason == 6) {
    Serial.println("** Double reset boot **");
    connectionManager.resetConfiguration();
  }
  connectionManager.init();
  ntpTime.init();

  total_energy_kwh = connectionManager.config.actual_counter;

  if(DEBUG) {
    USE_SERIAL.print("MQTT Server:    "); USE_SERIAL.println(connectionManager.config.mqtt_server);
    USE_SERIAL.print("MQTT Port:      "); USE_SERIAL.println(connectionManager.config.mqtt_port);
    USE_SERIAL.print("MQTT User:      "); USE_SERIAL.println(connectionManager.config.mqtt_user);
    // USE_SERIAL.print("MQTT Password:  "); USE_SERIAL.println(connectionManager.config.mqtt_password);
    USE_SERIAL.print("Actual Counter: "); USE_SERIAL.println(connectionManager.config.actual_counter);
    USE_SERIAL.print("Rotations:      "); USE_SERIAL.println(connectionManager.config.rotations_per_kwh);
  }

  msgBuilder.getConfigTopicName(hostname, "energy", energyTopicName, maxTopicNameSize);
  msgBuilder.getEnergyConfigurationPayload(hostname, energyConfigPayload, maxPayloadBufferSize, maxTopicNameSize);
  msgBuilder.getConfigTopicName(hostname, "power", powerTopicName, maxTopicNameSize);
  msgBuilder.getPowerConfigurationPayload(hostname, powerConfigPayload, maxPayloadBufferSize, maxTopicNameSize);
  msgBuilder.getStateTopicName(hostname, stateTopicName, maxTopicNameSize);


/*
  if(DEBUG) {
    USE_SERIAL.print("Energy Topic: "); USE_SERIAL.println(energyTopicName);
    USE_SERIAL.println(energyConfigPayload);
    USE_SERIAL.print("Power Topic: "); USE_SERIAL.println(powerTopicName);
    USE_SERIAL.println(powerConfigPayload);
    USE_SERIAL.print("State Topic: "); USE_SERIAL.println(stateTopicName);
  }
*/

  mqttClient.init(hostname, connectionManager.config.mqtt_server, connectionManager.config.mqtt_port, 
        connectionManager.config.mqtt_user, connectionManager.config.mqtt_password);
  
  delay(3000); // give some time to initialize...
}

void publish() {

  if(!mqttClient.isConnected()) {
    if(!mqttClient.reconnect()) {
      if(DEBUG)
        USE_SERIAL.println("MQTT Connect failed.");
    } 
  }

  if(mqttClient.isConnected()) {
    char statePayload[maxPayloadBufferSize];
    msgBuilder.getStatePayload(hostname, powerInWatts, total_energy_kwh, today_energy_kwh, statePayload, maxPayloadBufferSize);

    mqttClient.publish(energyTopicName, energyConfigPayload, true);
    mqttClient.publish(powerTopicName, powerConfigPayload, true);
    mqttClient.publish(stateTopicName, statePayload, false);

    if(DEBUG) {
      USE_SERIAL.printf("Published %.5f watts and %.5f kWh.", powerInWatts, total_energy_kwh);
      USE_SERIAL.println();   
    }
  }
}

/**
 *  do the math...
 *  75  rotations == 1 kWh         
 *  75  rotations == 60 minutes*1000 watts (watt minutes)
 *  1   rotations == 60000/75 watt minutes
 *  1   rotations/minute == 800 watts
 *  general:
 *  power in watts = 3.600.000 / (75 * seconds for 1 rotation)
 **/
float getPowerInWatts(uint16_t seconds_for_rotation) {
  float _result = (float) 3600000 / (connectionManager.config.rotations_per_kwh * seconds_for_rotation);
  return (_result);
}

float getEnergyInKwhPerImpulse() {
  float _kWh = (float) 1/connectionManager.config.rotations_per_kwh;
  return (_kWh);
}

void loop () {
  ntpTime.loop();
  
  uint32_t timestamp = ntpTime.getTimestamp();
  uint8_t day_of_the_week = ntpTime.getWeekday();
  char date[11];
  ntpTime.getDateAsString(date);
  char time[9];
  ntpTime.getTimeAsString(time);

 /*
  * stores HIGH or LOW signal of the IR Sensor.
  * Logic is:
  * As long as the silver color of the rotary disc is recognized, the signal is HIGH,
  * if red (or a darker) color is detected, the signal turns to LOW.
  */
  boolean _hasImpulse = !digitalRead(DIGITALPIN);

  if (_hasImpulse) {
    if(DEBUG) 
      USE_SERIAL.println("Impulse detected.");

    if (!previousImpulse && timeIsTicking) {
      /*
        The previous impulse is false so we only get the first positive impulse in a sequence.
        With timeIsTicking we ensure that we get the first full round.
      */

      if(timestamp > previousTimestamp) {
        powerInWatts = getPowerInWatts(timestamp - previousTimestamp);
        float _energyInkWh  = getEnergyInKwhPerImpulse();

        total_energy_kwh += _energyInkWh;
        if (previousDay != day_of_the_week)
          today_energy_kwh = 0.0;
        today_energy_kwh += _energyInkWh;  

        publish();

        if(DEBUG) {
          USE_SERIAL.printf("%s - %s.", date, time);
          USE_SERIAL.println();
          USE_SERIAL.printf("The actual power usage is %.2f watts.", powerInWatts);
          USE_SERIAL.println();
          USE_SERIAL.printf("The todays/total energy consumption is %.2f/%.2f kWh.", 
                                                    today_energy_kwh, total_energy_kwh);
          USE_SERIAL.println();
        }
      }
    }

    if (previousTimestamp == 0) {
      if(DEBUG) 
        USE_SERIAL.println("First round.... activating calculation.");
      timeIsTicking = true;
    }
    // save timestamp
    previousTimestamp = timestamp;
    // save day of the week
    previousDay = day_of_the_week;

  } //end if (hasImpulse)
  
  displayManager.updateDisplay(powerInWatts, total_energy_kwh, today_energy_kwh, date, time);

  previousImpulse = _hasImpulse;
}