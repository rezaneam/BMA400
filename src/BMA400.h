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
#define BMA400_REG_STEP_CNT0 0x15
#define BMA400_REG_ACC_CONFIG_0 0x19
#define BMA400_REG_ACC_CONFIG_1 0x1A
#define BMA400_REG_ACC_CONFIG_2 0x1B
#define BMA400_REG_INT_CONFIG_0 0x1F
#define BMA400_REG_INT1_MAP 0x21
#define BMA400_REG_INT2_MAP 0x22
#define BMA400_REG_INT12_MAP 0x23
#define BMA400_REG_INT_CONFIG_1 0x20
#define BMA400_REG_AUTO_LOW_POW_0 0x2A
#define BMA400_REG_AUTO_LOW_POW_1 0x2B
#define BMA400_REG_ORIENT_CONFIG_0 0x35
#define BMA400_REG_ORIENT_CONFIG_1 0x36
#define BMA400_REG_ORIENT_CONFIG_3 0x38
#define BMA400_REG_ORIENT_CONFIG_4 0x38
#define BMA400_REG_GEN_INT_1_CONFIG 0x3F
#define BMA400_REG_GEN_INT_2_CONFIG 0x4A
#define BMA400_REG_ACT_CHNG_INT_CONFIG_0 0x55
#define BMA400_REG_ACT_CHNG_INT_CONFIG_1 0x56
#define BMA400_REG_TAP_CONFIG_0 0x57
#define BMA400_REG_TAP_CONFIG_1 0x58
#define BMA400_REG_COMMAND 0x7E

#define BMA400_ADDRESS_PRIMARY 0x14
#define BMA400_ADDRESS_SECONDARY 0x15

#define BMA400_CHIP_ID 0x90

class BMA400
{
public:
    typedef enum // power modes (includes noise rate as well)
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

    typedef enum // Timeout modes for auto low power purpose
    {
        UNKNOWN_TIMEOUT,      // Something most be wrong
        DISABLE,              // Auto Low power timeout is disabled
        ON_TIMEOUT,           // Auto Low power timeout is reached
        ON_TIMEOUT_RST_G_INT2 // Auto Low Power ontime and also resets generic interrupt 2 asserted
    } auto_low_power_timeout_mode_t;

    typedef enum // accelerometer data rate (includes the Bandwidth and data source as well)
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

    typedef enum // accelerometer range
    {
        UNKNOWN_RANGE,
        RANGE_2G,
        RANGE_4G,
        RANGE_8G,
        RANGE_16G
    } acceleation_range_t;

    typedef enum // All interrupt sources
    {
        BAS_DATA_READY = 0x0001,                       // Data Ready
        BAS_FIFO_WATERMARK = 0x0002,                   // FIFO Watermark
        BAS_FIFO_FULL = 0x0004,                        // FIFI Full
        BAS_ENGINE_OVERRUN = 0x0008,                   // Engine Overrun - interrupt calculation couldn't be finished
        BAS_WAKEUP = 0x0010,                           // Weke up
        ADV_GENERIC_INTERRUPT_1 = 0x0020,              // Generic Interrupt for (in)activity detection
        ADV_GENERIC_INTERRUPT_2 = 0x0040,              //  Generic Interrupt for (in)activity detection
        ADV_SET_DETECTOR_COUNTER = 0x0080,             // Step Detector interrupt / Step Counter
        ADV_SET_DETECTOR_COUNTER_DOUBLE_STEP = 0x0100, // Step Detector interrupt / Step Counter (most likely double step)
        ADV_ACTIVITY_CHANGE = 0x0200,                  // Activity Change interrupt
        ADV_SINGLE_TAP = 0x0400,                       // Single tap is detected
        ADV_DOUBLE_TAP = 0x0800,                       // Double tap is detected
        ADV_ORIENTATION_CHANGE = 0x1000,               // Orientation is changed
        ADV_ORIENTATION_CHANGE_X = 0x2000,               // Orientation is changed
        ADV_ORIENTATION_CHANGE_Y = 0x4000,               // Orientation is changed
        ADV_ORIENTATION_CHANGE_Z = 0x8000,               // Orientation is changed
    } interrupt_source_t;

    typedef enum // Interrupt Pins
    {
        INT_NONE,    // No Link
        INT_PIN_1,   // Interrupt Pin 1
        INT_PIN_2,   // Interrupt Pin 2
        INT_PIN_BOTH // Wire to both
    } interrupt_pin_t;

    typedef enum // hysteresis amplitude for generic interrupt 1/2
    {
        AMP_0mg,  // 0 mg hysteresis amplitude
        AMP_24mg, // 24 mg hysteresis amplitude
        AMP_48mg, // 48 mg hysteresis amplitude
        AMP_96mg  // 96 mg hysteresis amplitude
    } generic_interrupt_hysteresis_amplitude_t;

