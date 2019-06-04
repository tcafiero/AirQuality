/*
  Reading CO2, humidity and temperature from the SCD30
  By: Nathan Seidle
  SparkFun Electronics
  Date: May 22nd, 2018
  License: MIT. See license file for more information but you can
  basically do whatever you want with this code.

  Feel like supporting open source hardware?
  Buy a board from SparkFun! https://www.sparkfun.com/products/14751

  This example prints the current CO2 level, relative humidity, and temperature in C.

  Hardware Connections:
  If needed, attach a Qwiic Shield to your Arduino/Photon/ESP32 or other
  Plug the device into an available Qwiic port
  Open the serial monitor at 9600 baud to see the output
*/

#include <Wire.h>
#include <SoftwareSerial.h>
#include <TheThingsNetwork.h>
#include <rn2xx3.h>

// Set your AppEUI and AppKey
const char *appEui = "70B3D57ED001D030";
const char *appKey = "E0937927111C1D478DD8445AC75321A8";

// Set your DevAddr, NwkSKey, AppSKey and the frequency plan
const char *devAddr = "26011125";
const char *nwkSKey = "37E1A55712D3937095A61E2872A00ECB";
const char *appSKey = "365CC50E9B7BE3A5C1571415E23FC39D";




#define PIN_LORA_TX       (15)
#define PIN_LORA_RX       (14)
#define PIN_LORA_RESET    (16)

//#define PIN_LORA_RX       (9)
//#define PIN_LORA_TX       (10)
//#define PIN_LORA_RESET    (7)

SoftwareSerial Serial1(PIN_LORA_TX, PIN_LORA_RX); // MICRO_RX, MICRO_TX
#define loraSerial Serial1
#define debugSerial Serial

// Replace REPLACE_ME with TTN_FP_EU868 or TTN_FP_US915
#define freqPlan TTN_FP_EU868

//Click here to get the library: http://librarymanager/All#SparkFun_SCD30
#include "SparkFun_SCD30_Arduino_Library.h"

SCD30 airSensor;
TheThingsNetwork ttn(loraSerial, debugSerial, freqPlan);
rn2xx3 myLora(loraSerial);

void setup()
{
  Wire.begin();
  loraSerial.begin(9600);
  debugSerial.begin(57600);
  ttn.resetHard(PIN_LORA_RESET);

  //Autobaud the rn2483 module to 9600. The default would otherwise be 57600.
  ttn.autoBaud();

  //Wait a maximum of 10s for Serial Monitor
  while (!debugSerial && millis() < 10000)
      ;
  debugSerial.println("-- PERSONALIZE");
  ttn.personalize(devAddr, nwkSKey, appSKey);

  debugSerial.println("SCD30 Example");
  airSensor.begin(); //This will cause readings to occur every two seconds
  debugSerial.println("-- STATUS");
  ttn.showStatus();
}

void loop()
{
  byte payload[6];
  short int value;
  debugSerial.println("-- LOOP");
  if (airSensor.dataAvailable())
  {
    value = airSensor.getCO2();
    debugSerial.print("co2(ppm):");
    debugSerial.print(value);
    value *= 100;
    payload[0] = highByte(value);
    payload[1] = lowByte(value);

    value = airSensor.getTemperature();
    debugSerial.print(" temp(C):");
    debugSerial.print(value);
    value *= 100;
    payload[2] = highByte(value);
    payload[3] = lowByte(value);

    value = airSensor.getHumidity();
    debugSerial.print(" humidity(%):");
    debugSerial.print(value);
    value *= 100;
    payload[4] = highByte(value);
    payload[5] = lowByte(value);
    ttn.sendBytes(payload, sizeof(payload));
  }
  else
    debugSerial.println("No data");

  delay(20000);
}
