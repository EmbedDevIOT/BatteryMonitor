#ifndef BATTERY_MONITOR_H
#define BATTERY_MONITOR_H

#include <Arduino.h>
#include <driver/adc.h>
#include <esp_adc_cal.h>
#include <vector>
#include <algorithm>

enum FilterMode {
    AVERAGE,
    MEDIAN,
    EXPONENTIAL
};

class BatteryMonitor {
public:
    BatteryMonitor(adc1_channel_t channel, float voltageMin, float voltageMax,
                   uint32_t r1, uint32_t r2, uint32_t vref = 1100);

    void begin();

    uint8_t getBatteryPercent();
    float getBatteryVoltage();
    float getDividerVoltage();
    uint32_t getAdcRaw();

    void setLowBatteryThreshold(float threshold);
    bool isLowBattery();

    void setFilterMode(FilterMode mode);
    void setExponentialAlpha(float alpha);

private:
    adc1_channel_t _channel;
    float _vMin, _vMax;
    float _dividerRatio;
    uint32_t _vref;
    esp_adc_cal_characteristics_t _adc_chars;

    float _lowBatteryThreshold = 5.5;

    FilterMode _filterMode = AVERAGE;
    float _emaAlpha = 0.1;
    float _emaState = 0;

    uint32_t readAdcRawFiltered();
    uint32_t readAdcMv();
};

#endif
