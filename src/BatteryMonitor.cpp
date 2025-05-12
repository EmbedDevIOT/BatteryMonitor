#include "BatteryMonitor.h"

#define NO_OF_SAMPLES 15

BatteryMonitor::BatteryMonitor(adc1_channel_t channel, float voltageMin, float voltageMax,
                               uint32_t r1, uint32_t r2, uint32_t vref)
    : _channel(channel), _vMin(voltageMin), _vMax(voltageMax), _vref(vref)
{
    _dividerRatio = (float)(r1 + r2) / r2;
}

void BatteryMonitor::begin() {
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(_channel, ADC_ATTEN_DB_11);
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, _vref, &_adc_chars);
}

void BatteryMonitor::setLowBatteryThreshold(float threshold) {
    _lowBatteryThreshold = threshold;
}

bool BatteryMonitor::isLowBattery() {
    return getBatteryVoltage() <= _lowBatteryThreshold;
}

void BatteryMonitor::setFilterMode(FilterMode mode) {
    _filterMode = mode;
}

void BatteryMonitor::setExponentialAlpha(float alpha) {
    _emaAlpha = constrain(alpha, 0.01, 1.0);
}

uint32_t BatteryMonitor::readAdcRawFiltered() {
    std::vector<uint32_t> readings;

    for (int i = 0; i < NO_OF_SAMPLES; i++) {
        readings.push_back(adc1_get_raw(_channel));
        delayMicroseconds(200);
    }

    if (_filterMode == AVERAGE) {
        uint32_t sum = 0;
        for (uint32_t val : readings) sum += val;
        return sum / NO_OF_SAMPLES;

    } else if (_filterMode == MEDIAN) {
        std::sort(readings.begin(), readings.end());
        return readings[NO_OF_SAMPLES / 2];

    } else if (_filterMode == EXPONENTIAL) {
        for (uint32_t val : readings) {
            _emaState = _emaAlpha * val + (1 - _emaAlpha) * _emaState;
        }
        return (uint32_t)_emaState;
    }

    return 0;
}

uint32_t BatteryMonitor::readAdcMv() {
    uint32_t raw = readAdcRawFiltered();
    return esp_adc_cal_raw_to_voltage(raw, &_adc_chars);
}

float BatteryMonitor::getDividerVoltage() {
    return readAdcMv() / 1000.0f;
}

float BatteryMonitor::getBatteryVoltage() {
    return getDividerVoltage() * _dividerRatio;
}

uint32_t BatteryMonitor::getAdcRaw() {
    return readAdcRawFiltered();
}

uint8_t BatteryMonitor::getBatteryPercent() {
    float voltage = getBatteryVoltage();
    if (voltage <= _vMin) return 0;
    if (voltage >= _vMax) return 100;

    float percent = (voltage - _vMin) / (_vMax - _vMin) * 100.0f;
    return (uint8_t)percent;
}
