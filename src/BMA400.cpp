#include <BMA400.h>

/*!
 *  @brief  Initializing the libary with auto address detect
 *  @param  _wire TwoWire interface - defalt Wire
 *  @return true if any BMA400 sensor found
 */
bool BMA400::Initialize(TwoWire &_wire)
{
    wire = &_wire;
    address = BMA400_ADDRESS_PRIMARY;

    if (read(BMA400_REG_CHIP_ID) == BMA400_CHIP_ID)
        return true;
    address = BMA400_ADDRESS_SECONDARY;

    return (read(BMA400_REG_CHIP_ID) == BMA400_CHIP_ID);
}

/*!
 *  @brief  Initializing the libary using sensor address
 *  @param  _address sensor address
 *  @param  _wire TwoWire interface - defalt Wire
 *  @return true if sensor found
 */
bool BMA400::Initialize(uint8_t _address, TwoWire &_wire)
{
    wire = &_wire;
    address = _address;
    return (read(BMA400_REG_CHIP_ID) == BMA400_CHIP_ID);
}

/*!
 *  @brief  Runs command
 *  @param  cmd check command_t for more details
 *  @return true if command is sent successfully
 */
bool BMA400::ExecuteCommand(command_t cmd)
{
    if ((read(BMA400_REG_STATUS) & 0x10) != 0x10)
        return false;

    write(BMA400_REG_COMMAND, cmd);
    return true;
}

/*!
 *  @brief  Getting current power mode (9 modes) includes Sleep/Low/Normal and 4 level of noise performance
 *  @return power mode. see power_mode_t for more details
 */
BMA400::power_mode_t BMA400::GetPowerMode()
{

    switch (read(BMA400_REG_ACC_CONFIG_0) & 0x03)
    {
    case 0:
    case 3:
        return power_mode_t::SLEEP;

    case 1: //# Low Power
        switch ((read(BMA400_REG_ACC_CONFIG_0) >> 5) & 0x03)
        {
        case 0:
            return power_mode_t::LOWEST_POWER_WITH_NOISE;
        case 1:
            return power_mode_t::ULTRA_LOW_POWER;
        case 2:
            return power_mode_t::LOW_POWER;
        case 3:
            return power_mode_t::LOW_POWER_LOW_NOISE;
        default:
            return power_mode_t::UNKNOWN_MODE;
        }
        break;

    case 2:
        switch ((read(BMA400_REG_ACC_CONFIG_1) >> 4) & 0x03)
        {
        case 0:
            return power_mode_t::NORMAL_LOWER_POWER_WITH_NOISE;
        case 1:
            return power_mode_t::NORMAL;
        case 2:
            return power_mode_t::NORMAL_LOW_NOISE;
        case 3:
            return power_mode_t::NORMAL_LOWEST_NOISE;

        default:
            return power_mode_t::UNKNOWN_MODE;
        }
        break;

    default:
        return power_mode_t::UNKNOWN_MODE;
    }
    return power_mode_t::UNKNOWN_MODE;
}

/*!
 *  @brief  Updating the power mode 
 *  @param  mode power mode see power_mode_t for more details
 */
void BMA400::SetPowerMode(const power_mode_t &mode)
{
    uint8_t config = read(BMA400_REG_ACC_CONFIG_0);

    switch (mode)
    {
    case power_mode_t::SLEEP:
        write(BMA400_REG_ACC_CONFIG_0, config & 0xFC);
        return;
    case power_mode_t::LOWEST_POWER_WITH_NOISE:
        config = (config & 0xFC) | 0x01;
        config &= 0x9F;
        write(BMA400_REG_ACC_CONFIG_0, config);
        return;

    case power_mode_t::ULTRA_LOW_POWER:
        config = (config & 0xFC) | 0x01;
        config = (config & 0x9F) | 0x20;
        write(BMA400_REG_ACC_CONFIG_0, config);
        return;

    case power_mode_t::LOW_POWER:
        config = (config & 0xFC) | 0x01;
        config = (config & 0x9F) | 0x40;
        write(BMA400_REG_ACC_CONFIG_0, config);
        return;

    case power_mode_t::LOW_POWER_LOW_NOISE:
        config = (config & 0xFC) | 0x01;
        config = (config & 0x9F) | 0x60;
        write(BMA400_REG_ACC_CONFIG_0, config);
        return;

    case power_mode_t::NORMAL_LOWER_POWER_WITH_NOISE:
        config = (config & 0xFC) | 0x01;
        write(BMA400_REG_ACC_CONFIG_0, config);
        config = read(BMA400_REG_ACC_CONFIG_1);
        config = config & 0xCF;
        write(BMA400_REG_ACC_CONFIG_1, config);
        return;

    case power_mode_t::NORMAL:
        config = (config & 0xFC) | 0x01;
        write(BMA400_REG_ACC_CONFIG_0, config);
        config = read(BMA400_REG_ACC_CONFIG_1);
        config = (config & 0xCF) | 0x10;
        write(BMA400_REG_ACC_CONFIG_1, config);
        return;

    case power_mode_t::NORMAL_LOW_NOISE:
        config = (config & 0xFC) | 0x01;
        write(BMA400_REG_ACC_CONFIG_0, config);
        config = read(BMA400_REG_ACC_CONFIG_1);
        config = (config & 0xCF) | 0x20;
        write(BMA400_REG_ACC_CONFIG_1, config);
        return;

    case power_mode_t::NORMAL_LOWEST_NOISE:
        config = (config & 0xFC) | 0x01;
        write(BMA400_REG_ACC_CONFIG_0, config);
        config = read(BMA400_REG_ACC_CONFIG_1);
        config = (config & 0xCF) | 0x30;
        write(BMA400_REG_ACC_CONFIG_1, config);
        return;
    }
}

