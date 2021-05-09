# [Bosch BMA400](https://www.bosch-sensortec.com/products/motion-sensors/accelerometers/bma400/) driver

<p align="center">
  <img width="800px" src="https://www.bosch-sensortec.com/media/boschsensortec/products/motion_sensors/accelerometers/16_14/bosch-sensortec_website-relaunch_stage_bma400-16-9_res_800x450.jpg">
</p>

## What is supported

- Custom TwoWire interface (default is Wire)
- Auto address detect. `Initialize`
- Getting/Setting Power Mode (8 modes. see `power_mode_t`) `SetPowerMode` `GetPowerMode`
- Getting/Setting Acceleration data (processed in mg/unprocessed raw values) `ReadAcceleration`
- Getting/Setting Auto Low Power configurations. `ConfigureAutoLowPower` `SetAutoLowPowerOnDataReady` `SetAutoLowPowerOnGenericInterrupt1` `SetAutoLowPowerOnTimeout`
- Getting/Setting Output Data rate (16 rates. see `output_data_rate_t`). `SetDataRate` `GetDataRate`
- Getting/Setting Range. `SetRange` `GetRange`
- Configuring Generic Interrupts. `ConfigureGenericInterrupt`
- Configuring Activity Change Interrupt. `ConfigureActivityChangeInterrupt`
- Configuring Step detection Interrupt (as well as step counter). `ConfigureStepDetectorCounter`
- Reading and Reseting to total steps. `GetTotalSteps` `ResetStepCounter`
- Configuring Single & Double tap detection Interrupt. `ConfigureTapInterrupt`
- Configuring Orientention change Interrupt. `ConfigureOrientationChangeInterrupt`
- Setting Reference vector(acceleration) for Generic & Oreintation Changed Interrupts usig current/given values. `SetGenericInterruptReference` `SetOrientationReference`
- Mapping interrupts with the external interrupt pins. `LinkToInterruptPin`
- Reading the interrupt register. `GetInterrupts`
- Electrical Configuration of interrupt pins. `ConfigureInterruptPinSettings`
- Configuring the basic interrupts `ConfigureBasicInterrupts`
