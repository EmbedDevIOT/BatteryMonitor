# BatteryMonitor

A battery voltage monitoring class for ESP32-based IoT devices. Supports ADC filtering, battery percentage conversion, and low-battery warning.

## Features

- Works with voltage dividers
- Supports multiple filter modes (average, median, exponential)
- Configurable low battery warning threshold
- Compatible with Arduino and PlatformIO

## Usage

```cpp
#include "BatteryMonitor.h"

BatteryMonitor battery(ADC1_CHANNEL_7, 5.4, 6.9, 130000, 100000);

void setup() {
  Serial.begin(115200);
  battery.begin();
  battery.setLowBatteryThreshold(5.5);
  battery.setFilterMode(EXPONENTIAL);
  battery.setExponentialAlpha(0.2);
}

void loop() {
  Serial.printf("Battery: %.2f V — %d%%
", battery.getBatteryVoltage(), battery.getBatteryPercent());
  delay(2000);
}
```