/*!
 *  @brief  Getting Acceleration - unprocessed 
 *  @param  values must be address of an array (uint16_t) with at least 3 elements 
 */
void BMA400::ReadAcceleration(uint16_t *values)
{
    uint8_t data[6];
    read(BMA400_REG_ACC_DATA, 6, data);

    for (uint8_t i = 0; i < 3; i++)
    {
        values[i] = data[0 + i * 2] + data[1 + i * 2];
        if (values[i] > 2047)
            values[i] -= 4096;
    }
}

/*!
 *  @brief  Getting Acceleration - processed in mg
 *  @param  values must be address of an array (float) with at least 3 elements 
 */
void BMA400::ReadAcceleation(float *values)
{
    uint8_t data[6];
    float divider;
    read(BMA400_REG_ACC_DATA, 6, data);

    switch (GetRange())
    {
    case acceleation_range_t::RANGE_2G:
        divider = 1024;
        break;

    case acceleation_range_t::RANGE_4G:
        divider = 512;
        break;

    case acceleation_range_t::RANGE_8G:
        divider = 256;
        break;

    case acceleation_range_t::RANGE_16G:
        divider = 128;
        break;
    }

    for (uint8_t i = 0; i < 3; i++)
    {
        values[i] = data[0 + i * 2] + data[1 + i * 2];
        if (values[i] > 2047)
            values[i] -= 4096;
        values[i] /= divider;
    }
}

/*!
 *  @brief  Checking if Auto Low Power on Data Ready is enabled
 *  @return true if Auto low power on Data Ready is enabled
 */
bool BMA400::GetAutoLowPowerOnDataReady()
{
    return (read(BMA400_REG_AUTO_LOW_POW_1) & 0x01) == 1;
}

/*!
 *  @brief  Checking if Auto Low Power on Generic Interrupt 1 is enabled
 *  @return true if Auto low power on Generic Interrupt 1 is enabled
 */
bool BMA400::GetAutoLowPowerOnGenericInterrupt1()
{
    return (read(BMA400_REG_AUTO_LOW_POW_1) & 0x02) == 2;
}

/*!
 *  @brief  Checking if Auto Low Power on timeout mode
 *  @return timeout mode. check auto_low_power_timeout_mode_t for more detail
 */
BMA400::auto_low_power_timeout_mode_t BMA400::GetAutoLowPowerOnTimeoutMode()
{
    uint8_t val = read(BMA400_REG_AUTO_LOW_POW_1) & 0x0C;
    switch (val)
    {
    case 0x00:
        return auto_low_power_timeout_mode_t::DISABLE;

    case 0x04:
    case 0x0C:
        return auto_low_power_timeout_mode_t::ON_TIMEOUT;

    case 0x08:
        return auto_low_power_timeout_mode_t::ON_TIMEOUT_RST_G_INT2;

    default:
        return auto_low_power_timeout_mode_t::UNKNOWN_TIMEOUT;
    }
}

/*!
 *  @brief  Getting Auto Low Power on Timeout threshold (time) in ms
 *  @return threshold in ms scale
 */
float BMA400::GetAutoLowPowerOnTimeoutThreshold()
{
    float threshold;
    threshold = (read(BMA400_REG_AUTO_LOW_POW_1) & 0xF0) >> 4;
    threshold += read(BMA400_REG_AUTO_LOW_POW_0) << 4;
    threshold *= 2.5;
    return threshold;
}

/*!
 *  @brief  Updating Auto Low Power On Data Ready
 *  @param  enable true will enable the auto low power
 */
void BMA400::SetAutoLowPowerOnDataReady(bool enable)
{
    uint8_t val = read(BMA400_REG_AUTO_LOW_POW_1) & 0xFE;
    if (enable)
        val |= 0x01;
    write(BMA400_REG_AUTO_LOW_POW_1, val);
}

/*!
 *  @brief  Updating Auto Low Power On Generic Interrupt 1
 *  @param  enable true will enable the auto low power
 */
void BMA400::SetAutoLowPowerOnGenericInterrupt1(bool enable)
{
    uint8_t val = read(BMA400_REG_AUTO_LOW_POW_1) & 0xFD;
    if (enable)
        val |= 0x02;
    write(BMA400_REG_AUTO_LOW_POW_1, val);
}

