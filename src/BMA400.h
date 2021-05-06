#pragma once
#include <Arduino.h>
#include <Wire.h>

#define BMA400_REG_CHIP_ID 0x00
#define BMA400_REG_STATUS 0x03
#define BMA400_REG_ACC_DATA 0x04

#define BMA400_REG_EVENT 0x0D
#define BMA400_REG_INT_STAT_0 0x0E
#define BMA400_REG_INT_STAT_1 0x0F
#define BMA400_REG_INT_STAT_2 0x10
#define BMA400_REG_TEMP_DATA 0x11

#define BMA400_REG_ACC_CONFIG_0 0x19
#define BMA400_REG_ACC_CONFIG_1 0x1A
#define BMA400_REG_ACC_CONFIG_2 0x1B
#define BMA400_REG_INT_CONFIG_0 0x1F
#define BMA400_REG_AUTO_LOW_POW_0 0x2A
#define BMA400_REG_AUTO_LOW_POW_1 0x2B
#define BMA400_REG_GEN_INT_1_CONFIG 0x3F
#define BMA400_REG_GEN_INT_2_CONFIG 0x4A
#define BMA400_ADDRESS_PRIMARY 0x14
#define BMA400_ADDRESS_SECONDARY 0x15

#define BMA400_CHIP_ID 0x90

class BMA400
{
public:
    typedef enum
    {
        UNKNOWN_MODE,                  // Something most be wrong
        SLEEP,                         // 0.2uA
        LOWEST_POWER_WITH_NOISE,       // 0.85uA
        ULTRA_LOW_POWER,               // 0.93 uA
        LOW_POWER,                     // 1.1uA
        LOW_POWER_LOW_NOISE,           // 1.35uA
        NORMAL_LOWER_POWER_WITH_NOISE, // 3.5uA
        NORMAL,                        // 5.8uA
        NORMAL_LOW_NOISE,              //9.5uA
        NORMAL_LOWEST_NOISE,           // 14.5uA
    } power_mode_t;

    typedef enum
    {
        UNKNOWN_TIMEOUT,      // Something most be wrong
        DISABLE,              // Auto Low power timeout is disabled
        ON_TIMEOUT,           // Auto Low power timeout is reached
        ON_TIMEOUT_RST_G_INT2 // Auto Low Power ontime and also resets generic interrupt 2 asserted
    } auto_low_power_timeout_mode_t;

    typedef enum
    {
        UNKNOWN_RATE,         // Something most be wrong
        Filter1_048x_800Hz,   // ODR 800Hz BW 384Hz
        Filter1_024x_800Hz,   // ODR 800Hz BW 192Hz
        Filter1_048x_400Hz,   // ODR 400Hz BW 192Hz
        Filter1_024x_400Hz,   // ODR 400Hz BW 96Hz
        Filter1_048x_200Hz,   // ODR 200Hz BW 96Hz
        Filter1_024x_200Hz,   // ODR 200Hz BW 48Hz
        Filter1_048x_100Hz,   // ODR 100Hz BW 48Hz
        Filter1_024x_100Hz,   // ODR 100Hz BW 24Hz
        Filter1_048x_50Hz,    // ODR 50Hz BW 24Hz
        Filter1_024x_50Hz,    // ODR 50Hz BW 12Hz
        Filter1_048x_25Hz,    // ODR 25Hz BW 12Hz
        Filter1_024x_25Hz,    // ODR 25Hz BW 6Hz
        Filter1_048x_12Hz,    // ODR 12.5Hz BW 6Hz
        Filter1_024x_12Hz,    // ODR 12.5Hz BW 3Hz
        Filter2_100Hz,        // Fixed ODR 100Hz
        Filter2_100Hz_LPF_1Hz //Fixed ODR 100Hz filter by a low pass filter (BW=1Hz)
    } output_data_rate_t;

    typedef enum
    {
        UNKNOWN_RANGE,
        RANGE_2G,
        RANGE_4G,
        RANGE_8G,
        RANGE_16G
    } acceleation_range_t;

    typedef enum
    {
        ADV_GENERIC_INTERRUPT_1, // Generic Interrupt for (in)activity detection
        ADV_GENERIC_INTERRUPT_2, //  Generic Interrupt for (in)activity detection
    } interrupt_source_t;