    typedef enum // Number of observations needed for triggering activity change interrupt
    {
        OBSERVATION_32,
        OBSERVATION_64,
        OBSERVATION_128,
        OBSERVATION_256,
        OBSERVATION_512
    } activity_change_observation_number_t;

    typedef enum // Modes for generic interrupt 1/2
    {
        ACTIVITY_DETECTION,   // activity detection. referenced aceleration above threshold
        INACTIVITY_DETECTION, // inactivity detection. referenced aceleration below threshold
    } generic_interrupt_mode_t;

    typedef enum // data sources used for interrupts
    {
        ACC_FILT_1, // using accelerometer filter 1 as data source for the interrupt generator
        ACC_FILT_2, // (recommended) using accelerometer filter 2 as data source for the interrupt generator
    } interrupt_data_source_t;

    typedef enum // data sources used for orientation change Interrupt
    {
        ORIENT_UPDATE_ACC_FILT_2_100HZ,        // using accelerometer filter 2
        ORIENT_UPDATE_ACC_FILT_2_100HZ_LP_1HZ, // using accelerometer filter 2 Low pass filtered with 1Hz cut-off frequency
    } orientation_change_data_source_t;

    typedef enum // data sources used for orientation change Interrupt reference
    {
        ORIENT_UPDATE_MANUAL,                       // Reference values are updated manually by user
        ORIENT_UPDATE_AUTO_ACC_FILT_2_100HZ,        // using accelerometer filter 2
        ORIENT_UPDATE_AUTO_ACC_FILT_2_100HZ_LP_1HZ, // using accelerometer filter 2 Low pass filtered with 1Hz cut-off frequency
    } orientation_reference_update_data_source_t;

    typedef enum // update mode for generic interrupt 1/2
    {
        MANUAL_UPDATE,                     // reference values are updated by the user manually
        ONETIME_UPDATE,                    // reference values are updated automatically after triggering the interrupt
        EVERYTIME_UPDATE_FROM_ACC_FILTx,   // reference values are updated automatically by the end of interrupt
        EVERYTIME_UPDATE_FROM_ACC_FILT_LP, // reference values are updated automatically after triggering the interrupt from acc_filt_low pass (1Hz)
    } generic_interrupt_reference_update_t;

    typedef enum // Commands for BMA400
    {
        CMD_FIFO_FLUSH = 0xB0,     // Clears all data in FIFO
        CMD_RESET_STEP_CNT = 0xB1, // Resets the step counter to 0
        CMD_SOFT_RESET = 0xB6      // Resets the chip and overwrites all user configurations

    } command_t;

    typedef enum // Tap detection axis
    {
        TAP_X_AXIS, // tap detection on X axis
        TAP_Y_AXIS, // tap detection on Y axis
        TAP_Z_AXIS  // tap detection on Z axis

    } tap_axis_t;

    typedef enum // Sensitivity levels for detection of tap. Range from 7 (highest sensitivity) to 0 (lowest sensitivity)
    {
        TAP_SENSITIVITY_7 = 0x00,
        TAP_SENSITIVITY_6 = 0x01,
        TAP_SENSITIVITY_5 = 0x02,
        TAP_SENSITIVITY_4 = 0x03,
        TAP_SENSITIVITY_3 = 0x04,
        TAP_SENSITIVITY_2 = 0x05,
        TAP_SENSITIVITY_1 = 0x06,
        TAP_SENSITIVITY_0 = 0x07,
    } tap_sensitivity_level_t;

    typedef enum // Maximum time between upper and lower peak of valid taps (in data samples) - default 12 samples
    {
        TAP_MAX_6_SAMPLES = 0x00,  // 6 data samples for high-low (pick to pick) tap signal change time
        TAP_MAX_9_SAMPLES = 0x01,  // 9 data samples for high-low (pick to pick) tap signal change time
        TAP_MAX_12_SAMPLES = 0x02, // 12 data samples for high-low (pick to pick) tap signal change time
        TAP_MAX_18_SAMPLES = 0x03, // 18 data samples for high-low (pick to pick) tap signal change time

    } tap_max_pick_to_pick_interval_t;

    typedef enum // Minimum quiet time (no tap) between two consecutive taps (in data samples) - default 80 samples
    {
        MIN_QUIET_60_SAMPLES,  // 60 data samples quiet tie between single or double taps
        MIN_QUIET_80_SAMPLES,  // 80 data samples quiet tie between single or double taps
        MIN_QUIET_100_SAMPLES, // 100 data samples quiet tie between single or double taps
        MIN_QUIET_120_SAMPLES, // 120 data samples quiet tie between single or double taps

    } tap_min_quiet_between_taps_t;