/*!
 *  @brief  Updating Auto Low Power On timeout
 *  @param  mode timeout mode. check auto_low_power_timeout_mode_t for more detail
 *  @param  timeout_threshold threshold in ms scale
 */
void BMA400::SetAutoLowPowerOnTimeout(auto_low_power_timeout_mode_t mode, float timeout_threshold)
{
    uint8_t val = read(BMA400_REG_AUTO_LOW_POW_1) & 0x03;

    switch (mode)
    {
    case auto_low_power_timeout_mode_t::DISABLE:
    case auto_low_power_timeout_mode_t::UNKNOWN_TIMEOUT:
        // Do nothing
        break;
    case auto_low_power_timeout_mode_t::ON_TIMEOUT:
        val |= 0x04;
        break;

    case auto_low_power_timeout_mode_t::ON_TIMEOUT_RST_G_INT2:
        val |= 0x08;
        break;
    }

    timeout_threshold /= 2.5;
    val = ((uint8_t)remainder(timeout_threshold, 16)) << 4;
    write(BMA400_REG_AUTO_LOW_POW_1, val);

    write(BMA400_REG_AUTO_LOW_POW_0, (uint8_t)(timeout_threshold / 16));
}

/*!
 *  @brief  Updating Auto Low Power
 *  @param  onDataReady enables Auto Low Power on Data Ready
 *  @param  onGenericInterrupt1 enables Auto Low Power on Generic Interrupt 1
 *  @param  mode timeout mode. check auto_low_power_timeout_mode_t for more detail
 *  @param  timeout_threshold threshold in ms scale
 */
void BMA400::SetAutoLowPower(bool onDataReady, bool onGenericInterrupt1, auto_low_power_timeout_mode_t mode, float timeout_threshold)
{
    uint8_t val = 0;

    if (onDataReady)
        val |= 0x01;

    if (onGenericInterrupt1)
        val |= 0x02;

    switch (mode)
    {
    case auto_low_power_timeout_mode_t::DISABLE:
    case auto_low_power_timeout_mode_t::UNKNOWN_TIMEOUT:
        // Do nothing
        break;
    case auto_low_power_timeout_mode_t::ON_TIMEOUT:
        val |= 0x04;
        break;

    case auto_low_power_timeout_mode_t::ON_TIMEOUT_RST_G_INT2:
        val |= 0x08;
        break;
    }

    timeout_threshold /= 2.5;
    val = ((uint8_t)remainder(timeout_threshold, 16)) << 4;

    write(BMA400_REG_AUTO_LOW_POW_1, val);

    write(BMA400_REG_AUTO_LOW_POW_0, (uint8_t)(timeout_threshold / 16));
}

/*!
 *  @brief  Updating the data rate
 *  @param  rate data rate select one between 16 rates. see output_data_rate_t for more details
 */
