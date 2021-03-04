#ifndef MqttClient_h
#define MqttClient_h

#include <stdint.h>
#include "Arduino.h"
#include <PubSubClient.h>
#include <WiFiClient.h>

class MqttClient {
  public:
    MqttClient();
    void init(const char* clientId, const char* server, uint16_t port, const char* user, const char* password);
    void setClientId(const char* clientId);
    void loop();
    //void callback(char* p_topic, byte* p_payload, unsigned int p_length);
    boolean reconnect();
    boolean isConnected();
    void publish(const char* topic, const char* payload, boolean retain);
    int unsigned short getBufferSize();
    static uint16_t getMaxPayloadBufferSize();
    static uint8_t getMaxTopicNameSize();

  private:
    const char* _clientId;
    const char* _user;
    const char* _password;
    static uint16_t _max_payload_buffer_size;
    static uint8_t _max_topic_name_size;
};

#endif