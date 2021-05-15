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

    Wire.begin(GPIO_NUM_21, GPIO_NUM_22, 400000);

    if (bma400.Initialize()) // Using default (Wire) interface & automatically resolving the address
    {
        printf("BMA400 Sensor successfully found\r\n");

        bma400.Setup(
            BMA400::power_mode_t::NORMAL,
            BMA400::output_data_rate_t::Filter2_100Hz,
            BMA400::acceleation_range_t::RANGE_2G);

        bma400.DisableInterrupts(); // disables all interrupts if previously set

        bma400.ConfigureStepDetectorCounter(
            true,                              // Enable Single tap
            BMA400::interrupt_pin_t::INT_PIN_1 // Trigger on Interrupt Pin 1
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
        if (source & BMA400::interrupt_source_t::ADV_STEP_DETECTOR_COUNTER)
            printf("Step Detected.\r\n");
        else if (source & BMA400::interrupt_source_t::ADV_STEP_DETECTOR_COUNTER_DOUBLE_STEP)
            printf("Double Step Detected.\r\n");

        newInterrupt = false;
    }
}
