#include <MessageBuilder.h>

MessageBuilder::MessageBuilder() {
}

void MessageBuilder::buildConfigrationPayload(const char* node, 
  const char* device_class, 
  const char* name, 
  const char* unit_of_measurement, 
  const char* value_template,
  char* payload,
  uint16_t buf_size,
  uint8_t tNSize) {

  char stateTopicName[tNSize];   
  getStateTopicName(node, stateTopicName, tNSize);

  JSONVar jpayload;
  jpayload["device_class"] = device_class;
  jpayload["name"] = name;
  jpayload["state_topic"] = stateTopicName;
  jpayload["json_attributes_topic"] = stateTopicName;
  jpayload["unique_id"] = String(name + String("-ESP"));
  jpayload["unit_of_measurement"] = unit_of_measurement;
  jpayload["value_template"] = value_template;
  
  strcpy(payload, JSONVar::stringify(jpayload).c_str());
}

void MessageBuilder::getStatePayload(const char* node, float watts, float kwh, char* buf, uint8_t buf_size) {
  
  JSONVar statePayload;
  statePayload["node"] = node;
  statePayload["energy"] = kwh;
  statePayload["power"] = watts;
  
  strcpy(buf, JSONVar::stringify(statePayload).c_str());
} 

void MessageBuilder::getConfigTopicName(const char* node, const char* device_class, char* tName, uint8_t buff_size) {
  strcpy(tName,"homeassistant/sensor/");
  strcat(tName, node);
  strcat(tName, "/");
  strcat(tName, device_class);
  strcat(tName, "/config");
  
}

void MessageBuilder::getStateTopicName(const char* node, char* tName, uint8_t buff_size) {
  strcpy(tName,"homeassistant/sensor/");
  strcat(tName, node);
  strcat(tName, "/state");
}

void MessageBuilder::getPowerConfigurationPayload(const char* node, char* payload, uint16_t buff_size, uint8_t tNSize) {
  char name[64];
  strcpy(name, node);
  strcat(name, "-Power");

  buildConfigrationPayload(node, "power", name, "W", "{{value_json.power}}", payload, buff_size, tNSize);
}

void MessageBuilder::getEnergyConfigurationPayload(const char* node, char* payload, uint16_t buff_size, uint8_t tNSize) {
  char name[64];
  strcpy(name, node);
  strcat(name, "-Energy");

  buildConfigrationPayload(node, "energy", name, "kWh", "{{value_json.energy}}", payload, buff_size, tNSize);
}