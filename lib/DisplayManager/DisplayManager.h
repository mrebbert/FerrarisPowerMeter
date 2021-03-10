#ifndef DisplayManager_h
#define DisplayManager_h

#include <Wire.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class DisplayManager {
  public:
    DisplayManager();
    void updateDisplay(float power, float total_energy, float today_energy);
    void updateDisplay(float power, float total_energy, float today_energy, const char* date, const char* time);
    void updateDisplay(float power, float total_energy, float today_energy, const char* date, const char* time, float temperature);
  private:
};

#endif