void BMA400::SetDataRate(output_data_rate_t rate)
{
    uint8_t val;
    switch (rate)
    {
    case output_data_rate_t::Filter1_048x_800Hz:
        unset(BMA400_REG_ACC_CONFIG_0, 7);
        write(BMA400_REG_ACC_CONFIG_1, 0x0B, 0xF0);
        write(BMA400_REG_ACC_CONFIG_2, 0x00, 0xF3);
        break;

    case output_data_rate_t::Filter1_024x_800Hz:
        set(BMA400_REG_ACC_CONFIG_0, 7);
        write(BMA400_REG_ACC_CONFIG_1, 0x0B, 0xF0);
        write(BMA400_REG_ACC_CONFIG_2, 0x00, 0xF3);
        break;

    case output_data_rate_t::Filter1_048x_400Hz:
        unset(BMA400_REG_ACC_CONFIG_0, 7);
        write(BMA400_REG_ACC_CONFIG_1, 0x0A, 0xF0);
        write(BMA400_REG_ACC_CONFIG_2, 0x00, 0xF3);
        break;

    case output_data_rate_t::Filter1_024x_400Hz:
        set(BMA400_REG_ACC_CONFIG_0, 7);
        write(BMA400_REG_ACC_CONFIG_1, 0x0A, 0xF0);
        write(BMA400_REG_ACC_CONFIG_2, 0x00, 0xF3);
        break;

    case output_data_rate_t::Filter1_048x_200Hz:
        unset(BMA400_REG_ACC_CONFIG_0, 7);
        write(BMA400_REG_ACC_CONFIG_1, 0x09, 0xF0);
        write(BMA400_REG_ACC_CONFIG_2, 0x00, 0xF3);
        break;

    case output_data_rate_t::Filter1_024x_200Hz:
        set(BMA400_REG_ACC_CONFIG_0, 7);
        write(BMA400_REG_ACC_CONFIG_1, 0x09, 0xF0);
        write(BMA400_REG_ACC_CONFIG_2, 0x00, 0xF3);
        break;

    case output_data_rate_t::Filter1_048x_100Hz:
        unset(BMA400_REG_ACC_CONFIG_0, 7);
        write(BMA400_REG_ACC_CONFIG_1, 0x08, 0xF0);
        write(BMA400_REG_ACC_CONFIG_2, 0x00, 0xF3);
        break;

    case output_data_rate_t::Filter1_024x_100Hz:
        set(BMA400_REG_ACC_CONFIG_0, 7);
        write(BMA400_REG_ACC_CONFIG_1, 0x08, 0xF0);
        write(BMA400_REG_ACC_CONFIG_2, 0x00, 0xF3);
        break;

    case output_data_rate_t::Filter1_048x_50Hz:
        unset(BMA400_REG_ACC_CONFIG_0, 7);
        write(BMA400_REG_ACC_CONFIG_1, 0x07, 0xF0);
        write(BMA400_REG_ACC_CONFIG_2, 0x00, 0xF3);
        break;

    case output_data_rate_t::Filter1_024x_50Hz:
        set(BMA400_REG_ACC_CONFIG_0, 7);
        write(BMA400_REG_ACC_CONFIG_1, 0x07, 0xF0);
        write(BMA400_REG_ACC_CONFIG_2, 0x00, 0xF3);
        break;

    case output_data_rate_t::Filter1_048x_25Hz:
        unset(BMA400_REG_ACC_CONFIG_0, 7);
        write(BMA400_REG_ACC_CONFIG_1, 0x06, 0xF0);
        write(BMA400_REG_ACC_CONFIG_2, 0x00, 0xF3);
        break;

    case output_data_rate_t::Filter1_024x_25Hz:
        set(BMA400_REG_ACC_CONFIG_0, 7);
        write(BMA400_REG_ACC_CONFIG_1, 0x06, 0xF0);
        write(BMA400_REG_ACC_CONFIG_2, 0x00, 0xF3);
        break;

    case output_data_rate_t::Filter1_048x_12Hz:
        unset(BMA400_REG_ACC_CONFIG_0, 7);
        write(BMA400_REG_ACC_CONFIG_1, 0x05, 0xF0);
        write(BMA400_REG_ACC_CONFIG_2, 0x00, 0xF3);
        break;

    case output_data_rate_t::Filter1_024x_12Hz:
        set(BMA400_REG_ACC_CONFIG_0, 7);
        write(BMA400_REG_ACC_CONFIG_1, 0x05, 0xF0);
        write(BMA400_REG_ACC_CONFIG_2, 0x00, 0xF3);
        break;

    case output_data_rate_t::Filter2_100Hz:
        write(BMA400_REG_ACC_CONFIG_2, 0x04, 0xF3);
        break;

    case output_data_rate_t::Filter2_100Hz_LPF_1Hz:
        write(BMA400_REG_ACC_CONFIG_2, 0x08, 0xF3);
        break;

    default:
        break;
    }
}

/*!
 *  @brief  Getting the data rate
 *  @return  data rate (16 rates). see output_data_rate_t for more details
 */
BMA400::output_data_rate_t BMA400::GetDataRate()
{
    uint8_t val = read(BMA400_REG_ACC_CONFIG_2);

    if (val == 0x04)
        return output_data_rate_t::Filter2_100Hz;
    else if (val == 0x08)
        return output_data_rate_t::Filter2_100Hz_LPF_1Hz;

    val = read(BMA400_REG_ACC_CONFIG_1) & 0x0F;
    bool is24x = (read(BMA400_REG_ACC_CONFIG_0) & 0x80) == 0x80;

    if (val >= 0x0B)
        return is24x ? output_data_rate_t::Filter1_024x_800Hz : output_data_rate_t::Filter1_048x_800Hz;
    else if (val <= 0x05)
        return is24x ? output_data_rate_t::Filter1_024x_12Hz : output_data_rate_t::Filter1_048x_12Hz;

    switch (val)
    {
    case 0x0A:
        return is24x ? output_data_rate_t::Filter1_024x_400Hz : output_data_rate_t::Filter1_048x_400Hz;

    case 0x09:
        return is24x ? output_data_rate_t::Filter1_024x_200Hz : output_data_rate_t::Filter1_048x_200Hz;

    case 0x08:
        return is24x ? output_data_rate_t::Filter1_024x_100Hz : output_data_rate_t::Filter1_048x_100Hz;

    case 0x07:
        return is24x ? output_data_rate_t::Filter1_024x_50Hz : output_data_rate_t::Filter1_048x_50Hz;

    case 0x06:
        return is24x ? output_data_rate_t::Filter1_024x_25Hz : output_data_rate_t::Filter1_048x_25Hz;
    }

    output_data_rate_t::UNKNOWN_RATE;
}

/*!
 *  @brief  Updating the sampling range
 *  @param  range sampling range 2, 4, 8, 16G. Use acceleation_range_t data type
 */
void BMA400::SetRange(acceleation_range_t range)
{
    switch (range)
    {
    case acceleation_range_t::RANGE_2G:
        write(BMA400_REG_ACC_CONFIG_1, 0x00, 0x3F);
        break;

    case acceleation_range_t::RANGE_4G:
        write(BMA400_REG_ACC_CONFIG_1, 0x40, 0x3F);
        break;

    case acceleation_range_t::RANGE_8G:
        write(BMA400_REG_ACC_CONFIG_1, 0x80, 0x3F);
        break;

    case acceleation_range_t::RANGE_16G:
        write(BMA400_REG_ACC_CONFIG_1, 0xC0, 0x3F);
        break;
    }
}

