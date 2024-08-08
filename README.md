# SmartBulbController
SmartBulbController is an open-source input device built for the ESP32 microcontroller for controlling Smart Bulbs using the TP-Link Smart Home Protocol.

The goal of this project was to create a remote control device for Kasa Smart Lighting system within local network in order to eliminate the need for a smart phone app for lighting control.

# Dependencies
Several libraries are required for the device to work. 
- Adafruit_SSD1306 by Adafruit
- Adafruit_BusIO by Adafruit
- Adafruit_GFX_Library by Adafruit
- Ai_Esp32_Rotary_Encoder by Igor Antolic
- KasaSmartDevice by Justin Ham
- ArduinoJson by Benoit Blanchon

# Designs

## Device Render
Device was designed with Fusion and prototype was printed using an Ender 3 Pro 3D Printer.

![Device Render](https://github.com/justin-ham61/SmartBulbController/blob/main/images/Case-Render.jpg)

## Inputs
- 2 Rotary Encoders
    - 1 Encoder for menu navigation
    - 1 Encoder for On/Off and brightness control
- 5 Switches (MX Style)
    - Quick toggle for individual bulbs
    
## Control Options
User can control their light bulbs in several different ways

1. Use the quick use rotary encoder
    - Press encoder button to toggle On/Off for all bulbs
    - Rotate encoder knob to change the brightness for all bulbs
2. Quck individual bulb toggle
    - The 5 switches allow the users to quickly turn on and off individual bulbs
3. Menu Select toggle
    - Navigate and select desired bulb on the menu in order to toggle and change the brightness of a specific bulb.  

# Set Up Guide

In order to flash the ESP32 module with the firmware, the user must update the config file with some information.
- Home Wifi SSID (name) and Password
- Names of all light bulbs they want
    - Name must match the name set for the bulb during set up exactly.
- Total number of bulbs
~~~c++
#ifndef CONFIG_H
#define CONFIG_H

const char* SSID = "<User WiFi SSID>";
const char* PASSWORD = "<User WiFi Password>";

//List of all user bulb names
char* aliases[] = {
  "<User Bulb Name 1>",
  "<User Bulb Name 2>",
  "<User Bulb Name 3>"
};

const int size = 3;

//Time between inactivity and sleep
const int inactivity_time = 15000;

//Minimum and Maximum brightness
const int min_brightness = 0;
const int max_brightness = 100;

#endif
~~~~

On start up, the ESP32 will begin to look for bulbs that are active and add them to the device's list of bulbs if the name is included in the config file.

For best results, please turn the smart bulb on with a phone during the set up.