    typedef enum // Mininum time between two taps in a double tap (in data samples) - default 4 samples
    {
        MIN_QUIET_DT_4_SAMPLES,  // 4 data samples minimum time between double taps
        MIN_QUIET_DT_8_SAMPLES,  // 8 data samples minimum time between double taps
        MIN_QUIET_DT_12_SAMPLES, // 12 data samples minimum time between double taps
        MIN_QUIET_DT_16_SAMPLES, // 16 data samples minimum time between double taps

    } tap_min_quiet_inside_double_taps_t;

    bool Initialize(TwoWire &_wire = Wire);
    bool Initialize(uint8_t _address, TwoWire &_wire = Wire);
    power_mode_t GetPowerMode();
    void SetPowerMode(const power_mode_t &mode);
    void ReadAcceleration(uint16_t *values);
    void ReadAcceleation(float *values);
    bool ExecuteCommand(command_t cmd);

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
    interrupt_source_t GetInterrupts();
    bool HasInterrupt(interrupt_source_t source);

    void ConfigureInterruptPin(interrupt_source_t interrupt, interrupt_pin_t pin);

    void SetGenericInterrupt(
        interrupt_source_t interrupt, bool enable,
        interrupt_pin_t pin,
        generic_interrupt_reference_update_t reference,
        generic_interrupt_mode_t mode,
        uint8_t threshold, uint16_t duration,
        generic_interrupt_hysteresis_amplitude_t hystersis,
        interrupt_data_source_t data_source = interrupt_data_source_t::ACC_FILT_2,
        bool enableX = true, bool enableY = true, bool enableZ = true,
        bool all_combined = false, bool ignoreSamplingRateFix = false);

    void SetGenericInterrupt(
        interrupt_source_t interrupt, bool enable,
        interrupt_pin_t pin,
        generic_interrupt_reference_update_t reference,
        generic_interrupt_mode_t mode,
        float threshold, float duration,
        generic_interrupt_hysteresis_amplitude_t hystersis,
        interrupt_data_source_t data_source = interrupt_data_source_t::ACC_FILT_2,
        bool enableX = true, bool enableY = true, bool enableZ = true,
        bool all_combined = false, bool ignoreSamplingRateFix = false);

    void SetGenericInterruptReference(interrupt_source_t interrupt, uint8_t *values);
    void SetGenericInterruptReference(interrupt_source_t interrupt);

    void SetStepDetectorCounter(bool enable, interrupt_pin_t pin); //TODO: Support for Sensor Position (wrist/none wrist)
    uint32_t GetTotalSteps();
    bool ResetStepCounter();

    void SetActivityChangeInterrupt(bool enable,
                                    interrupt_pin_t pin,
                                    uint8_t threshold,
                                    activity_change_observation_number_t observation_number,
                                    interrupt_data_source_t data_source = interrupt_data_source_t::ACC_FILT_2,
                                    bool enableX = true, bool enableY = true, bool enableZ = true);

    void SetActivityChangeInterrupt(bool enable,
                                    interrupt_pin_t pin,
                                    float threshold,
                                    activity_change_observation_number_t observation_number,
                                    interrupt_data_source_t data_source = interrupt_data_source_t::ACC_FILT_2,
                                    bool enableX = true, bool enableY = true, bool enableZ = true);

    void SetTapInterrupt(
        bool enableSingleTap, bool enableDoubleTap,
        tap_axis_t axis,
        interrupt_pin_t pin,
        tap_sensitivity_level_t sensitivity = tap_sensitivity_level_t::TAP_SENSITIVITY_0,
        tap_max_pick_to_pick_interval_t pick_to_pick_interval = tap_max_pick_to_pick_interval_t::TAP_MAX_12_SAMPLES,
        tap_min_quiet_between_taps_t quiet_interval = tap_min_quiet_between_taps_t::MIN_QUIET_80_SAMPLES,
        tap_min_quiet_inside_double_taps_t double_taps_time = tap_min_quiet_inside_double_taps_t::MIN_QUIET_DT_4_SAMPLES);

    void SetOrientationChangeInterrupt(
        bool enable,
        bool enableX, bool enableY, bool enableZ,
        interrupt_pin_t pin,
        orientation_change_data_source_t source,
        orientation_reference_update_data_source_t reference_update_mode,
        uint8_t threshold,
        uint8_t duration);

    void SetOrientationChangeInterrupt(
        bool enable,
        bool enableX, bool enableY, bool enableZ,
        interrupt_pin_t pin,
        orientation_change_data_source_t source,
        orientation_reference_update_data_source_t reference_update_mode,
        float_t threshold,
        float duration);

    void SetOrientationReference(uint8_t *values);
    void SetOrientationReference();

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