    typedef enum
    {
        AMP_0mg,  // 0 mg hysteresis amplitude
        AMP_24mg, // 24 mg hysteresis amplitude
        AMP_48mg, // 48 mg hysteresis amplitude
        AMP_96mg  // 96 mg hysteresis amplitude
    } generic_interrupt_hysteresis_amplitude_t;

    typedef enum
    {
        ACTIVITY_DETECTION,   // activity detection. referenced aceleration above threshold
        INACTIVITY_DETECTION, // inactivity detection. referenced aceleration below threshold
    } generic_interrupt_mode_t;

    typedef enum
    {
        ACC_FILT_1, // using accelerometer filter 1 as data source for the interrupt generator
        ACC_FILT_2, // (recommended) using accelerometer filter 2 as data source for the interrupt generator
    } generic_interrupt_data_source_t;

    typedef enum
    {
        MANUAL_UPDATE,                     // reference values are updated by the user manually
        ONETIME_UPDATE,                    // reference values are updated automatically after triggering the interrupt
        EVERYTIME_UPDATE_FROM_ACC_FILTx,   // reference values are updated automatically by the end of interrupt
        EVERYTIME_UPDATE_FROM_ACC_FILT_LP, // reference values are updated automatically after triggering the interrupt from acc_filt_low pass (1Hz)
    } generic_interrupt_reference_update_t;

    bool Initialize(TwoWire &_wire = Wire);
    bool Initialize(uint8_t _address, TwoWire &_wire = Wire);
    power_mode_t GetPowerMode();
    void SetPowerMode(const power_mode_t &mode);
    void ReadAcceleration(uint16_t *values);
    void ReadAcceleation(float *values);

    //# Auto Low Power Configuration
    bool GetAutoLowPowerOnDataReady();
    bool GetAutoLowPowerOnGenericInterrupt1();
    auto_low_power_timeout_mode_t GetAutoLowPowerOnTimeoutMode();
    float GetAutoLowPowerOnTimeoutThreshold();
    void SetAutoLowPowerOnDataReady(bool enable = true);
    void SetAutoLowPowerOnGenericInterrupt1(bool enable = true);
    void SetAutoLowPowerOnTimeout(auto_low_power_timeout_mode_t mode, float timeout_threshold);
    void SetAutoLowPower(bool onDataReady, bool onGenericInterrupt1, auto_low_power_timeout_mode_t mode, float timeout_threshold);

    //# Filter Configuration
    void SetDataRate(output_data_rate_t rate);
    output_data_rate_t GetDataRate();

    void SetRange(acceleation_range_t range);
    acceleation_range_t GetRange();

    //# Interrupts
    void SetGenericInterrupt(
        interrupt_source_t interrupt, bool enable,
        generic_interrupt_reference_update_t reference,
        generic_interrupt_mode_t mode,
        uint8_t threshold, uint16_t duration,
        generic_interrupt_hysteresis_amplitude_t hystersis,
        generic_interrupt_data_source_t data_source = generic_interrupt_data_source_t::ACC_FILT_2,
        bool enableX = true, bool enableY = true, bool enableZ = true,
        bool all_combined = false, bool ignoreSamplingRateFix = false);

    void SetGenericInterrupt(
        interrupt_source_t interrupt, bool enable,
        generic_interrupt_reference_update_t reference,
        generic_interrupt_mode_t mode,
        float threshold, float duration,
        generic_interrupt_hysteresis_amplitude_t hystersis,
        generic_interrupt_data_source_t data_source = generic_interrupt_data_source_t::ACC_FILT_2,
        bool enableX = true, bool enableY = true, bool enableZ = true,
        bool all_combined = false, bool ignoreSamplingRateFix = false);

    void SetGenericInterruptReference(interrupt_source_t interrupt, uint8_t *values);

private:
    uint8_t address;
    TwoWire *wire;

    void read(uint8_t _register, uint8_t length, uint8_t *values);
    uint8_t read(uint8_t _register);
    void write(uint8_t _register, const uint8_t &value);
    void write(uint8_t _register, const uint8_t &value, const uint8_t &mask);

    void set(uint8_t _register, const uint8_t &_bit);
    void unset(uint8_t _register, const uint8_t &_bit);
};
