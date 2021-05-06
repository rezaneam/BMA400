#include <BMA400.h>

bool BMA400::Initialize(TwoWire &_wire)
{
    wire = &_wire;
    address = BMA400_ADDRESS_PRIMARY;

    if (read(BMA400_REG_CHIP_ID) == BMA400_CHIP_ID)
        return true;
    address = BMA400_ADDRESS_SECONDARY;

    return (read(BMA400_REG_CHIP_ID) == BMA400_CHIP_ID);
}

bool BMA400::Initialize(uint8_t _address, TwoWire &_wire)
{
    wire = &_wire;
    address = _address;
    return (read(BMA400_REG_CHIP_ID) == BMA400_CHIP_ID);
}

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

bool BMA400::GetAutoLowPowerOnDataReady()
{
    return (read(BMA400_REG_AUTO_LOW_POW_1) & 0x01) == 1;
}

bool BMA400::GetAutoLowPowerOnGenericInterrupt1()
{
    return (read(BMA400_REG_AUTO_LOW_POW_1) & 0x02) == 2;
}

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

float BMA400::GetAutoLowPowerOnTimeoutThreshold()
{
    float threshold;
    threshold = (read(BMA400_REG_AUTO_LOW_POW_1) & 0xF0) >> 4;
    threshold += read(BMA400_REG_AUTO_LOW_POW_0) << 4;
    threshold *= 2.5;
    return threshold;
}

void BMA400::SetAutoLowPowerOnDataReady(bool enable)
{
    uint8_t val = read(BMA400_REG_AUTO_LOW_POW_1) & 0xFE;
    if (enable)
        val |= 0x01;
    write(BMA400_REG_AUTO_LOW_POW_1, val);
}

void BMA400::SetAutoLowPowerOnGenericInterrupt1(bool enable)
{
    uint8_t val = read(BMA400_REG_AUTO_LOW_POW_1) & 0xFD;
    if (enable)
        val |= 0x02;
    write(BMA400_REG_AUTO_LOW_POW_1, val);
}

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