/*!
 *  @brief  Getting the sampling range
 *  @return sampling range 2, 4, 8, 16G acceleation_range_t data type
 */
BMA400::acceleation_range_t BMA400::GetRange()
{
    switch (read(BMA400_REG_ACC_CONFIG_1) & 0xC0)
    {
    case 0x00:
        return acceleation_range_t::RANGE_2G;

    case 0x40:
        return acceleation_range_t::RANGE_4G;

    case 0x80:
        return acceleation_range_t::RANGE_8G;

    case 0xC0:
        return acceleation_range_t::RANGE_16G;
    }

    return acceleation_range_t::UNKNOWN_RANGE;
}

/*!
 *  @brief  Configures Generic Interrupt 1 or 2
 *  @param  interrupt target interrupt. it has to be either ADV_GENERIC_INTERRUPT_1 or ADV_GENERIC_INTERRUPT_2
 *  @param  enable true if enables interrupt otherwise it disables the interrupt
 *  @param  reference mode of updating reference acceleration. see generic_interrupt_reference_update_t
 *  @param  mode Interrupt mode. On Activity or On Inactivity
 *  @param  threshold threshold (raw value) LSB = 8mg
 *  @param  duration minimum duration can generate interrupt - (raw value) depending on ODR
 *  @param  hystersis hystersis amplitude. can be selected between 0, 24, 48, 96mg
 *  @param  data_source data source is used to monitor the acceleration. Acc Filt 2 is recommended
 *  @param  enableX enables interrupt on X Axis
 *  @param  enableY enables interrupt on Y Axis
 *  @param  enableZ enables interrupt on Z Axis
 *  @param  all_combined if true uses AND logic applies on all axes to generate interrupts, otherwise OR logic
 *  @param  ignoreSamplingRateFix if false automatically increases the ODR to 100Hz if it's lower
 */
void BMA400::SetGenericInterrupt(
    interrupt_source_t interrupt, bool enable,
    generic_interrupt_reference_update_t reference,
    generic_interrupt_mode_t mode,
    uint8_t threshold,
    uint16_t duration,
    generic_interrupt_hysteresis_amplitude_t hystersis,
    generic_interrupt_data_source_t data_source = generic_interrupt_data_source_t::ACC_FILT_2,
    bool enableX = true, bool enableY = true, bool enableZ = true,
    bool all_combined = false, bool ignoreSamplingRateFix = false)
{
    if (interrupt != interrupt_source_t::ADV_GENERIC_INTERRUPT_1 &
        interrupt != interrupt_source_t::ADV_GENERIC_INTERRUPT_2) //# ignore if not a generic interrupt
        return;
    if (!enable) //# just disable the interrupt
    {
        unset(BMA400_REG_INT_CONFIG_0, interrupt == interrupt_source_t::ADV_GENERIC_INTERRUPT_1 ? 2 : 3);
        return;
    }

    uint8_t _register = interrupt == interrupt_source_t::ADV_GENERIC_INTERRUPT_1 ? BMA400_REG_GEN_INT_1_CONFIG : BMA400_REG_GEN_INT_2_CONFIG;
    uint8_t val = 0;

    //# increasing the rate to be at least 100Hz
    if (!ignoreSamplingRateFix)
    {
        output_data_rate_t rate = GetDataRate();
        if (rate == output_data_rate_t::Filter1_024x_12Hz |
            rate == output_data_rate_t::Filter1_024x_25Hz |
            rate == output_data_rate_t::Filter1_024x_50Hz)
            SetDataRate(output_data_rate_t::Filter1_024x_100Hz);
        else if (rate == output_data_rate_t::Filter1_048x_12Hz |
                 rate == output_data_rate_t::Filter1_048x_25Hz |
                 rate == output_data_rate_t::Filter1_048x_50Hz)
            SetDataRate(output_data_rate_t::Filter1_048x_100Hz);
    }

    //# enabling interrupt
    set(BMA400_REG_INT_CONFIG_0, interrupt == interrupt_source_t::ADV_GENERIC_INTERRUPT_1 ? 2 : 3);

    //# setting config 0 register
    switch (hystersis)
    {
    case generic_interrupt_hysteresis_amplitude_t::AMP_0mg:
        // Do nothing
        break;

    case generic_interrupt_hysteresis_amplitude_t::AMP_24mg:
        val |= 0x01;
        break;

    case generic_interrupt_hysteresis_amplitude_t::AMP_48mg:
        val |= 0x02;
        break;

    case generic_interrupt_hysteresis_amplitude_t::AMP_96mg:
        val |= 0x03;
        break;
    }

    switch (reference)
    {
    case generic_interrupt_reference_update_t::MANUAL_UPDATE:
        // Do nothing
        break;

    case generic_interrupt_reference_update_t::ONETIME_UPDATE:
        val |= 0x04;
        break;

    case generic_interrupt_reference_update_t::EVERYTIME_UPDATE_FROM_ACC_FILTx:
        val |= 0x08;
        break;

    case generic_interrupt_reference_update_t::EVERYTIME_UPDATE_FROM_ACC_FILT_LP:
        val |= 0x0C;
        break;
    }

    if (data_source == generic_interrupt_data_source_t::ACC_FILT_2)
        val |= 0x10;

    if (enableX)
        val |= 0x20;

    if (enableY)
        val |= 0x40;

    if (enableZ)
        val |= 0x80;

    write(_register, val);

    //# setting config 1 register
    _register++;
    val = 0;

    if (all_combined)
        val |= 0x01;

    if (mode == generic_interrupt_mode_t::ACTIVITY_DETECTION)
        val |= 0x02;

    write(_register, val);

    //# setting config 2 register
    _register++;
    write(_register, threshold);

    //# setting config 3 register
    _register++;
    write(_register, (uint8_t)(duration >> 8));

    //# setting config 4 register
    _register++;
    write(_register, (uint8_t)duration);
}

