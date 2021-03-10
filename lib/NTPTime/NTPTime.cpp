#include <NTPTime.h>

#define DEBUG true
#define USE_SERIAL Serial

NTPTime::NTPTime() {

}

void NTPTime::init() {
  configTime(MY_TZ, MY_NTP_SERVER); 
  loop();
}

uint32_t NTPTime::getTimestamp() {
  return (now);
}

void NTPTime::loop() {
  time(&now);                               // read the current time
  localtime_r(&now, &timestruct);           // update the structure tm with the current time
}

void NTPTime::getDateAsString(char* buf) {
  sprintf(buf, "%04d/%02d/%02d",  timestruct.tm_year+1900, timestruct.tm_mon+1, timestruct.tm_mday);
}

void NTPTime::getTimeAsString(char* buf) {
  sprintf(buf, "%02d:%02d:%02d", timestruct.tm_hour, timestruct.tm_min , timestruct.tm_sec);
}

uint8_t NTPTime::getWeekday() {
  // days since Sunday 0-6
  return (timestruct.tm_wday);
}

void NTPTime::showTime() {
  Serial.print("year:");
  Serial.print(timestruct.tm_year + 1900);  // years since 1900
  Serial.print("\tmonth:");
  Serial.print(timestruct.tm_mon + 1);      // January = 0 (!)
  Serial.print("\tday:");
  Serial.print(timestruct.tm_mday);         // day of month
  Serial.print("\thour:");
  Serial.print(timestruct.tm_hour);         // hours since midnight  0-23
  Serial.print("\tmin:");
  Serial.print(timestruct.tm_min);          // minutes after the hour  0-59
  Serial.print("\tsec:");
  Serial.print(timestruct.tm_sec);          // seconds after the minute  0-61*
  Serial.print("\twday");
  Serial.print(timestruct.tm_wday);         // days since Sunday 0-6
  if (timestruct.tm_isdst == 1)             // Daylight Saving Time flag
    Serial.print("\tDST");
  else
    Serial.print("\tstandard");
  Serial.println();
}
