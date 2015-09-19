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
 * REVISION HISTORY
 * Version 1.0 - Henrik EKblad
 * 
 * DESCRIPTION
 * This sketch provides an example how to implement a humidity/temperature
 * sensor using DHT11/DHT-22 
 * http://www.mysensors.org/build/humidity
 */
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
#include <DHT.h>  

#define CHILD_ID_HUM 0
#define CHILD_ID_TEMP 1
#define HUMIDITY_SENSOR_DIGITAL_PIN 5
unsigned long SLEEP_TIME = 12000; // Sleep time between reads (in milliseconds)

int BATTERY_SENSE_PIN = A0;  // select the input pin for the battery sense point
int oldBatteryPcnt = 0;

MySensor gw;
DHT dht;
float lastTemp;
float lastHum;
boolean metric = true; 
MyMessage msgHum(CHILD_ID_HUM, V_HUM);
MyMessage msgTemp(CHILD_ID_TEMP, V_TEMP);


void setup()  
{ 
  // use the 1.1 V internal reference
  #if defined(__AVR_ATmega2560__)
     analogReference(INTERNAL1V1);
  #else
     analogReference(INTERNAL);
  #endif

  gw.begin();
  dht.setup(HUMIDITY_SENSOR_DIGITAL_PIN); 

  // Send the Sketch Version Information to the Gateway
  gw.sendSketchInfo("TempHum", "1.0");

  // Register all sensors to gw (they will be created as child devices)
  gw.present(CHILD_ID_HUM, S_HUM);
  gw.present(CHILD_ID_TEMP, S_TEMP);
  
  metric = gw.getConfig().isMetric;  
}

void loop()      
{  
  delay(dht.getMinimumSamplingPeriod());
  
  float temperature = dht.getTemperature();
  if (isnan(temperature)) {
      Serial.println("Failed reading temperature from DHT");
  } else if (temperature != lastTemp) {
    lastTemp = temperature;
    if (!metric) {
      temperature = dht.toFahrenheit(temperature);
    }
    gw.send(msgTemp.set(temperature, 1));
  }
  
  float humidity = dht.getHumidity();
  if (isnan(humidity)) {
      Serial.println(". Failed reading humidity from DHT");
  } else if (humidity != lastHum) {
      lastHum = humidity;
      gw.send(msgHum.set(humidity, 1));
  }

  Serial.print("T: ");
  Serial.print(temperature);
  Serial.print("; H: ");
  Serial.println(humidity);

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
   
  gw.sleep(SLEEP_TIME); //sleep a bit
}


