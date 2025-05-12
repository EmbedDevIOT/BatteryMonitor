#include <BatteryMonitor.h>

BatteryMonitor battery(ADC1_CHANNEL_7, 5.4, 6.9, 130000, 100000);

void setup() {
  Serial.begin(115200);
  battery.begin();
  battery.setLowBatteryThreshold(5.5);
  battery.setFilterMode(EXPONENTIAL);
  battery.setExponentialAlpha(0.2);
}

void loop() {
  float voltage = battery.getBatteryVoltage();
  uint8_t percent = battery.getBatteryPercent();

  Serial.printf("Battery: %.2f V — %d%%", voltage, percent);

  if (battery.isLowBattery()) {
    Serial.print(" [LOW BATTERY]");
  }

  Serial.println();
  delay(2000);
}
