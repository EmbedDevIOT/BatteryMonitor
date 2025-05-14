#include <BatteryMonitor.h>

// Prescaller for ADC
#define R1 130000
#define R2 100000

// Battery voltage limits
#define BATTERY_VOLTAGE_MIN 5.4
#define BATTERY_VOLTAGE_MAX 6.9

BatteryMonitor battery(ADC1_CHANNEL_7, BATTERY_VOLTAGE_MIN, BATTERY_VOLTAGE_MAX, R1, R2);

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