/*!
 *  @brief  Configures Generic Interrupt 1 or 2
 *  @param  interrupt target interrupt. it has to be either ADV_GENERIC_INTERRUPT_1 or ADV_GENERIC_INTERRUPT_2
 *  @param  enable true if enables interrupt otherwise it disables the interrupt
 *  @param  reference mode of updating reference acceleration. see generic_interrupt_reference_update_t
 *  @param  mode Interrupt mode. On Activity or On Inactivity
 *  @param  threshold threshold - in mg
 *  @param  duration minimum duration can generate interrupt - in mili seconds
 *  @param  hystersis hystersis amplitude. can be selected between 0, 24, 48, 96mg
 *  @param  data_source data source is used to monitor the acceleration. Acc Filt 2 is recommended
 *  @param  enableX enables interrupt on X Axis
 *  @param  enableY enables interrupt on Y Axis
 *  @param  enableZ enables interrupt on Z Axis
 *  @param  all_combined if true uses AND logic applies on all axes to generate interrupts, otherwise OR logic
 *  @param  ignoreSamplingRateFix if false automatically increases the ODR to 100Hz if it's lower
 */
void BMA400::SetGenericInterrupt(
    interrupt_source_t interrupt, bool enable,
    generic_interrupt_reference_update_t reference,
    generic_interrupt_mode_t mode,
    float threshold,
    float duration,
    generic_interrupt_hysteresis_amplitude_t hystersis,
    generic_interrupt_data_source_t data_source = generic_interrupt_data_source_t::ACC_FILT_2,
    bool enableX = true, bool enableY = true, bool enableZ = true,
    bool all_combined = false, bool ignoreSamplingRateFix = false)
{
    if (interrupt != interrupt_source_t::ADV_GENERIC_INTERRUPT_1 &
        interrupt != interrupt_source_t::ADV_GENERIC_INTERRUPT_2) //# ignore if not a generic interrupt
        return;
    if (!enable) //# just disable the interrupt
    {
        unset(BMA400_REG_INT_CONFIG_0, interrupt == interrupt_source_t::ADV_GENERIC_INTERRUPT_1 ? 2 : 3);
        return;
    }

    uint8_t _register = interrupt == interrupt_source_t::ADV_GENERIC_INTERRUPT_1 ? BMA400_REG_GEN_INT_1_CONFIG : BMA400_REG_GEN_INT_2_CONFIG;
    uint8_t val = 0;

    //# increasing the rate to be at least 100Hz
    if (!ignoreSamplingRateFix)
    {
        output_data_rate_t rate = GetDataRate();
        if (rate == output_data_rate_t::Filter1_024x_12Hz |
            rate == output_data_rate_t::Filter1_024x_25Hz |
            rate == output_data_rate_t::Filter1_024x_50Hz)
            SetDataRate(output_data_rate_t::Filter1_024x_100Hz);
        else if (rate == output_data_rate_t::Filter1_048x_12Hz |
                 rate == output_data_rate_t::Filter1_048x_25Hz |
                 rate == output_data_rate_t::Filter1_048x_50Hz)
            SetDataRate(output_data_rate_t::Filter1_048x_100Hz);
    }

    //# enabling interrupt
    set(BMA400_REG_INT_CONFIG_0, interrupt == interrupt_source_t::ADV_GENERIC_INTERRUPT_1 ? 2 : 3);

    //# setting config 0 register
    switch (hystersis)
    {
    case generic_interrupt_hysteresis_amplitude_t::AMP_0mg:
        // Do nothing
        break;

    case generic_interrupt_hysteresis_amplitude_t::AMP_24mg:
        val |= 0x01;
        break;

    case generic_interrupt_hysteresis_amplitude_t::AMP_48mg:
        val |= 0x02;
        break;

    case generic_interrupt_hysteresis_amplitude_t::AMP_96mg:
        val |= 0x03;
        break;
    }

    switch (reference)
    {
    case generic_interrupt_reference_update_t::MANUAL_UPDATE:
        // Do nothing
        break;

    case generic_interrupt_reference_update_t::ONETIME_UPDATE:
        val |= 0x04;
        break;

    case generic_interrupt_reference_update_t::EVERYTIME_UPDATE_FROM_ACC_FILTx:
        val |= 0x08;
        break;

    case generic_interrupt_reference_update_t::EVERYTIME_UPDATE_FROM_ACC_FILT_LP:
        val |= 0x0C;
        break;
    }

    if (data_source == generic_interrupt_data_source_t::ACC_FILT_2)
        val |= 0x10;

    if (enableX)
        val |= 0x20;

    if (enableY)
        val |= 0x40;

    if (enableZ)
        val |= 0x80;

    write(_register, val);

    //# setting config 1 register
    _register++;
    val = 0;

    if (all_combined)
        val |= 0x01;

    if (mode == generic_interrupt_mode_t::ACTIVITY_DETECTION)
        val |= 0x02;

    write(_register, val);

    //# setting config 2 register
    _register++;
    threshold /= 8;
    val = threshold > 255 ? 255 : (uint8_t)threshold;
    write(_register, val);

    //# setting config 3 register
    _register++;

    switch (GetDataRate())
    {
    case output_data_rate_t::Filter1_024x_12Hz:
    case output_data_rate_t::Filter1_048x_12Hz:
        duration *= 0.0125;
        break;

    case output_data_rate_t::Filter1_024x_25Hz:
    case output_data_rate_t::Filter1_048x_25Hz:
        duration *= 0.025;
        break;

    case output_data_rate_t::Filter1_024x_50Hz:
    case output_data_rate_t::Filter1_048x_50Hz:
        duration *= 0.05;
        break;

    case output_data_rate_t::Filter1_024x_100Hz:
    case output_data_rate_t::Filter1_048x_100Hz:
    case output_data_rate_t::Filter2_100Hz:
    case output_data_rate_t::Filter2_100Hz_LPF_1Hz:
        duration *= 0.1;
        break;

    case output_data_rate_t::Filter1_024x_200Hz:
    case output_data_rate_t::Filter1_048x_200Hz:
        duration *= 0.2;
        break;

    case output_data_rate_t::Filter1_024x_400Hz:
    case output_data_rate_t::Filter1_048x_400Hz:
        duration *= 0.4;
        break;

    case output_data_rate_t::Filter1_024x_800Hz:
    case output_data_rate_t::Filter1_048x_800Hz:
        duration *= 0.8;
        break;
    }
    uint16_t dur = (uint16_t)round(duration);

    write(_register, (uint8_t)(dur >> 8));

    //# setting config 4 register
    _register++;
    write(_register, (uint8_t)dur);
}

