/**
 * The MySensors Arduino library handles the wireless radio link and protocol
 * between your home built sensors/actuators and HA controller of choice.
 * The sensors forms a self healing radio network with optional repeaters. Each
 * repeater and gateway builds a routing tables in EEPROM which keeps track of the
 * network topology allowing messages to be routed to nodes.
 *
 * Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
 * Copyright (C) 2013-2015 Sensnology AB
 * Full contributor list: https://github.com/mysensors/Arduino/graphs/contributors
 *
 * Documentation: http://www.mysensors.org
 * Support Forum: http://forum.mysensors.org
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 *******************************
 *
 * DESCRIPTION
 *
 * This is an example that demonstrates how to report the battery level for a sensor
 * Instructions for measuring battery capacity on A0 are available here:
 * http://www.mysensors.org/build/battery
 * 
 */


#include <SPI.h>
#include <MySensor.h>
#include <Wire.h>

#define DEV_ID 0x90 >> 1                        // shift required by wire.h
#define CHILD_ID_TEMP 1

int BATTERY_SENSE_PIN = A0;  // select the input pin for the battery sense point
boolean metric = true; 
MySensor gw;
MyMessage msgTemp(CHILD_ID_TEMP, V_TEMP);
unsigned long SLEEP_TIME = 180000;  // sleep time between reads (seconds * 1000 milliseconds)
int oldBatteryPcnt = 0;
float lastTemp = 0;


void setupDS1621()
{
  
    Wire.begin();
    Wire.beginTransmission(DEV_ID);             // connect to DS1621 (#0)
    Wire.write(0xAC);                            // Access Config
    Wire.write(0x02);                            // set for continuous conversion
    Wire.beginTransmission(DEV_ID);             // restart
    Wire.write(0xEE);                            // start conversions
    Wire.endTransmission();
}

float readDS1621()
{
    // get the temperature reading
    int8_t firstByte;
    int8_t secondByte;
    float temp = 0;
  
    delay(1000);                                // give time for measurement
    Wire.beginTransmission(DEV_ID);
    Wire.write(0xAA);                           // read temperature command
    Wire.endTransmission();
    Wire.requestFrom(DEV_ID, 2);                // request two bytes from DS1621 (0.5 deg. resolution)

    firstByte = Wire.read();                    // get first byte
    secondByte = Wire.read();                   // get second byte

    temp = firstByte;

    if (secondByte)             // if there is a 0.5 deg difference
        temp += 0.5;

    Serial.println(temp);
    return temp;
}

void setup()  
{
    // use the 1.1 V internal reference
    #if defined(__AVR_ATmega2560__)
      analogReference(INTERNAL1V1);
    #else
      analogReference(INTERNAL);
    #endif
    gw.begin();

    // Initialize DS1621 reading
    setupDS1621();

    // Send the sketch version information to the gateway and Controller
    gw.sendSketchInfo("DS1621", "1.0");

    // Register all sensors to gw (they will be created as child devices)
    gw.present(CHILD_ID_TEMP, S_TEMP);
  
    metric = gw.getConfig().isMetric;
}


void loop()
{
    float temperature;
    temperature =  readDS1621();

    if (temperature != lastTemp)
    {
        // get the temperature
        gw.send(msgTemp.set(temperature, 1));  
        lastTemp = temperature;  
    }
    
    Serial.print("T: ");
    Serial.println(temperature);

   // get the battery Voltage
   int sensorValue = analogRead(BATTERY_SENSE_PIN);
   #ifdef DEBUG
   Serial.println(sensorValue);
   #endif
   
   // 1M, 470K divider across battery and using internal ADC ref of 1.1V
   // Sense point is bypassed with 0.1 uF cap to reduce noise at that point
   // ((1e6+470e3)/470e3)*1.1 = Vmax = 3.44 Volts
   // 3.44/1023 = Volts per bit = 0.003363075
   float batteryV  = sensorValue * 0.003363075;
   int batteryPcnt = sensorValue / 10;

   #ifdef DEBUG
   Serial.print("Battery Voltage: ");
   Serial.print(batteryV);
   Serial.println(" V");

   Serial.print("Battery percent: ");
   Serial.print(batteryPcnt);
   Serial.println(" %");
   #endif

   if (oldBatteryPcnt != batteryPcnt) {
     // Power up radio after sleep
     gw.sendBatteryLevel(batteryPcnt);
     oldBatteryPcnt = batteryPcnt;
   }
   gw.sleep(SLEEP_TIME);
}
