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

void BMA400::write(uint8_t _register, uint8_t value)
{
    wire->beginTransmission(address);
    wire->write((uint8_t)_register);
    wire->write((uint8_t)value);
    wire->endTransmission();
}