#include <MqttClient.h>

#define DEBUG false
#define USE_SERIAL Serial

WiFiClient _wifiClient;
PubSubClient _pubsub(_wifiClient);

uint8_t MqttClient::_max_topic_name_size{ 128 };
uint16_t  MqttClient::_max_payload_buffer_size{ 512 };

MqttClient::MqttClient() {

}

void MqttClient::init(const char* clientId, const char* server, uint16_t port, 
  const char* user, const char* password) {
  _clientId = clientId;
  _user = user;
  _password = password;
  _pubsub.setServer(server, port);
  _pubsub.setBufferSize(_max_payload_buffer_size);  
}

void MqttClient::setClientId(const char* clientId) {
  _clientId = clientId;
}

// function called when a MQTT message arrived
//void MqttClient::callback(char* p_topic, byte* p_payload, unsigned int p_length) {
//}

boolean MqttClient::reconnect() {
  
  if (!_pubsub.connected()) {
    if (_pubsub.connect(_clientId, _user, _password)) {
      if (DEBUG) {
        USE_SERIAL.println("MQTT: connected.");
      }
    } else {
      if (DEBUG) {
        USE_SERIAL.printf("MQTT: Connection failed. Maybe Wifi not ready... (rc=%i)", _pubsub.state());
        USE_SERIAL.println();
      }
    }
  }
  return _pubsub.connected();
}

void MqttClient::publish(const char* topic, const char* payload, boolean retain) {

  if(_pubsub.connected()) {
    if(!_pubsub.publish(topic, payload, retain)) {
      if (DEBUG) {
        USE_SERIAL.printf("MQTT: Publishing failed (rc=%i)", _pubsub.state());
        USE_SERIAL.println();
      }
    }
  } else {
      if (DEBUG) {
        USE_SERIAL.printf("MQTT: Publishing failed. No Connection. (rc=%i)", _pubsub.state());
        USE_SERIAL.println();
      }
  }
}

boolean MqttClient::isConnected() {
  return (_pubsub.connected());
}

void MqttClient::loop() {
  _pubsub.loop();
}

uint16_t MqttClient::getMaxPayloadBufferSize() {
  return(_max_payload_buffer_size);
}

uint8_t MqttClient::getMaxTopicNameSize() {
  return(_max_topic_name_size);
}