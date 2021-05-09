#include <Arduino.h>
#include <BMA400.h>
#include <Wire.h>

BMA400 bma400;
void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);

  Wire.begin(GPIO_NUM_21, GPIO_NUM_22, 400000);

  if (bma400.Initialize()) // Automatically resolving the address
  {
    printf("BMA400 Sensor successfully found\r\n");
    bma400.Setup(
        BMA400::power_mode_t::NORMAL,
        BMA400::output_data_rate_t::Filter1_048x_200Hz,
        BMA400::acceleation_range_t::RANGE_4G);
  }
  else
    printf("Error! no BMA400 sensor found\r\n");
}

void loop()
{
  // put your main code here, to run repeatedly:
  float acceleration[3] = {0};
  bma400.ReadAcceleration(acceleration);
  printf("Acceleration(g) [X, Y, Z] = %2.2f %2.2f %2.2f\r\n", acceleration[0], acceleration[1], acceleration[2]);
  delay(1000);
}
