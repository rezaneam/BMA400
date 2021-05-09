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
 *  @brief  Quick Stepup for BMA400
 *  @param  mode power mode see power_mode_t for more details
 *  @param  rate data rate select one between 16 rates. see output_data_rate_t for more details
 *  @param  range sampling range 2, 4, 8, 16G. Use acceleation_range_t data type
 */
void BMA400::Setup(const power_mode_t &mode, output_data_rate_t rate, acceleation_range_t range)
{
    SetPowerMode(mode);
    SetRange(range);
    SetDataRate(rate);
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
        config = (config & 0xFC) | 0x02;
        write(BMA400_REG_ACC_CONFIG_0, config);
        config = read(BMA400_REG_ACC_CONFIG_1);
        config = (config & 0xCF) | 0x10;
        write(BMA400_REG_ACC_CONFIG_1, config);
        return;

    case power_mode_t::NORMAL_LOW_NOISE:
        config = (config & 0xFC) | 0x02;
        write(BMA400_REG_ACC_CONFIG_0, config);
        config = read(BMA400_REG_ACC_CONFIG_1);
        config = (config & 0xCF) | 0x20;
        write(BMA400_REG_ACC_CONFIG_1, config);
        return;

    case power_mode_t::NORMAL_LOWEST_NOISE:
        config = (config & 0xFC) | 0x02;
        write(BMA400_REG_ACC_CONFIG_0, config);
        config = read(BMA400_REG_ACC_CONFIG_1);
        config = (config & 0xCF) | 0x30;
        write(BMA400_REG_ACC_CONFIG_1, config);
        return;

    default:
        break;
    }
}

/*!
 *  @brief  Getting Acceleration - unprocessed 
 *  @param  values must be address of an array (uint16_t) with at least 3 elements 
 */
void BMA400::ReadAcceleration(int16_t *values)
{
    uint8_t data[6];

    read(BMA400_REG_ACC_DATA, 6, data);

    for (uint8_t i = 0; i < 3; i++)
    {
        values[i] = data[0 + i * 2] + 256 * data[1 + i * 2];
        if (values[i] > 2047)
            values[i] -= 4096;
    }
}

/*!
 *  @brief  Getting Acceleration - processed in g
 *  @param  values must be address of an array (float) with at least 3 elements 
 */
void BMA400::ReadAcceleration(float *values)
{
    uint8_t data[6];
    float divider = 1;
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

    default:
        break;
    }

    for (uint8_t i = 0; i < 3; i++)
    {
        values[i] = data[0 + i * 2] + 256 * data[1 + i * 2];
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
    if (enable)
        set(BMA400_REG_AUTO_LOW_POW_1, 0);
    else
        unset(BMA400_REG_AUTO_LOW_POW_1, 0);
}

/*!
 *  @brief  Updating Auto Low Power On Generic Interrupt 1
 *  @param  enable true will enable the auto low power
 */
