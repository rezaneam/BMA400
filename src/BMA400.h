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

#define BMA400_REG_AUTO_LOW_POW_0 0x2A
#define BMA400_REG_AUTO_LOW_POW_1 0x2B

#define BMA400_ADDRESS_PRIMARY 0x14
#define BMA400_ADDRESS_SECONDARY 0x15

#define BMA400_CHIP_ID 0x90

class BMA400
{
public:
    typedef enum
    {
        UNKNOWN_MODE,                       // Something most be wrong
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
        UNKNOWN_TIMEOUT,              // Something most be wrong
        DISABLE,              // Auto Low power timeout is disabled
        ON_TIMEOUT,           // Auto Low power timeout is reached
        ON_TIMEOUT_RST_G_INT2 // Auto Low Power ontime and also resets generic interrupt 2 asserted
    } auto_low_power_timeout_mode_t;

    typedef enum
    {
        UNKNOWN_RATE,
        Filter1_048x_800Hz,
        Filter1_024x_800Hz,
        Filter1_048x_400Hz,
        Filter1_024x_400Hz,
        Filter1_048x_200Hz,
        Filter1_024x_200Hz,
        Filter1_048x_100Hz,
        Filter1_024x_100Hz,
        Filter1_048x_50Hz,
        Filter1_024x_50Hz,
        Filter1_048x_25Hz,
        Filter1_024x_25Hz,
        Filter1_048x_12Hz,
        Filter1_024x_12Hz,
        Filter2_100Hz,
        Filter2_100Hz_LPF_1Hz
    } output_data_rate_t;

    typedef enum
    {
        UNKNOWN_RANGE,
        RANGE_2G,
        RANGE_4G,
        RANGE_8G,
        RANGE_16G
    } acceleation_range_t;

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

    //# MISC

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
