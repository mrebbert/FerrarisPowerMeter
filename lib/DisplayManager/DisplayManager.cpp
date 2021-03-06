#include <DisplayManager.h>

#define DEBUG true
#define USE_SERIAL Serial

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

DisplayManager::DisplayManager() {
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
}

void DisplayManager::updateDisplay(float power, float total_energy, float today_energy) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Aktuelle Leistung:");
  display.setCursor(40, 11);
  display.printf("%.2f (W)", power);
  display.drawLine(0,21,128,21,WHITE);

  display.setCursor(0, 23);
  display.println("Gesamtverbrauch:");
  display.setCursor(40, 34);
  display.printf("%.2f (kWh)", total_energy);
  display.drawLine(0,43,128,43,WHITE);

  display.setCursor(0, 45);
  display.println("Heutiger Verbrauch:");
  display.setCursor(40, 56);
  display.printf("%.2f (kWh)", today_energy);

  display.display(); 
}