/*!
 *  @brief  Manually updating the reference acceleration
 *  @param  interrupt target interrupt. it has to be either ADV_GENERIC_INTERRUPT_1 or ADV_GENERIC_INTERRUPT_2
 *  @param  values raw values in order of [0]X(MSB) [1]X(LSB) [2]Y(MSB) [2]Y(LSB) [3]Z(MSB) [3]Z(LSB) 
 */
void BMA400::SetGenericInterruptReference(interrupt_source_t interrupt, uint8_t *values)
{
    uint8_t _register = interrupt == interrupt_source_t::ADV_GENERIC_INTERRUPT_1 ? BMA400_REG_GEN_INT_1_CONFIG : BMA400_REG_GEN_INT_2_CONFIG;
    _register += 5; //# pointing to config 4

    for (uint8_t i = 0; i < 6; i++)
    {
        write(_register, values[i]);
        _register++;
    }
}

/*!
 *  @brief  Enabling/Disabling the step detector interrupt and step counter
 *  @param  enable true to enable the interrupt (counter)
 */
void BMA400::SetStepDetectorCounter(bool enable)
{
    if (enable)
        set(BMA400_REG_INT_CONFIG_1, 0);
    else
        unset(BMA400_REG_INT_CONFIG_1, 0);
}

/*!
 *  @brief  Getting the total steps counted so far. 
 *  @return total steps counted
 */
uint32_t BMA400::GetTotalSteps()
{
    uint32_t value;
    uint8_t values[3] = {0};
    read(BMA400_REG_STEP_CNT0, 3, values);
    value = values[0] + values[1] * 256 + values[2] * 256 * 256;
    return value;
}

/*!
 *  @brief  Reseting the total steps counted so far. 
 *  @return true if reset step counter is successfully sent
 */
bool BMA400::ResetStepCounter()
{
    return ExecuteCommand(command_t::CMD_RESET_STEP_CNT);
}

/*!
 *  @brief  Configures Activity change Interrupt
 *  @param  threshold threshold - raw value
 *  @param  enable true if enables interrupt otherwise it disables the interrupt
 *  @param  observation_number number of observations generates the interrupt
 *  @param  data_source data source is used to monitor the acceleration. Acc Filt 2 is recommended
 *  @param  enableX enables interrupt on X Axis
 *  @param  enableY enables interrupt on Y Axis
 *  @param  enableZ enables interrupt on Z Axis
 */
