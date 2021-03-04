#include <Arduino.h>
#include <ConnectionManager.h> 
#include <MqttClient.h>
#include <MessageBuilder.h>

#define DIGITALPIN D5
#define RESET_PIN 1
#define DEBUG true
#define USE_SERIAL Serial

ConnectionManager connectionManager;
MqttClient        mqttClient;
MessageBuilder    msgBuilder;

uint32_t previousImpulseMillis = 0;
boolean  previousImpulse       = false;
boolean  timeIsTicking         = false;

//float    actual_counter        = (float) 0.0;

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
  
  //connectionManager.resetConfiguration();
  connectionManager.init();

  //initialize the power meter with the actual value
  // actual_counter = connectionManager.config.actual_counter;

  if(DEBUG) {
    USE_SERIAL.print("MQTT Server: ");
    USE_SERIAL.println(connectionManager.config.mqtt_server);
    USE_SERIAL.print("MQTT Port: ");
    USE_SERIAL.println(connectionManager.config.mqtt_port);
    USE_SERIAL.print("MQTT User: ");
    USE_SERIAL.println(connectionManager.config.mqtt_user);
    //USE_SERIAL.print("MQTT Password: ");
    //USE_SERIAL.println(connectionManager.config.mqtt_password);
    USE_SERIAL.print("Rotations per kWh: ");
    USE_SERIAL.println(connectionManager.config.rotations_per_kwh);
    USE_SERIAL.print("Actual Counter: ");
    USE_SERIAL.println(connectionManager.config.actual_counter);
  }

  msgBuilder.getConfigTopicName(hostname, "energy", energyTopicName, maxTopicNameSize);
  msgBuilder.getEnergyConfigurationPayload(hostname, energyConfigPayload, maxPayloadBufferSize, maxTopicNameSize);
  msgBuilder.getConfigTopicName(hostname, "power", powerTopicName, maxTopicNameSize);
  msgBuilder.getPowerConfigurationPayload(hostname, powerConfigPayload, maxPayloadBufferSize, maxTopicNameSize);
  msgBuilder.getStateTopicName(hostname, stateTopicName, maxTopicNameSize);

/*
  if(DEBUG) {
    USE_SERIAL.print("Energy Topic: ");
    USE_SERIAL.println(energyTopicName);
    USE_SERIAL.println(energyConfigPayload);
    USE_SERIAL.print("Power Topic: ");
    USE_SERIAL.println(powerTopicName);
    USE_SERIAL.println(powerConfigPayload);
    USE_SERIAL.print("State Topic: ");
    USE_SERIAL.println(stateTopicName);
  }
*/

  mqttClient.init(hostname, connectionManager.config.mqtt_server, connectionManager.config.mqtt_port, 
        connectionManager.config.mqtt_user, connectionManager.config.mqtt_password);
}

void publish(float watts, float kwh) {

  if(!mqttClient.isConnected()) {
    if(!mqttClient.reconnect()) {
      if(DEBUG)
        USE_SERIAL.println("MQTT Connect failed.");
    } 
  }

  if(mqttClient.isConnected()) {
    char statePayload[maxPayloadBufferSize];
    msgBuilder.getStatePayload(hostname, watts, kwh, statePayload, maxPayloadBufferSize);

    mqttClient.publish(energyTopicName, energyConfigPayload, true);
    mqttClient.publish(powerTopicName, powerConfigPayload, true);
    mqttClient.publish(stateTopicName, statePayload, false);

    if(DEBUG) {
      USE_SERIAL.printf("Published %.5f watts and %.5f kWh.", watts, connectionManager.config.actual_counter);
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
float getPowerInWatts(float seconds_for_rotation) {
  return (3600000 / (connectionManager.config.rotations_per_kwh * seconds_for_rotation));
}

float getEnergyInKwhPerImpulse() {
  float _kWh = (float) 1/connectionManager.config.rotations_per_kwh;
  return (_kWh);
}

void loop () {

  uint32_t _currentMillis = millis();
  boolean _isThresholdReached = digitalRead(DIGITALPIN);

  if (_isThresholdReached) {
    if(DEBUG) 
      USE_SERIAL.println("Threshold reached.");

    if (!previousImpulse && timeIsTicking) {
      /*
        The previous threshold is false so we only get the first positive impulse in a sequence.
        With timeIsTicking we ensure that we get the first full round.
      */

      if(_currentMillis > previousImpulseMillis) {
        /*
          if stmt needed to prevent misbehaviour due to time reset.
          see: https://www.arduino.cc/reference/en/language/functions/time/millis/
        */
        float _seconds                           = (float) (_currentMillis - previousImpulseMillis) / 1000;
        float _powerInWatts                      = getPowerInWatts(_seconds);
        connectionManager.config.actual_counter += getEnergyInKwhPerImpulse();
        publish(_powerInWatts, connectionManager.config.actual_counter);
        
        if(DEBUG) {
          USE_SERIAL.printf("The actual power consumption is %0.5f watts.", _powerInWatts);
          USE_SERIAL.println();
        }
      }
    }

    if (previousImpulseMillis == 0) {
      if(DEBUG) 
        USE_SERIAL.println("First round.... activating calculation.");
      timeIsTicking = true;
    }
    // save timestamp
    previousImpulseMillis = _currentMillis;

  } //end if (_isThresholdReached)

  previousImpulse = _isThresholdReached;
  delay(500);
}