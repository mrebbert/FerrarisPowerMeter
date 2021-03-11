#ifndef NTPTime_h
#define NTPTime_h

#include <Arduino.h>
#include <time.h> // time() ctime()

/* Configuration of NTP */
#define MY_NTP_SERVER "de.pool.ntp.org"

#define MY_TZ "CET-1CEST,M3.5.0,M10.5.0/3" //https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv

class NTPTime {

  public:
    NTPTime();
    void init();
    uint32_t getTimestamp();
    void getDateAsString(char* buf);
    void getTimeAsString(char* buf);
    uint8_t getWeekday();
    void loop();
    void showTime();
  private:
    time_t now;                         // this is the epoch
    tm timestruct;                      // the structure tm holds time information in a more convient way
};

#endif