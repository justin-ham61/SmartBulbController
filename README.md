# SmartBulbController
SmartBulbController is an open-source input device built for the ESP32 microcontroller for controlling Smart Bulbs using the TP-Link Smart Home Protocol.

The goal of this project was to create a remote control device for Kasa Smart Lighting system within local network in order to eliminate the need for a smart phone app for lighting control.

# Dependencies
Several libraries are required for the device to work. 
- Adafruit_SSD1306 by Adafruit
- Adafruit_BusIO by Adafruit
- Adafruit_GFX_Library by Adafruit
- Ai_Esp32_Rotary_Encoder by Igor Antolic
- KasaSmartPlug updated by Justin Ham
    - Modified version of the [KasaSmartPlug](https://github.com/kj831ca/KasaSmartPlug?tab=readme-ov-file) library by Kris Jearakul to be compatible with Smart Bulbs and Smart Plugs.
- ArduinoJson by Benoit Blanchon

# Features
- Quick communication with light bulbs using the TP-Link Smart Home Protocol which is a TCP/IP Protocol that sends JSON payloads to devices. 
- Rotary Knob allows the control of brightness for multiple bulbs at the same time which is not available in the Kasa app.
- Built on FreeRTOS for efficient and responsive device interaction.
- OLED Display for bulb management and view of device states such as (On, Off, Error).

# FreeRTOS
- WiFi related tasks are managed using Semaphores and prohibits reliant tasks from running unless a connection is successfully made.
- User inputs are handled with ISR's that interact with RTOS queues.
- All tasks only operate when items are added to the queue and executes them promptly.
- Tasks notify lower priority tasks such as display task in order to update display.

# Designs

## Device Render
Device was designed with Fusion and prototype was printed using an Ender 3 Pro 3D Printer.

![Device Render](https://github.com/justin-ham61/SmartBulbController/blob/main/images/Case-Render.jpg)

## First Working Prototype
First working prototype of the controller with a custom PCB and the 3D printed case.

![Prototype Image](https://github.com/justin-ham61/SmartBulbController/blob/main/images/Controller_Resized.jpg)

## OLED Display
- Menu
    - List of all bulbs and a dedicated "Reset" command for the ESP32 microcontroller. 
        - Although the firmware has auto update for dynamic IP assignment, it may have issue updating to the most recent IP address of the bulb.
        - The reset button can be used to restart the device and connect to the new IP.
- Bulbs show the current state of the bulb 
    - On
    - Off
    - Error (Exclamation icon)
- Changing the brightness will show the current brightness as a progress bar.

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

1. Gather all materials listed in the parts document, you can order the PCB using the provided files or wire the device with a breadboard following the schematics. 
2. Prepare config file before flashing the device (Instruction shown below).
3. Once the device is put together, flash the device with the firmware and required libraries using the Arduino IDE.
4. Device set up will run automatically on initial start up. 
    - This step includes the device searching for bulbs as outlined in the config file on the local wifi network.
    - Having the device connected to a computer and reading the Serial output will help with any connectivity issues. 
    - Notes about better connection below
5. Once set up, all lightbulbs named in the config file should show up on the main menu.
6. The first 5 lights can be toggled with the 5 switches.

## Config File Instruction
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

//Total number of bulbs, needs to match the number of bulb names above.
const int size = 0;

//Time between inactivity and sleep
const int inactivity_time = 15000;

//Minimum and Maximum brightness
const int min_brightness = 0;
const int max_brightness = 100;

#endif
~~~~

On start up, the ESP32 will begin to look for bulbs that are active and add them to the device's list of bulbs if the name is included in the config file.

For best results, please turn the smart bulb on with a phone during the set up.

# Future Plans
- Light temperature and color control.
- Custom group creations to group certain bulbs with another.
- Better rotary knob control between menu and controlling bulbs.
- Compatibility with devices that require Kasa Authentication such as light strips.
