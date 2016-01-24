/* YourDuino.com Example Software Sketch
 20 character 4 line I2C Display
 Backpack Interface labelled "YwRobot Arduino LCM1602 IIC V1"
 Connect Vcc and Ground, SDA to A4, SCL to A5 on Arduino
 terry@yourduino.com */

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
 * Example sketch showing how to send in DS1820B OneWire temperature readings back to the controller
 * http://www.mysensors.org/build/temp
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
 * REVISION HISTORY
 * Version 1.0 - Henrik Ekblad
 * 
 * DESCRIPTION
 * Motion Sensor example using HC-SR501 
 * http://www.mysensors.org/build/motion
 *
 */


/*-----( Includes  )-----*/
#include <Wire.h>  // Comes with Arduino IDE
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <MySensor.h>  

/*-----( Declare Constants )-----*/
#define DIGITAL_INPUT_DALLAS 5   // Pin where the Dallas sensor is connected 
#define DIGITAL_INPUT_MOTION 3   // The digital input you attached your motion sensor.  (Only 2 and 3 generates interrupt!)
#define INTERRUPT DIGITAL_INPUT_MOTION-2 // Usually the interrupt = pin -2 (on uno/nano anyway)
#define CHILD_ID_TEMP 0
#define CHILD_ID_MOTION 1
#define CHILD_ID_LCD1 8                  // LCD line 1
#define CHILD_ID_LCD2 9                  // LCD line 2

/*-----( Declare objects )-----*/
// set the LCD address to 0x27 for a 20 chars 4 line display
// Set the pins on the I2C chip used for LCD connections:
//                    addr, en,rw,rs,d4,d5,d6,d7,bl,blpol
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address
OneWire oneWire(DIGITAL_INPUT_DALLAS); // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWire); // Pass the oneWire reference to Dallas Temperature.

const byte V_TEXT = 47 ;                  // values taken from development edition MyMessage.h
const byte S_INFO = 36 ;

char lastLCD1[21] = "              ";    // define & init before first receive
char lastLCD2[21] = "              ";

unsigned long SLEEP_TIME = 300000; // Sleep time between reads (in milliseconds)
boolean metric = true; 

MySensor gw;
MyMessage msgTemp(CHILD_ID_TEMP, V_TEMP);
MyMessage msgMotion(CHILD_ID_MOTION, V_TRIPPED);
MyMessage msgText(0, V_TEXT);


void setup()   /*----( SETUP: RUNS ONCE )----*/
{
  lcd.begin(16,4);         // initialize the lcd for 16 chars 2 lines

  //-------- Write characters on the display ------------------
  // NOTE: Cursor Position: Lines and Characters start at 0  
  lcd.clear();
  lcd.backlight();
  lcd.setCursor(0,0); //Start at character 0 on line 0
  lcd.print("Hi Carlos       ");
  lcd.setCursor(0,1); //Start at character 0 on line 1
  lcd.print("Hello world     ");
  delay(2500);  
  lcd.noBacklight();
  
  // Initialize  Sensors library
  // Startup up the OneWire library
  sensors.begin();
  // requestTemperatures() will not block current thread
  sensors.setWaitForConversion(false);

  // Initialize Motion Sensor
  pinMode(DIGITAL_INPUT_MOTION, INPUT);      // sets the motion sensor digital pin as input

  // Initialize library and add callback for incoming messages
  gw.begin(incomingMessage);
  
  // Send the sketch version information to the gateway and Controller
  gw.sendSketchInfo("LCD Display", "1.2");

  // Present the temperature sensor to the controller
  gw.present(CHILD_ID_TEMP, S_TEMP); gw.wait(500);
   
  // Present the motion sensor to the controller
  gw.present(CHILD_ID_MOTION, S_MOTION); gw.wait(500);

  // Present the LCD device
  gw.present(CHILD_ID_LCD1, S_INFO, "LCD line1"); gw.wait(500);     // new S_type 20150905 (not know by domoticz)
  gw.present(CHILD_ID_LCD2, S_INFO, "LCD line2"); gw.wait(500);
  gw.send(msgText.setSensor(CHILD_ID_LCD1).set("-"));    // initialize the V_TEXT at controller for sensor to none (trick for Domoticz)
  gw.send(msgText.setSensor(CHILD_ID_LCD2).set("-"));
 
}/*--(end setup )---*/


 
void loop()   /*----( LOOP: RUNS CONSTANTLY )----*/
{ 
  gw.process(); 

  // Request new LCD values from controller
  gw.request(CHILD_ID_LCD1, V_TEXT, 0);                  
  gw.request(CHILD_ID_LCD2, V_TEXT, 0);
  
  // Motion activity
  // Read digital motion value
  boolean tripped = digitalRead(DIGITAL_INPUT_MOTION) == HIGH; 
 
  // Fetch temperatures from Dallas sensors
  sensors.requestTemperatures();
  // query conversion time and sleep until conversion completed
  int16_t conversionTime = sensors.millisToWaitForConversion(sensors.getResolution());
  // sleep() call can be replaced by wait() call if node need to process incoming messages (or if node is repeater)
  gw.sleep(conversionTime);

  // Read temperatures and send them to controller 
  // Fetch and round temperature to one decimal
  float temperature = static_cast<float>(static_cast<int>((gw.getConfig().isMetric?sensors.getTempCByIndex(0):sensors.getTempFByIndex(0)) * 10.)) / 10.;

  gw.send(msgMotion.set(tripped?"1":"0"));  // Send tripped value to gw 

  LCD_local_display();
  lcd.backlight(); 
  if (temperature != -127.00 && temperature != 85.00) {
    lcd.setCursor(12, 0); 
    lcd.print(temperature);
    // Send the new temperature
    gw.send(msgTemp.setSensor(0).set(temperature,1));
  }


  gw.wait(10000);
  lcd.noBacklight();

  // Process incoming messages (like config from server)
  gw.process(); 
  
  // Sleep until interrupt comes in on motion sensor. Send update every two minute. 
  gw.sleep(INTERRUPT,CHANGE, SLEEP_TIME);

}/* --(end main loop )-- */


// This is called when a message is received 
void incomingMessage(const MyMessage &message) {
    if (message.type==V_TEXT) {                         // Text messages only
     // Write some debug info
     Serial.print("Message: "); Serial.print(message.sensor); Serial.print(", Message: "); Serial.println(message.getString());
    if (message.sensor == CHILD_ID_LCD1 ) {
        //snprintf(lastLCD1, sizeof(lastLCD1), "%16s", message.getString());  // load text into LCD string
        strcpy(lastLCD1, message.getString());
        }
    else if (message.sensor == CHILD_ID_LCD2){
        //snprintf(lastLCD2, sizeof(lastLCD2), "%16s", message.getString());
        strcpy(lastLCD2, message.getString());
        }
    } 
}


void LCD_local_display(void){
// take care of LCD display information
      lcd.setCursor(0, 0);  
      lcd.print(lastLCD1);                      
      lcd.setCursor(0, 1);
      lcd.print(lastLCD2);   
}

/* ( THE END ) */
