#include <Arduino.h>
#include <BMA400.h>
#include <Wire.h>

#define MOTION_DETECT_INT_PIN GPIO_NUM_37
BMA400 bma400;

bool newInterrupt;
portMUX_TYPE weakupInterruptPinMux = portMUX_INITIALIZER_UNLOCKED;
void IRAM_ATTR handleWeakupExternalInterrupt()
{
  portENTER_CRITICAL_ISR(&weakupInterruptPinMux);
  newInterrupt = true;
  portEXIT_CRITICAL_ISR(&weakupInterruptPinMux);
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);

  pinMode(MOTION_DETECT_INT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(MOTION_DETECT_INT_PIN), handleWeakupExternalInterrupt, FALLING);

  Wire1.begin(GPIO_NUM_21, GPIO_NUM_22, 400000);

  if (bma400.Initialize(Wire1)) // Using user defined (Wire1) interface & automatically resolving the address
  {
    printf("BMA400 Sensor successfully found\r\n");

    bma400.Setup(
        BMA400::power_mode_t::NORMAL_LOW_NOISE,
        BMA400::output_data_rate_t::Filter2_100Hz,
        BMA400::acceleation_range_t::RANGE_2G);

    bma400.DisableInterrupts(); // disables all interrupts if previously set

    bma400.ConfigureGenericInterrupt(
        BMA400::interrupt_source_t::ADV_GENERIC_INTERRUPT_1,                           // Generic Interrupt 1
        true,                                                                          // Enable Interrupt
        BMA400::interrupt_pin_t::INT_PIN_1,                                            // Link to INT Pin 1
        BMA400::generic_interrupt_reference_update_t::EVERYTIME_UPDATE_FROM_ACC_FILTx, // Update reference values from Filter2 automatically
        BMA400::generic_interrupt_mode_t::ACTIVITY_DETECTION,                          // Generate Interrupt on finding activity
        (float)100,                                                                    // 100mg
        (float)20,                                                                     // 20 ms
        BMA400::generic_interrupt_hysteresis_amplitude_t::AMP_48mg,                    // Hysteresis amplitude
        BMA400::interrupt_data_source_t::ACC_FILT_2,                                   // Data source for the interrupt
        true,                                                                          // Monitor X Axis
        true,                                                                          // Monitor Y Axis
        true,                                                                          // Monitor Z Axis
        false,                                                                         // Any of the axes can generate interrupt independenty
        false                                                                          // false = automatically increases data rate to 100Hz if it is lower than 100Hz
    );

    bma400.ConfigureInterruptPinSettings(
        false, // Disable Latch
        false, // Interrupt Pin 1 active low
        false, // Interrupt Pin 1 in push-pull mode
        false, // Interrupt Pin 2 active low
        false  // Interrupt Pin 2 in push-pull mode
    );

    while (bma400.GetInterrupts()) // make sure there is no interrupt on the queue
      ;
  }
  else
    printf("Error! no BMA400 sensor found\r\n");
}

void loop()
{
  if (newInterrupt)
  {
    if (bma400.GetInterrupts() & BMA400::interrupt_source_t::ADV_GENERIC_INTERRUPT_1)
      printf("Motion Detected.\r\n");
    newInterrupt = false;
  }
}
