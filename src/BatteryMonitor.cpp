#include "BatteryMonitor.h"

#define NO_OF_SAMPLES 15

BatteryMonitor::BatteryMonitor(adc1_channel_t channel, float voltageMin, float voltageMax,
                               uint32_t r1, uint32_t r2, uint32_t vref)
    : _channel(channel), _vMin(voltageMin), _vMax(voltageMax), _vref(vref)
{
    _dividerRatio = (float)(r1 + r2) / r2;
}

/**
 * @brief Initializes the BatteryMonitor by configuring the ADC (Analog-to-Digital Converter) settings.
 * 
 * This function sets the ADC resolution to 12 bits, configures the attenuation for the specified ADC channel,
 * and characterizes the ADC calibration based on the provided reference voltage (_vref).
 * 
 * @note Ensure that the _channel and _vref member variables are properly initialized before calling this function.
 */
void BatteryMonitor::begin() {
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(_channel, ADC_ATTEN_DB_11);
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, _vref, &_adc_chars);
}

void BatteryMonitor::setLowBatteryThreshold(float threshold) {
    _lowBatteryThreshold = threshold;
}

/**
 * @brief Checks if the battery voltage is below the low battery threshold.
 * 
 * This function compares the current battery voltage with the predefined
 * low battery threshold to determine if the battery level is critically low.
 * 
 * @return true if the battery voltage is less than or equal to the low battery threshold, false otherwise.
 */
bool BatteryMonitor::isLowBattery() {
    return getBatteryVoltage() <= _lowBatteryThreshold;
}

/**
 * @brief Sets the filter mode for ADC readings.
 * 
 * This function allows selecting the filtering method (AVERAGE, MEDIAN, or EXPONENTIAL)
 * to process the raw ADC readings.
 * 
 * @param mode The desired filter mode.
 */
void BatteryMonitor::setFilterMode(FilterMode mode) {
    _filterMode = mode;
}

/**
 * @brief Sets the alpha value for the exponential moving average filter.
 * 
 * This function configures the smoothing factor for the exponential filter,
 * ensuring the value is constrained between 0.01 and 1.0.
 * 
 * @param alpha The alpha value for the exponential filter.
 */
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

/**
 * @brief Calculates the actual battery voltage using the voltage divider ratio.
 * 
 * This function multiplies the measured divider voltage by the divider ratio
 * to compute the actual battery voltage.
 * 
 * @return float The calculated battery voltage.
 */
float BatteryMonitor::getBatteryVoltage() {
    return getDividerVoltage() * _dividerRatio;
}

uint32_t BatteryMonitor::getAdcRaw() {
    return readAdcRawFiltered();
}

/**
 * @brief Calculates the battery percentage based on the current voltage.
 * 
 * This function determines the battery's charge level as a percentage by 
 * comparing the current voltage to the minimum (_vMin) and maximum (_vMax) 
 * voltage thresholds. If the voltage is less than or equal to _vMin, the 
 * function returns 0%. If the voltage is greater than or equal to _vMax, 
 * the function returns 100%. Otherwise, it calculates the percentage 
 * proportionally between _vMin and _vMax.
 * 
 * @return uint8_t The battery percentage (0-100).
 */
uint8_t BatteryMonitor::getBatteryPercent() {
    float voltage = getBatteryVoltage();
    if (voltage <= _vMin) return 0;
    if (voltage >= _vMax) return 100;

    float percent = (voltage - _vMin) / (_vMax - _vMin) * 100.0f;
    return (uint8_t)percent;
}