void BMA400::SetAutoLowPowerOnGenericInterrupt1(bool enable)
{
    if (enable)
        set(BMA400_REG_AUTO_LOW_POW_1, 1);
    else
        unset(BMA400_REG_AUTO_LOW_POW_1, 1);
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
 *  @brief  Configuring Auto Low Power
 *  @param  onDataReady enables Auto Low Power on Data Ready
 *  @param  onGenericInterrupt1 enables Auto Low Power on Generic Interrupt 1
 *  @param  mode timeout mode. check auto_low_power_timeout_mode_t for more detail
 *  @param  timeout_threshold threshold in ms scale
 */
void BMA400::ConfigureAutoLowPower(bool onDataReady, bool onGenericInterrupt1, auto_low_power_timeout_mode_t mode, float timeout_threshold)
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

    return output_data_rate_t::UNKNOWN_RATE;
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

    default:
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
 *  @brief  Getting all triggered interrupts
 *  @return combination of all interrupts if there is more than one
 */
BMA400::interrupt_source_t BMA400::GetInterrupts()
{
    uint16_t result = 0;
    uint8_t interrupts[3] = {0};
    read(BMA400_REG_INT_STAT_0, 3, interrupts);

    if (interrupts[0] & 0x01)
        result |= interrupt_source_t::BAS_WAKEUP;

    if (interrupts[0] & 0x02)
        result |= interrupt_source_t::ADV_ORIENTATION_CHANGE;

    if (interrupts[0] & 0x04)
        result |= interrupt_source_t::ADV_GENERIC_INTERRUPT_1;

    if (interrupts[0] & 0x08)
        result |= interrupt_source_t::ADV_GENERIC_INTERRUPT_2;

    if ((interrupts[0] & 0x10) | (interrupts[1] & 0x10) | (interrupts[2] & 0x10))
        result |= interrupt_source_t::BAS_ENGINE_OVERRUN;

    if (interrupts[0] & 0x20)
        result |= interrupt_source_t::BAS_FIFO_FULL;

    if (interrupts[0] & 0x40)
        result |= interrupt_source_t::BAS_FIFO_WATERMARK;

    if (interrupts[0] & 0x80)
        result |= interrupt_source_t::BAS_DATA_READY;

    if (interrupts[1] & 0x01)
        result |= interrupt_source_t::ADV_SET_DETECTOR_COUNTER;

    if (interrupts[1] & 0x02)
        result |= interrupt_source_t::ADV_SET_DETECTOR_COUNTER_DOUBLE_STEP;

    if (interrupts[1] & 0x04)
        result |= interrupt_source_t::ADV_SINGLE_TAP;

    if (interrupts[1] & 0x08)
        result |= interrupt_source_t::ADV_DOUBLE_TAP;

    if (interrupts[2] & 0x01)
        result |= interrupt_source_t::ADV_ORIENTATION_CHANGE_X;

    if (interrupts[2] & 0x02)
        result |= interrupt_source_t::ADV_ORIENTATION_CHANGE_Y;

    if (interrupts[2] & 0x04)
        result |= interrupt_source_t::ADV_ORIENTATION_CHANGE_Z;

    return (interrupt_source_t)result;
}

/*!
 *  @brief  Checks if an specific interrupt is rised after reading all interrupts blindly.
 *  Not a good idea to use this method if you enabled multiple interrupts
 *  @return true if the target interrupt is triggered
 */
bool BMA400::HasInterrupt(interrupt_source_t source)
{
    return (bool)(GetInterrupts() & source);
}

/*!
 *  @brief  Enabling/Disabling Basic Interrupts
 *  @param  source Interrupt source. can be either Data Ready, FIFO Full or FIFO Watermark
 *  @param  enable true to enable interrupt
 */
void BMA400::ConfigureBasicInterrupts(interrupt_source_t source, bool enable)
{
    switch (source)
    {
    case interrupt_source_t::BAS_DATA_READY:
        if (enable)
            set(BMA400_REG_INT_CONFIG_0, 7);
        else
            unset(BMA400_REG_INT_CONFIG_0, 7);
        break;

    case interrupt_source_t::BAS_FIFO_WATERMARK:
        if (enable)
            set(BMA400_REG_INT_CONFIG_0, 6);
        else
            unset(BMA400_REG_INT_CONFIG_0, 6);
        break;

    case interrupt_source_t::BAS_FIFO_FULL:
        if (enable)
            set(BMA400_REG_INT_CONFIG_0, 5);
        else
            unset(BMA400_REG_INT_CONFIG_0, 5);
        break;

    default:
        // Ignore the rest
        break;
    }
}

/*!
 *  @brief  Configure Basic Interrupts (Enabling/Disabling & Linking to Interrupt Pins)
 *  @param  source Interrupt source. can be either Data Ready, FIFO Full or FIFO Watermark
 *  @param  enable true to enable interrupt
 *  @param  pin target interrupt pin. see interrupt_pin_t for more detail
 */
void BMA400::ConfigureBasicInterrupts(interrupt_source_t source, bool enable, interrupt_pin_t pin)
{
    ConfigureBasicInterrupts(source, enable);
    LinkToInterruptPin(source, pin);
}

/*!
 *  @brief  Configuring electrical behavior of interrupt pins
 *  @param  isLatched set true to enable latched (hold) interrupt. In latch mode interrupt stays active until reading the corresponding interrupt register is read.
 *  @param  isINT1_active_hi set true to set interrupt pin 1 active mode to high level (pull up), otherwise it's pulled down when active
 *  @param  isINT2_active_hi set true to set interrupt pin 1 active mode to high level (pull up), otherwise it's pulled down when active
 *  @param  isINT1_open_drive set true to enable open drive mode for Interrupt Pin 1, otherwise it is using push pull electrical drive
 *  @param  isINT2_open_drive set true to enable open drive mode for Interrupt Pin 1, otherwise it is using push pull electrical drive
 */
void BMA400::ConfigureInterruptPinSettings(
    bool isLatched,
    bool isINT1_active_hi,
    bool isINT2_active_hi,
    bool isINT1_open_drive,
    bool isINT2_open_drive)
{
    if (isLatched)
        set(BMA400_REG_INT_CONFIG_1, 7);
    else
        unset(BMA400_REG_INT_CONFIG_1, 7);

    uint8_t val = 0;

    if (isINT1_active_hi)
        val |= 0x02;

    if (isINT2_active_hi)
        val |= 0x20;

    if (isINT1_open_drive)
        val |= 0x04;

    if (isINT2_open_drive)
        val |= 0x40;

    write(BMA400_REG_INT_IO_CTRL, val);
}

/*!
 *  @brief  Links/unlinks an interrupt source to/from interrupt pins
 *  @param  interrupt Interrupt source (can be any interrupt source)
 *  @param  pin target pin(s) should be linked to the interrupt source. can either, none or both
 */
void BMA400::LinkToInterruptPin(interrupt_source_t interrupt, interrupt_pin_t pin)
{
    switch (interrupt)
    {
    case interrupt_source_t::BAS_DATA_READY:
        switch (pin)
        {

        case interrupt_pin_t::INT_NONE:
            unset(BMA400_REG_INT1_MAP, 7);
            unset(BMA400_REG_INT2_MAP, 7);
            break;

        case interrupt_pin_t::INT_PIN_1:
            set(BMA400_REG_INT1_MAP, 7);
            unset(BMA400_REG_INT2_MAP, 7);
            break;

        case interrupt_pin_t::INT_PIN_2:
            unset(BMA400_REG_INT1_MAP, 7);
            set(BMA400_REG_INT2_MAP, 7);
            break;

        case interrupt_pin_t::INT_PIN_BOTH:
            set(BMA400_REG_INT1_MAP, 7);
            set(BMA400_REG_INT2_MAP, 7);
            break;
        }
        break;

    case interrupt_source_t::BAS_FIFO_WATERMARK:
        switch (pin)
        {

        case interrupt_pin_t::INT_NONE:
            unset(BMA400_REG_INT1_MAP, 6);
            unset(BMA400_REG_INT2_MAP, 6);
            break;

        case interrupt_pin_t::INT_PIN_1:
            set(BMA400_REG_INT1_MAP, 6);
            unset(BMA400_REG_INT2_MAP, 6);
            break;

        case interrupt_pin_t::INT_PIN_2:
            unset(BMA400_REG_INT1_MAP, 6);
            set(BMA400_REG_INT2_MAP, 6);
            break;

        case interrupt_pin_t::INT_PIN_BOTH:
            set(BMA400_REG_INT1_MAP, 6);
            set(BMA400_REG_INT2_MAP, 6);
            break;
        }
        break;

    case interrupt_source_t::BAS_FIFO_FULL:
        switch (pin)
        {

        case interrupt_pin_t::INT_NONE:
            unset(BMA400_REG_INT1_MAP, 5);
            unset(BMA400_REG_INT2_MAP, 5);
            break;

        case interrupt_pin_t::INT_PIN_1:
            set(BMA400_REG_INT1_MAP, 5);
            unset(BMA400_REG_INT2_MAP, 5);
            break;

        case interrupt_pin_t::INT_PIN_2:
            unset(BMA400_REG_INT1_MAP, 5);
            set(BMA400_REG_INT2_MAP, 5);
            break;

        case interrupt_pin_t::INT_PIN_BOTH:
            set(BMA400_REG_INT1_MAP, 5);
            set(BMA400_REG_INT2_MAP, 5);
            break;
        }
        break;

    case interrupt_source_t::BAS_ENGINE_OVERRUN:
        switch (pin)
        {

        case interrupt_pin_t::INT_NONE:
            unset(BMA400_REG_INT1_MAP, 4);
            unset(BMA400_REG_INT2_MAP, 4);
            break;

        case interrupt_pin_t::INT_PIN_1:
            set(BMA400_REG_INT1_MAP, 4);
            unset(BMA400_REG_INT2_MAP, 4);
            break;

        case interrupt_pin_t::INT_PIN_2:
            unset(BMA400_REG_INT1_MAP, 4);
            set(BMA400_REG_INT2_MAP, 4);
            break;

        case interrupt_pin_t::INT_PIN_BOTH:
            set(BMA400_REG_INT1_MAP, 4);
            set(BMA400_REG_INT2_MAP, 4);
            break;
        }
        break;

    case interrupt_source_t::BAS_WAKEUP:
        switch (pin)
        {

        case interrupt_pin_t::INT_NONE:
            unset(BMA400_REG_INT1_MAP, 0);
            unset(BMA400_REG_INT2_MAP, 0);
            break;

        case interrupt_pin_t::INT_PIN_1:
            set(BMA400_REG_INT1_MAP, 0);
            unset(BMA400_REG_INT2_MAP, 0);
            break;

        case interrupt_pin_t::INT_PIN_2:
            unset(BMA400_REG_INT1_MAP, 0);
            set(BMA400_REG_INT2_MAP, 0);
            break;

        case interrupt_pin_t::INT_PIN_BOTH:
            set(BMA400_REG_INT1_MAP, 0);
            set(BMA400_REG_INT2_MAP, 0);
            break;
        }
        break;

    case interrupt_source_t::ADV_GENERIC_INTERRUPT_1:
        switch (pin)
        {

        case interrupt_pin_t::INT_NONE:
            unset(BMA400_REG_INT1_MAP, 2);
            unset(BMA400_REG_INT2_MAP, 2);
            break;

        case interrupt_pin_t::INT_PIN_1:
            set(BMA400_REG_INT1_MAP, 2);
            unset(BMA400_REG_INT2_MAP, 2);
            break;

        case interrupt_pin_t::INT_PIN_2:
            unset(BMA400_REG_INT1_MAP, 2);
            set(BMA400_REG_INT2_MAP, 2);
            break;

        case interrupt_pin_t::INT_PIN_BOTH:
            set(BMA400_REG_INT1_MAP, 2);
            set(BMA400_REG_INT2_MAP, 2);
            break;
        }
        break;

    case interrupt_source_t::ADV_GENERIC_INTERRUPT_2:
        switch (pin)
        {

        case interrupt_pin_t::INT_NONE:
            unset(BMA400_REG_INT1_MAP, 3);
            unset(BMA400_REG_INT2_MAP, 3);
            break;

        case interrupt_pin_t::INT_PIN_1:
            set(BMA400_REG_INT1_MAP, 3);
            unset(BMA400_REG_INT2_MAP, 3);
            break;

        case interrupt_pin_t::INT_PIN_2:
            unset(BMA400_REG_INT1_MAP, 3);
            set(BMA400_REG_INT2_MAP, 3);
            break;

        case interrupt_pin_t::INT_PIN_BOTH:
            set(BMA400_REG_INT1_MAP, 3);
            set(BMA400_REG_INT2_MAP, 3);
            break;
        }
        break;

    case interrupt_source_t::ADV_ORIENTATION_CHANGE:
    case interrupt_source_t::ADV_ORIENTATION_CHANGE_X:
    case interrupt_source_t::ADV_ORIENTATION_CHANGE_Y:
    case interrupt_source_t::ADV_ORIENTATION_CHANGE_Z:
        switch (pin)
        {

        case interrupt_pin_t::INT_NONE:
            unset(BMA400_REG_INT1_MAP, 1);
            unset(BMA400_REG_INT2_MAP, 1);
            break;

        case interrupt_pin_t::INT_PIN_1:
            set(BMA400_REG_INT1_MAP, 1);
            unset(BMA400_REG_INT2_MAP, 1);
            break;

        case interrupt_pin_t::INT_PIN_2:
            unset(BMA400_REG_INT1_MAP, 1);
            set(BMA400_REG_INT2_MAP, 1);
            break;

        case interrupt_pin_t::INT_PIN_BOTH:
            set(BMA400_REG_INT1_MAP, 1);
            set(BMA400_REG_INT2_MAP, 1);
            break;
        }
        break;

    case interrupt_source_t::ADV_SET_DETECTOR_COUNTER:
    case interrupt_source_t::ADV_SET_DETECTOR_COUNTER_DOUBLE_STEP:
        switch (pin)
        {

        case interrupt_pin_t::INT_NONE:
            unset(BMA400_REG_INT12_MAP, 0);
            unset(BMA400_REG_INT12_MAP, 4);
            break;

        case interrupt_pin_t::INT_PIN_1:
            set(BMA400_REG_INT12_MAP, 0);
            unset(BMA400_REG_INT12_MAP, 4);
            break;

        case interrupt_pin_t::INT_PIN_2:
            unset(BMA400_REG_INT12_MAP, 0);
            set(BMA400_REG_INT12_MAP, 4);
            break;

        case interrupt_pin_t::INT_PIN_BOTH:
            set(BMA400_REG_INT12_MAP, 0);
            set(BMA400_REG_INT12_MAP, 4);
            break;
        }
        break;

    case interrupt_source_t::ADV_SINGLE_TAP:
    case interrupt_source_t::ADV_DOUBLE_TAP:
        switch (pin)
        {

        case interrupt_pin_t::INT_NONE:
            unset(BMA400_REG_INT12_MAP, 2);
            unset(BMA400_REG_INT12_MAP, 6);
            break;

        case interrupt_pin_t::INT_PIN_1:
            set(BMA400_REG_INT12_MAP, 2);
            unset(BMA400_REG_INT12_MAP, 6);
            break;

        case interrupt_pin_t::INT_PIN_2:
            unset(BMA400_REG_INT12_MAP, 2);
            set(BMA400_REG_INT12_MAP, 6);
            break;

        case interrupt_pin_t::INT_PIN_BOTH:
            set(BMA400_REG_INT12_MAP, 2);
            set(BMA400_REG_INT12_MAP, 6);
            break;
        }
        break;

    case interrupt_source_t::ADV_ACTIVITY_CHANGE:
        switch (pin)
        {

        case interrupt_pin_t::INT_NONE:
            unset(BMA400_REG_INT12_MAP, 3);
            unset(BMA400_REG_INT12_MAP, 7);
            break;

        case interrupt_pin_t::INT_PIN_1:
            set(BMA400_REG_INT12_MAP, 3);
            unset(BMA400_REG_INT12_MAP, 7);
            break;

        case interrupt_pin_t::INT_PIN_2:
            unset(BMA400_REG_INT12_MAP, 3);
            set(BMA400_REG_INT12_MAP, 7);
            break;

        case interrupt_pin_t::INT_PIN_BOTH:
            set(BMA400_REG_INT12_MAP, 3);
            set(BMA400_REG_INT12_MAP, 7);
            break;
        }
        break;
    }
}

/*!
 *  @brief  Configures Generic Interrupt 1 or 2
 *  @param  interrupt target interrupt. it has to be either ADV_GENERIC_INTERRUPT_1 or ADV_GENERIC_INTERRUPT_2
 *  @param  enable true if enables interrupt otherwise it disables the interrupt
 *  @param  pin wires the interrupt with any/both INT Pin 1 and INT Pin 2
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
void BMA400::ConfigureGenericInterrupt(
    interrupt_source_t interrupt, bool enable,
    interrupt_pin_t pin,
    generic_interrupt_reference_update_t reference,
    generic_interrupt_mode_t mode,
    uint8_t threshold,
    uint16_t duration,
    generic_interrupt_hysteresis_amplitude_t hystersis,
    interrupt_data_source_t data_source,
    bool enableX, bool enableY, bool enableZ,
    bool all_combined, bool ignoreSamplingRateFix)
{
    if ((interrupt != interrupt_source_t::ADV_GENERIC_INTERRUPT_1) &
        (interrupt != interrupt_source_t::ADV_GENERIC_INTERRUPT_2)) //# ignore if not a generic interrupt
        return;
    if (!enable) //# just disable the interrupt
    {
        unset(BMA400_REG_INT_CONFIG_0, interrupt == interrupt_source_t::ADV_GENERIC_INTERRUPT_1 ? 2 : 3);
        return;
    }

    //# Wiring Interrupt to Interrupt pins
    LinkToInterruptPin(interrupt, pin);

    uint8_t _register = interrupt == interrupt_source_t::ADV_GENERIC_INTERRUPT_1 ? BMA400_REG_GEN_INT_1_CONFIG : BMA400_REG_GEN_INT_2_CONFIG;
    uint8_t val = 0;

    //# increasing the rate to be at least 100Hz
    if (!ignoreSamplingRateFix)
    {
        output_data_rate_t rate = GetDataRate();
        if ((rate == output_data_rate_t::Filter1_024x_12Hz) |
            (rate == output_data_rate_t::Filter1_024x_25Hz) |
            (rate == output_data_rate_t::Filter1_024x_50Hz))
            SetDataRate(output_data_rate_t::Filter1_024x_100Hz);
        else if ((rate == output_data_rate_t::Filter1_048x_12Hz) |
                 (rate == output_data_rate_t::Filter1_048x_25Hz) |
                 (rate == output_data_rate_t::Filter1_048x_50Hz))
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

    if (data_source == interrupt_data_source_t::ACC_FILT_2)
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
 *  @param  pin wires the interrupt with any/both INT Pin 1 and INT Pin 2
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
void BMA400::ConfigureGenericInterrupt(
    interrupt_source_t interrupt, bool enable,
    interrupt_pin_t pin,
    generic_interrupt_reference_update_t reference,
    generic_interrupt_mode_t mode,
    float threshold,
    float duration,
    generic_interrupt_hysteresis_amplitude_t hystersis,
    interrupt_data_source_t data_source,
    bool enableX, bool enableY, bool enableZ,
    bool all_combined, bool ignoreSamplingRateFix)
{
    if ((interrupt != interrupt_source_t::ADV_GENERIC_INTERRUPT_1) &
        (interrupt != interrupt_source_t::ADV_GENERIC_INTERRUPT_2)) //# ignore if not a generic interrupt
        return;
    if (!enable) //# just disable the interrupt
    {
        unset(BMA400_REG_INT_CONFIG_0, interrupt == interrupt_source_t::ADV_GENERIC_INTERRUPT_1 ? 2 : 3);
        return;
    }

    //# Wiring Interrupt to Interrupt pins
    LinkToInterruptPin(interrupt, pin);

    uint8_t _register = interrupt == interrupt_source_t::ADV_GENERIC_INTERRUPT_1 ? BMA400_REG_GEN_INT_1_CONFIG : BMA400_REG_GEN_INT_2_CONFIG;
    uint8_t val = 0;

    //# increasing the rate to be at least 100Hz
    if (!ignoreSamplingRateFix)
    {
        output_data_rate_t rate = GetDataRate();
        if ((rate == output_data_rate_t::Filter1_024x_12Hz) |
            (rate == output_data_rate_t::Filter1_024x_25Hz) |
            (rate == output_data_rate_t::Filter1_024x_50Hz))
            SetDataRate(output_data_rate_t::Filter1_024x_100Hz);
        else if ((rate == output_data_rate_t::Filter1_048x_12Hz) |
                 (rate == output_data_rate_t::Filter1_048x_25Hz) |
                 (rate == output_data_rate_t::Filter1_048x_50Hz))
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

    if (data_source == interrupt_data_source_t::ACC_FILT_2)
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

    default:
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
 *  @brief  Use current acceleration values to Manually updating the reference acceleration
 *  @param  interrupt target interrupt. it has to be either ADV_GENERIC_INTERRUPT_1 or ADV_GENERIC_INTERRUPT_2
 */
void BMA400::SetGenericInterruptReference(interrupt_source_t interrupt)
{
    uint8_t data[6] = {0};
    read(BMA400_REG_ACC_DATA, 6, data);
    uint8_t _register = interrupt == interrupt_source_t::ADV_GENERIC_INTERRUPT_1 ? BMA400_REG_GEN_INT_1_CONFIG : BMA400_REG_GEN_INT_2_CONFIG;
    _register += 5; //# pointing to config 4

    for (uint8_t i = 0; i < 6; i++)
    {
        write(_register, data[i]);
        _register++;
    }
}

/*!
 *  @brief  Enabling/Disabling the step detector interrupt and step counter
 *  @param  pin wires the interrupt with any/both INT Pin 1 and INT Pin 2
 *  @param  enable true to enable the interrupt (counter)
 */
void BMA400::ConfigureStepDetectorCounter(bool enable, interrupt_pin_t pin)
{
    if (enable)
    {
        set(BMA400_REG_INT_CONFIG_1, 0);
        LinkToInterruptPin(interrupt_source_t::ADV_SET_DETECTOR_COUNTER, pin);
    }
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
 *  @param  pin wires the interrupt with any/both INT Pin 1 and INT Pin 2
 *  @param  enable true if enables interrupt otherwise it disables the interrupt
 *  @param  observation_number number of observations generates the interrupt
 *  @param  data_source data source is used to monitor the acceleration. Acc Filt 2 is recommended
 *  @param  enableX enables interrupt on X Axis
 *  @param  enableY enables interrupt on Y Axis
 *  @param  enableZ enables interrupt on Z Axis
 */
void BMA400::ConfigureActivityChangeInterrupt(bool enable,
                                              interrupt_pin_t pin,
                                              uint8_t threshold,
                                              activity_change_observation_number_t observation_number,
                                              interrupt_data_source_t data_source,
                                              bool enableX, bool enableY, bool enableZ)
{
    if (!enable) //# Just disable the interrupt
    {
        unset(BMA400_REG_INT_CONFIG_1, 4);
        return;
    }

    //# Wiring Interrupt to Interrupt pins
    LinkToInterruptPin(interrupt_source_t::ADV_ACTIVITY_CHANGE, pin);

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

    if (data_source == interrupt_data_source_t::ACC_FILT_2)
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
 *  @param  pin wires the interrupt with any/both INT Pin 1 and INT Pin 2
 *  @param  enable true if enables interrupt otherwise it disables the interrupt
 *  @param  observation_number number of observations generates the interrupt
 *  @param  data_source data source is used to monitor the acceleration. Acc Filt 2 is recommended
 *  @param  enableX enables interrupt on X Axis
 *  @param  enableY enables interrupt on Y Axis
 *  @param  enableZ enables interrupt on Z Axis
 */
void BMA400::ConfigureActivityChangeInterrupt(bool enable,
                                              interrupt_pin_t pin,
                                              float threshold,
                                              activity_change_observation_number_t observation_number,
                                              interrupt_data_source_t data_source,
                                              bool enableX, bool enableY, bool enableZ)
{
    if (!enable) //# Just disable the interrupt
    {
        unset(BMA400_REG_INT_CONFIG_1, 4);
        return;
    }

    //# Wiring Interrupt to Interrupt pins
    LinkToInterruptPin(interrupt_source_t::ADV_ACTIVITY_CHANGE, pin);

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

    if (data_source == interrupt_data_source_t::ACC_FILT_2)
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
 *  @brief  Configures Single and double tap interrupts
 *  @param  enableSingleTap true enables single tap interrupt otherwise it disables the interrupt
 *  @param  enableDoubleTap true enables double tap interrupt otherwise it disables the interrupt
 *  @param  axis tap axis can be either X, Y, or Z. use tap_axis_t data def
 *  @param  pin wires the interrupt with any/both INT Pin 1 and INT Pin 2
 *  @param  mode Interrupt mode. On Activity or On Inactivity
 *  @param  sensitivity sensitivity level to tap ranged from 7 (highest) to 0 (lowest). use tap_sensitivity_level_t type def 
 *  @param  pick_to_pick_interval maximum time between upper and lower peak of valid taps (in data samples). use tap_max_pick_to_pick_interval_t data type
 *  @param  quiet_interval Minimum quiet time (no tap) between two consecutive taps (in data samples). use tap_min_quiet_between_taps_t data type
 *  @param  double_taps_time Mininum time between two taps in a double tap (in data samples). use tap_min_quiet_inside_double_taps_t data type
 */
void BMA400::ConfigureTapInterrupt(
    bool enableSingleTap, bool enableDoubleTap,
    tap_axis_t axis,
    interrupt_pin_t pin,
    tap_sensitivity_level_t sensitivity,
    tap_max_pick_to_pick_interval_t pick_to_pick_interval,
    tap_min_quiet_between_taps_t quiet_interval,
    tap_min_quiet_inside_double_taps_t double_taps_time)
{

    if (!enableSingleTap & !enableDoubleTap)
    {
        unset(BMA400_REG_INT_CONFIG_1, 2);
        unset(BMA400_REG_INT_CONFIG_1, 3);
        return; //# Just disable both interrupts
    }

    //# Wiring Interrupt to Interrupt pins
    LinkToInterruptPin(interrupt_source_t::ADV_SINGLE_TAP, pin);

    //# Force increasing the ODR to 200Hz
    output_data_rate_t rate = GetDataRate();
    if ((rate == output_data_rate_t::Filter1_024x_12Hz) |
        (rate == output_data_rate_t::Filter1_024x_25Hz) |
        (rate == output_data_rate_t::Filter1_024x_50Hz) |
        (rate == output_data_rate_t::Filter1_024x_100Hz))
        SetDataRate(output_data_rate_t::Filter1_024x_200Hz);
    else if ((rate == output_data_rate_t::Filter1_048x_12Hz) |
             (rate == output_data_rate_t::Filter1_048x_25Hz) |
             (rate == output_data_rate_t::Filter1_048x_50Hz) |
             (rate == output_data_rate_t::Filter1_048x_100Hz) |
             (rate == output_data_rate_t::Filter2_100Hz) |
             (rate == output_data_rate_t::Filter2_100Hz_LPF_1Hz))
        SetDataRate(output_data_rate_t::Filter1_048x_200Hz);

    //# Enabling the interrupts
    if (enableSingleTap)
        set(BMA400_REG_INT_CONFIG_1, 2);
    else
        unset(BMA400_REG_INT_CONFIG_1, 2);

    if (enableDoubleTap)
        set(BMA400_REG_INT_CONFIG_1, 3);
    else
        unset(BMA400_REG_INT_CONFIG_1, 3);

    uint8_t val = (uint8_t)sensitivity;

    switch (axis)
    {
    case tap_axis_t::TAP_X_AXIS:
        val |= 0x08;
        break;

    case tap_axis_t::TAP_Y_AXIS:
        val |= 0x04;
        break;

    case tap_axis_t::TAP_Z_AXIS:
        // Do nothing
        break;
    }

    write(BMA400_REG_TAP_CONFIG_0, val);

    val = (uint8_t)pick_to_pick_interval;

    switch (quiet_interval)
    {
    case tap_min_quiet_between_taps_t::MIN_QUIET_60_SAMPLES:
        // Do nothing
        break;

    case tap_min_quiet_between_taps_t::MIN_QUIET_80_SAMPLES:
        val |= 0x04;
        break;

    case tap_min_quiet_between_taps_t::MIN_QUIET_100_SAMPLES:
        val |= 0x08;
        break;

    case tap_min_quiet_between_taps_t::MIN_QUIET_120_SAMPLES:
        val |= 0x0C;
        break;
    }

    switch (double_taps_time)
    {
    case tap_min_quiet_inside_double_taps_t::MIN_QUIET_DT_4_SAMPLES:
        // Do nothing
        break;

    case tap_min_quiet_inside_double_taps_t::MIN_QUIET_DT_8_SAMPLES:
        val |= 0x10;
        break;

    case tap_min_quiet_inside_double_taps_t::MIN_QUIET_DT_12_SAMPLES:
        val |= 0x10;
        break;

    case tap_min_quiet_inside_double_taps_t::MIN_QUIET_DT_16_SAMPLES:
        val |= 0x10;
        break;
    }

    write(BMA400_REG_TAP_CONFIG_1, val);
}

/*!
 *  @brief  Configures Orientation Changed Interrupt
 *  @param  enable  set true to enable the interrupt
 *  @param  enableX enables interrupt on X Axis
 *  @param  enableY enables interrupt on Y Axis
 *  @param  enableZ enables interrupt on Z Axis
 *  @param  pin wires the interrupt with any/both INT Pin 1 and INT Pin 2
 *  @param  source  data source is used as input for the orientation change detection. 
 * see orientation_reference_update_data_source_t for mode details
 *  @param  reference_update_mode   mode of updating the orientation refernce vector (acceleration). 
 * see orientation_reference_update_data_source_t for more details
 *  @param  threshold   Threshold of orientation change will generate interrupt (raw value) - 1 LSB = 8 mg
 *  @param  duration    Minimum duration of the new orientation will generate interrupt (raw value) - 1 LSB = 10ms
 */
void BMA400::ConfigureOrientationChangeInterrupt(
    bool enable,
    bool enableX, bool enableY, bool enableZ,
    interrupt_pin_t pin,
    orientation_change_data_source_t source,
    orientation_reference_update_data_source_t reference_update_mode,
    uint8_t threshold,
    uint8_t duration)
{
    if (!enable) //# Just disable the interrupt
    {
        unset(BMA400_REG_INT_CONFIG_0, 1);
        return;
    }

    //# enable the interrupt
    set(BMA400_REG_INT_CONFIG_0, 1);

    //# Wiring Interrupt to Interrupt pins
    LinkToInterruptPin(interrupt_source_t::ADV_ORIENTATION_CHANGE, pin);

    uint8_t val;
    switch (reference_update_mode)
    {
    case orientation_reference_update_data_source_t::ORIENT_UPDATE_MANUAL:
        val = 0x00;
        break;

    case orientation_reference_update_data_source_t::ORIENT_UPDATE_AUTO_ACC_FILT_2_100HZ:
        val = 0x04;
        break;

    case orientation_reference_update_data_source_t::ORIENT_UPDATE_AUTO_ACC_FILT_2_100HZ_LP_1HZ:
        val = 0x08;
        break;
    }

    if (source == orientation_change_data_source_t::ORIENT_UPDATE_ACC_FILT_2_100HZ_LP_1HZ)
        val |= 0x10;

    if (enableX)
        val |= 0x20;

    if (enableY)
        val |= 0x40;

    if (enableZ)
        val |= 0x80;

    write(BMA400_REG_ORIENT_CONFIG_0, val);
    write(BMA400_REG_ORIENT_CONFIG_1, threshold);
    write(BMA400_REG_ORIENT_CONFIG_3, duration);
}

/*!
 *  @brief  Configures Orientation Changed Interrupt
 *  @param  enable  set true to enable the interrupt
 *  @param  enableX enables interrupt on X Axis
 *  @param  enableY enables interrupt on Y Axis
 *  @param  enableZ enables interrupt on Z Axis
 *  @param  source  data source is used as input for the orientation change detection. 
 * see orientation_reference_update_data_source_t for mode details
 *  @param  reference_update_mode   mode of updating the orientation refernce vector (acceleration). 
 * see orientation_reference_update_data_source_t for more details
 *  @param  threshold   Threshold of orientation change will generate interrupt (in mg scale)
 *  @param  duration    Minimum duration of the new orientation will generate interrupt (in ms scale)
 */
void BMA400::ConfigureOrientationChangeInterrupt(
    bool enable,
    bool enableX, bool enableY, bool enableZ,
    interrupt_pin_t pin,
    orientation_change_data_source_t source,
    orientation_reference_update_data_source_t reference_update_mode,
    float threshold,
    float duration)
{
    if (!enable) //# Just disable the interrupt
    {
        unset(BMA400_REG_INT_CONFIG_0, 1);
        return;
    }

    //# enable the interrupt
    set(BMA400_REG_INT_CONFIG_0, 1);

    uint8_t val;
    switch (reference_update_mode)
    {
    case orientation_reference_update_data_source_t::ORIENT_UPDATE_MANUAL:
        val = 0x00;
        break;

    case orientation_reference_update_data_source_t::ORIENT_UPDATE_AUTO_ACC_FILT_2_100HZ:
        val = 0x04;
        break;

    case orientation_reference_update_data_source_t::ORIENT_UPDATE_AUTO_ACC_FILT_2_100HZ_LP_1HZ:
        val = 0x08;
        break;
    }

    if (source == orientation_change_data_source_t::ORIENT_UPDATE_ACC_FILT_2_100HZ_LP_1HZ)
        val |= 0x10;

    if (enableX)
        val |= 0x20;

    if (enableY)
        val |= 0x40;

    if (enableZ)
        val |= 0x80;

    write(BMA400_REG_ORIENT_CONFIG_0, val);

    threshold /= 8;
    val = threshold > 255 ? 255 : (uint8_t)threshold;
    write(BMA400_REG_ORIENT_CONFIG_1, val);

    duration /= 10;
    val = duration > 255 ? 255 : (uint8_t)duration;
    write(BMA400_REG_ORIENT_CONFIG_3, val);
}

/*!
 *  @brief  Sets reference vector(acceleration) for Orientation Changed Interrupt
 *  @param  values  Address of arrary 8bit values (length >= 6) includes the values as follows
 * X(LSB) X(MSB) Y(LSB) Y(MSB) Z(LSB) Z(MSB) 
 */
void BMA400::SetOrientationReference(uint8_t *values)
{
    uint8_t _register = BMA400_REG_ORIENT_CONFIG_4;
    for (uint8_t i = 0; i < 6; i++)
    {
        write(_register, values[i]);
        _register++;
    }
}

/*!
 *  @brief  Automatically Use current values to set reference vector(acceleration) for Orientation Changed Interrupt
 */
void BMA400::SetOrientationReference()
{
    uint8_t data[6] = {0};
    read(BMA400_REG_ACC_DATA, 6, data);
    uint8_t _register = BMA400_REG_ORIENT_CONFIG_4;
    for (uint8_t i = 0; i < 6; i++)
    {
        write(_register, data[i]);
        _register++;
    }
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
    wire->requestFrom(address, (uint8_t)1);
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