#pragma once
#include <Arduino.h>
#include <Wire.h>

#define BMA400_REG_CHIP_ID 0x00

#define BMA400_ADDRESS_PRIMARY 0x14
#define BMA400_ADDRESS_SECONDARY 0x15

#define BMA400_CHIP_ID 0x90

class BMA400
{
public:
    bool Initialize(TwoWire &_wire = Wire);
    bool Initialize(uint8_t _address, TwoWire &_wire = Wire);

private:
    uint8_t address;
    TwoWire *wire;

    void read(uint8_t _register, uint8_t length, uint8_t *values);
    uint8_t read(uint8_t _register);
    void write(uint8_t _register, uint8_t _value);
};
