# SmartBulbController
SmartBulbController is an open-source input device built for the ESP32 microcontroller for controlling Smart Bulbs using the TP-Link Smart Home Protocol.

The goal of this project was to create a remote control device for Kasa Smart Lighting system within local network in order to eliminate the need for a smart phone app for lighting control.

# Dependencies
Several libraries are required for the device to work. 
Adafruit_SSD1306 by Adafruit
Adafruit_BusIO by Adafruit
Adafruit_GFX_Library by Adafruit
Ai_Esp32_Rotary_Encoder by Igor Antolic
KasaSmartDevice by Justin Ham
ArduinoJson by Benoit Blanchon

# Designs

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

