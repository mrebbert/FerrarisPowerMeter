#ifndef MessageBuilder_h
#define MessageBuilder_h

#include "Arduino.h"
#include <Arduino_JSON.h>

class MessageBuilder {
  public:
    MessageBuilder();
    void getConfigTopicName(const char* node, const char* device_class, char* tName, uint8_t buff_size);
    void getStateTopicName(const char* node, char* tName, uint8_t buff_size);
    void getPowerConfigurationPayload(const char* node, char* payload, uint16_t buff_size, uint8_t tNSize);
    void getEnergyConfigurationPayload(const char* node, char* payload, uint16_t buff_size, uint8_t tNSize);
    void getStatePayload(const char* node, float powerInWatts, float total_energy_in_kwh, float today_energy_in_kwh, char* buf, uint8_t buff_size);

  private:
    void buildConfigrationPayload(const char* node, 
      const char* device_class, 
      const char* name, 
      const char* unit_of_measurement, 
      const char* value_template,
      char* payload,
      uint16_t buf_size,
      uint8_t tNSize);
};

#endif
