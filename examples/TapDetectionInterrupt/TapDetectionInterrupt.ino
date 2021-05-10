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
        BMA400::power_mode_t::NORMAL,
        BMA400::output_data_rate_t::Filter1_048x_200Hz,
        BMA400::acceleation_range_t::RANGE_2G);

    bma400.DisableInterrupts(); // disables all interrupts if previously set

    bma400.ConfigureTapInterrupt(
        true,                                                               // Enable Single tap
        true,                                                               // Enable Double tap
        BMA400::tap_axis_t::TAP_Z_AXIS,                                     // Tap detection on Z axis
        BMA400::interrupt_pin_t::INT_PIN_1,                                 // Trigger on Interrupt Pin 1
        BMA400::tap_sensitivity_level_t::TAP_SENSITIVITY_7,                 // Highest sensitivity
        BMA400::tap_max_pick_to_pick_interval_t::TAP_MAX_18_SAMPLES,        // no more than 18 samples between picks
        BMA400::tap_min_quiet_between_taps_t::MIN_QUIET_80_SAMPLES,         // at least 60 samples between to different taps
        BMA400::tap_min_quiet_inside_double_taps_t::MIN_QUIET_DT_12_SAMPLES // at least 12 samples between to taps in a double tap
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
    BMA400::interrupt_source_t source = bma400.GetInterrupts();
    if (source & BMA400::interrupt_source_t::ADV_SINGLE_TAP)
      printf("Single Detected.\r\n");
    else if (source & BMA400::interrupt_source_t::ADV_DOUBLE_TAP)
      printf("Double Detected.\r\n");

    newInterrupt = false;
  }
}