void BMA400::SetActivityChangeInterrupt(bool enable,
                                        uint8_t threshold,
                                        activity_change_observation_number_t observation_number,
                                        generic_interrupt_data_source_t data_source,
                                        bool enableX, bool enableY, bool enableZ)
{
    if (!enable) //# Just disable the interrupt
    {
        unset(BMA400_REG_INT_CONFIG_1, 4);
        return;
    }

    set(BMA400_REG_INT_CONFIG_1, 4);
    write(BMA400_REG_ACT_CHNG_INT_CONFIG_0, threshold);

    uint8_t val = 0;

    switch (observation_number)
    {
    case activity_change_observation_number_t::OBSERVATION_32:
        // Do nothing
        break;

    case activity_change_observation_number_t::OBSERVATION_64:
        val |= 0x01;
        break;

    case activity_change_observation_number_t::OBSERVATION_128:
        val |= 0x02;
        break;

    case activity_change_observation_number_t::OBSERVATION_256:
        val |= 0x03;
        break;

    case activity_change_observation_number_t::OBSERVATION_512:
        val |= 0x04;
        break;
    }

    if (data_source == generic_interrupt_data_source_t::ACC_FILT_2)
        val |= 0x10;

    if (enableX)
        val |= 0x20;

    if (enableY)
        val |= 0x40;

    if (enableZ)
        val |= 0x80;

    write(BMA400_REG_ACT_CHNG_INT_CONFIG_1, val);
}

/*!
 *  @brief  Configures Activity change Interrupt
 *  @param  threshold threshold - in mg
 *  @param  enable true if enables interrupt otherwise it disables the interrupt
 *  @param  observation_number number of observations generates the interrupt
 *  @param  data_source data source is used to monitor the acceleration. Acc Filt 2 is recommended
 *  @param  enableX enables interrupt on X Axis
 *  @param  enableY enables interrupt on Y Axis
 *  @param  enableZ enables interrupt on Z Axis
 */
void BMA400::SetActivityChangeInterrupt(bool enable,
                                        float threshold,
                                        activity_change_observation_number_t observation_number,
                                        generic_interrupt_data_source_t data_source,
                                        bool enableX, bool enableY, bool enableZ)
{
    if (!enable) //# Just disable the interrupt
    {
        unset(BMA400_REG_INT_CONFIG_1, 4);
        return;
    }

    set(BMA400_REG_INT_CONFIG_1, 4);
    threshold /= 8.0;
    threshold = round(threshold);
    write(BMA400_REG_ACT_CHNG_INT_CONFIG_0, (uint8_t)threshold);

    uint8_t val = 0;

    switch (observation_number)
    {
    case activity_change_observation_number_t::OBSERVATION_32:
        // Do nothing
        break;

    case activity_change_observation_number_t::OBSERVATION_64:
        val |= 0x01;
        break;

    case activity_change_observation_number_t::OBSERVATION_128:
        val |= 0x02;
        break;

    case activity_change_observation_number_t::OBSERVATION_256:
        val |= 0x03;
        break;

    case activity_change_observation_number_t::OBSERVATION_512:
        val |= 0x04;
        break;
    }

    if (data_source == generic_interrupt_data_source_t::ACC_FILT_2)
        val |= 0x10;

    if (enableX)
        val |= 0x20;

    if (enableY)
        val |= 0x40;

    if (enableZ)
        val |= 0x80;

    write(BMA400_REG_ACT_CHNG_INT_CONFIG_1, val);
}

//* Private methods
void BMA400::read(uint8_t _register, uint8_t length, uint8_t *values)
{
    wire->beginTransmission(address);
    wire->write(_register);
    wire->endTransmission();
    wire->requestFrom(address, length);
    for (uint8_t i = 0; i < length; i++)
        values[i] = wire->read();
}

uint8_t BMA400::read(uint8_t _register)
{
    wire->beginTransmission(address);
    wire->write(_register);
    wire->endTransmission();
    wire->requestFrom(address, 1);
    return wire->read();
}

void BMA400::write(uint8_t _register, const uint8_t &value)
{
    wire->beginTransmission(address);
    wire->write((uint8_t)_register);
    wire->write((uint8_t)value);
    wire->endTransmission();
}

void BMA400::write(uint8_t _register, const uint8_t &value, const uint8_t &mask)
{
    uint8_t val = (read(_register) & mask) | value;
    wire->beginTransmission(address);
    wire->write((uint8_t)_register);
    wire->write((uint8_t)val);
    wire->endTransmission();
}

void BMA400::set(uint8_t _register, const uint8_t &_bit)
{
    uint8_t value = read(_register);
    value |= (1 << _bit);
    write(_register, value);
}

void BMA400::unset(uint8_t _register, const uint8_t &_bit)
{
    uint8_t value = read(_register);
    value &= ~(1 << _bit);
    write(_register, value);
}