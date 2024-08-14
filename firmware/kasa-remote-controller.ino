#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "KasaSmartPlug.h"
#include "config.h"
#include "./structs/device.h"
#include "./structs/circ_buff.h"
#include "AiEsp32RotaryEncoder.h"
#include "AiEsp32RotaryEncoderNumberSelector.h"
#include "./structs/menu_item.h"
#include "Arduino.h"
#include "bitmap.h"
#include <queue>

//Font Imports for Display
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>

#define OLED_ADDR 0x3C
Adafruit_SSD1306 display(128,64, &Wire, -1);

//Menu Rotary Setting
#define MENU_ROTARY_ENCODER_DT_PIN      23
#define MENU_ROTARY_ENCODER_CLK_PIN     17
#define MENU_ROTARY_ENCODER_BUTTON_PIN  34

//Quick Rotary Setting
#define QUICK_ROTARY_ENCODER_DT_PIN     18
#define QUICK_ROTARY_ENCODER_CLK_PIN    16
#define QUICK_ROTARY_ENCODER_BUTTON_PIN 35
#define ROTARY_ENCODER_VCC_PIN          -1
#define ROTARY_ENCODER_STEPS            4

//Pin for waking up from deep sleep
#define SLEEP_WAKE_PIN          GPIO_NUM_34
#define SLEEP_WAKE_BITMASK      0xE005000

#define PIN_1 25
#define PIN_2 26
#define PIN_3 27
#define PIN_4 14
#define PIN_5 12

//Global Control Encoder Initializer
AiEsp32RotaryEncoder *rotaryEncoderQuick = new AiEsp32RotaryEncoder(QUICK_ROTARY_ENCODER_DT_PIN, QUICK_ROTARY_ENCODER_CLK_PIN, QUICK_ROTARY_ENCODER_BUTTON_PIN, ROTARY_ENCODER_VCC_PIN, ROTARY_ENCODER_STEPS);
AiEsp32RotaryEncoderNumberSelector numberSelectorQuick = AiEsp32RotaryEncoderNumberSelector();

//Menu Rotary Encoder Initializer
AiEsp32RotaryEncoder rotaryEncoderMenu = AiEsp32RotaryEncoder(MENU_ROTARY_ENCODER_DT_PIN, MENU_ROTARY_ENCODER_CLK_PIN, MENU_ROTARY_ENCODER_BUTTON_PIN, ROTARY_ENCODER_VCC_PIN, ROTARY_ENCODER_STEPS);

//Bulb state for determining if bulb is on or off, initialized to off
int bulb_state = 0;

//Initializing brightness to 50 and cofing for Encoder
int currBrightness = 49;
bool brightnessChanged = false;

int individualBrightness = 0;
bool individualBrightnessChanged = false;

unsigned long lastChanged = 0;

bool button = false;

//Debounce delay for handling button bounce
unsigned long lastDebounceTime = 0; 
const unsigned long debounceDelay = 500;

//Which display is currently being shown
//0: Main Menu
//1: Device Brightness Screen
//2: All device management screen
int displayType = 0;

//Menu Item selection variables
MenuItem menuItems[size + 1];

int numberOfItems;
int selectedItem = 0;
int previousItem;
int nextItem;
KASASmartBulb* currentBulb;

//Set up last action for sleep timer
unsigned long lastInteractedWith = 0;
bool asleep = false;

//Variable for auto updating ip address;
unsigned long lastUpdated = 0;

//Button states
volatile bool buttonPressed[] = {false, false, false, false, false};

#define BUTTON_BIT_MASK_1 (0x1 << 0)
#define BUTTON_BIT_MASK_2 (0x1 << 1)
#define BUTTON_BIT_MASK_3 (0x1 << 2)
#define BUTTON_BIT_MASK_4 (0x1 << 3)
#define BUTTON_BIT_MASK_5 (0x1 << 4)
#define FLAG_BIT_MASK (0x1 << 7)
volatile int button_state;
uint8_t data_space[64];
volatile circ_buff_t button_buffer = {data_space, 0, 0, 64};
int button_data;

int circ_buff_push(volatile circ_buff_t *c, int data)
{
    int next;

    next = c->head + 1;  // next is where head will point to after this write.
    if (next >= c->maxlen)
        next = 0;

    if (next == c->tail)  // if the head + 1 == tail, circular buffer is full
        return -1;

    c->buffer[c->head] = data;  // Load data and then move
    c->head = next;             // head to next data offset.
    return 0;  // return success to indicate successful push.
}

int circ_buff_pop(volatile circ_buff_t *c, int *data)
{
    int next;

    if (c->head == c->tail)  // if the head == tail, we don't have any data
        return -1;

    next = c->tail + 1;  // next is where tail will point to after this read.
    if(next >= c->maxlen)
        next = 0;

    *data = c->buffer[c->tail];  // Read data and then move
    c->tail = next;              // tail to next offset.
    return 0;  // return success to indicate successful push.
}

KASAUtil kasaUtil;

void IRAM_ATTR handleButton1() {
  if(asleep){
    return;
  }
  unsigned long currentMillis = millis();
  if (currentMillis - lastDebounceTime > debounceDelay) {
    button_state |= BUTTON_BIT_MASK_1;
    button_state |= FLAG_BIT_MASK;
    circ_buff_push(&button_buffer, 0);
  }
  lastDebounceTime = currentMillis;
}

void IRAM_ATTR handleButton2() {
  if(asleep){
    return;
  }
  unsigned long currentMillis = millis();
  if (currentMillis - lastDebounceTime > debounceDelay) {
    button_state |= BUTTON_BIT_MASK_2;
    button_state |= FLAG_BIT_MASK;
    circ_buff_push(&button_buffer, 1);
  }
  lastDebounceTime = currentMillis;
}

void IRAM_ATTR handleButton3() {
  if(asleep){
    return;
  }
  unsigned long currentMillis = millis();
  if (currentMillis - lastDebounceTime > debounceDelay) {
    button_state |= BUTTON_BIT_MASK_3;
    button_state |= FLAG_BIT_MASK;
    circ_buff_push(&button_buffer, 2);
    
  }
  lastDebounceTime = currentMillis;
}

void IRAM_ATTR handleButton4() {
  if(asleep){
    return;
  }
  unsigned long currentMillis = millis();
  if (currentMillis - lastDebounceTime > debounceDelay) {
    button_state |= BUTTON_BIT_MASK_4;
    button_state |= FLAG_BIT_MASK;
    circ_buff_push(&button_buffer, 3);
  }
  lastDebounceTime = currentMillis;
}

void IRAM_ATTR handleButton5() {
  if(asleep){
    return;
  }
  unsigned long currentMillis = millis();
  if (currentMillis - lastDebounceTime > debounceDelay) {
    button_state |= BUTTON_BIT_MASK_5;
    button_state |= FLAG_BIT_MASK;
    circ_buff_push(&button_buffer, 4);
  }
  lastDebounceTime = currentMillis;
}

void addFromConfig(UserDevice userdevices[]){
    for(int i = 0; i < 4; i++){
        kasaUtil.CreateDevice(userdevices[i].name, userdevices[i].ip, userdevices[i].type);
    }
} 

void updateLastInteractedWith(){
  for(int i = 0; i < size; i++){
    if(kasaUtil.GetSmartPlugByIndex(i)->err_code == 1){
      menuItems[i].icon = 2;
    } else if (kasaUtil.GetSmartPlugByIndex(i)->state == 1){
      menuItems[i].icon = 1;
    } else {
      menuItems[i].icon = 0;
    }
  }
  lastInteractedWith = millis();
}

void connectToWifi(){
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println(".");
  }
  Serial.println("Connected to the WiFi network");
}

void quick_rotary_onButtonClick(){
  unsigned long currentTime = millis();
  if ((currentTime - lastDebounceTime) > debounceDelay) {
    // Set the button pressed flag
    kasaUtil.ToggleAll(bulb_state);
    if(bulb_state == 0){
      bulb_state = 1;
    } else {
      bulb_state = 0;
    }
    // Update the last debounce time
    lastDebounceTime = currentTime;
  }
}

void menu_rotary_onButtonClick(){
  unsigned long currentTime = millis();
  if ((currentTime - lastDebounceTime) > debounceDelay) {
    if(selectedItem == numberOfItems + 1){
      ESP.restart();
    } else {
      displayType = 1;
      currentBulb = static_cast<KASASmartBulb*>(kasaUtil.GetSmartPlugByIndex(selectedItem));
      individualBrightness = currentBulb->brightness;
      numberSelectorQuick.setValue(individualBrightness);
      lastChanged = millis();
      // Update the last debounce time
      lastDebounceTime = currentTime;
    }
  }
}

void individual_brightness_onButtonClick(){
  unsigned long currentTime = millis();
  if((currentTime - lastDebounceTime) > debounceDelay){
    int currentState = currentBulb->state;
    if(currentState == 0){
      currentBulb->turnOn();
    } else {
      currentBulb->turnOff();
    }
  }
}

void quick_rotary_loop(){
    //dont print anything unless value changed
    if (rotaryEncoderQuick->encoderChanged())
    {
      displayType = 2;
      currBrightness = numberSelectorQuick.getValue();
      brightnessChanged = true;
      lastChanged = millis();
      updateLastInteractedWith();
    }
    if (rotaryEncoderQuick->isEncoderButtonClicked())
    {
      quick_rotary_onButtonClick();
      updateLastInteractedWith();
    }
}

void individual_brightness_rotary_loop(){
  if(rotaryEncoderQuick->encoderChanged()){
    individualBrightnessChanged = true;
    individualBrightness = numberSelectorQuick.getValue();
    lastChanged = millis();
    updateLastInteractedWith();
  }
  if(rotaryEncoderQuick->isEncoderButtonClicked()){
    individual_brightness_onButtonClick();
    updateLastInteractedWith();
  }
}

void menu_rotary_loop(){
    //dont print anything unless value changed
    if (rotaryEncoderMenu.encoderChanged())
    {
      selectedItem = rotaryEncoderMenu.readEncoder();
      updateLastInteractedWith();
    }
    if (rotaryEncoderMenu.isEncoderButtonClicked())
    {
      menu_rotary_onButtonClick();
      updateLastInteractedWith();
    }
}

void handleBrightnessChange(){
  //Change the brightness for all bulbs after .4 seconds of not turning the knob
  if(brightnessChanged && (millis() - lastChanged) > 400){
    brightnessChanged = false;
    kasaUtil.SetBrightnessAll(currBrightness);
    for(int i = 0; i < numberOfItems; i++){
      KASADevice* dev = kasaUtil.GetSmartPlugByIndex(i);
      KASASmartBulb* bulb = static_cast<KASASmartBulb*>(dev);
      bulb->brightness = currBrightness;
    }
  }

  //Return to main menu after 3 seconds of no activity on all brightness change
  if(millis() - lastChanged > 3000){
    displayType = 0;
  }
}

void handleIndividualBrightnessChange(){
  if(individualBrightnessChanged && (millis() - lastChanged) > 400){
    individualBrightnessChanged = false;
    currentBulb->setBrightness(individualBrightness);
  }
  if(millis() - lastChanged > 3000){
    displayType = 0;
  }
}

void IRAM_ATTR readEncoderISRQuick(){
  rotaryEncoderQuick->readEncoder_ISR();
}

void IRAM_ATTR readEncoderISRMenu(){
  rotaryEncoderMenu.readEncoder_ISR();
}

void main_menu_display_loop(){
  previousItem = selectedItem - 1;
  if(previousItem < 0){
    previousItem = numberOfItems;
  }
  nextItem = selectedItem + 1;
  if(nextItem >= numberOfItems + 1){
    nextItem = 0;
  }

  display.clearDisplay();

  display.drawBitmap(4,2,bitmap_array[menuItems[previousItem].icon],16,16,1);
  display.drawBitmap(4,24,bitmap_array[menuItems[selectedItem].icon],16,16,1);
  display.drawBitmap(4,46,bitmap_array[menuItems[nextItem].icon],16,16,1);

  display.drawBitmap(0, 22, bitmap_item_sel_background, 128, 21, 1);
  display.drawBitmap(120, 0, bitmap_scrollbar_background, 8, 64, 1);

  display.setTextSize(1);
  display.setTextColor(WHITE);

  //Previous Item
  display.setFont(&FreeSans9pt7b);
  display.setCursor(26, 15);
  display.print(menuItems[previousItem].name);

  //Current Item
  display.setFont(&FreeSansBold9pt7b);
  display.setCursor(26, 37);
  display.print(menuItems[selectedItem].name);

  //Next Item
  display.setFont(&FreeSans9pt7b);
  display.setCursor(26, 59);
  display.print(menuItems[nextItem].name);

  //Scroll position box
  display.fillRect(125, (64/(numberOfItems + 1)) * selectedItem, 3, (64/(numberOfItems + 1)), WHITE);

  //Display
  display.display();
}

void all_brightness_display_loop(){
  display.clearDisplay();
  display.setFont();
  display.setCursor(20, 20);
  display.print("Brightness: ");
  display.print(currBrightness);
  display.print("%");
  int barWidth = map(currBrightness, 0, 100, 0, 100);
  display.drawRect(14, 40, 100, 10, WHITE); // Draw the outline of the bar
  display.fillRect(14, 40, barWidth, 10, WHITE); // Fill the bar according to currBrightness
  display.display();
}

void individual_brightness_display_loop(){
  display.clearDisplay();
  display.setFont();
  display.setCursor(20, 20);
  display.print("Brightness: ");
  display.print(individualBrightness);
  display.print("%");
  int barWidth = map(individualBrightness, 0, 100, 0, 100);
  display.drawRect(14, 40, 100, 10, WHITE); // Draw the outline of the bar
  display.fillRect(14, 40, barWidth, 10, WHITE); // Fill the bar according to currBrightness
  display.display();
}

void sleep_loop(){
  if((millis() - lastInteractedWith) > inactivity_time){
    Serial.println("Going to sleep now");
    display.clearDisplay();
    display.display();
    delay(1000);
    asleep = true;
    esp_light_sleep_start();


    //Set up after waking up
    lastInteractedWith = millis();
    if(WiFi.status() != WL_CONNECTED){
      Serial.println("Attempting to reconnect to WiFi");
      connectToWifi();
    } else {
      Serial.println("WiFi is still connected properly");
    }

    delay(200);
    asleep = false;
    Serial.println("Waking up");
  }
}

void quickToggle(int index){
  currentBulb = static_cast<KASASmartBulb*>(kasaUtil.GetSmartPlugByIndex(index));
  Serial.print("Toggling: ");
  Serial.println(currentBulb->alias);
  individualBrightness = currentBulb->state;
  int currentState = currentBulb->state;
  if(currentState == 0){
    currentBulb->turnOn();
  } else {
    currentBulb->turnOff();
  }
  updateLastInteractedWith();
}


void button_loop(){
  if(button_state & FLAG_BIT_MASK){
    circ_buff_pop(&button_buffer, &button_data);
    button_state &= ~(0x1 << button_data);
    button_state &= ~FLAG_BIT_MASK;
    quickToggle(button_data);
  }
}

void update_loop(){
  unsigned long curr = millis();
  if((curr - lastUpdated) > 7200000 && bulb_state == 1){
    kasaUtil.ScanDevicesAndAdd(1000, aliases, size);
    lastUpdated = curr;
  }
}

void setup() {
  //Initialize serial
  Serial.begin(115200);


  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setFont();
  display.display(); // Display the message
  display.clearDisplay();
  //Connect to wifi with ssid and password from config.h
  connectToWifi();

  //Quick Rotary encoder set up
  rotaryEncoderQuick->begin();
  rotaryEncoderQuick->setup(readEncoderISRQuick);
  numberSelectorQuick.attachEncoder(rotaryEncoderQuick);
  numberSelectorQuick.setRange(1, 99, 2, false, 0);
  numberSelectorQuick.setValue(currBrightness);
  rotaryEncoderQuick->disableAcceleration();

  //Menu Rotary encoder set up
  rotaryEncoderMenu.begin();
  rotaryEncoderMenu.setup(readEncoderISRMenu);
  rotaryEncoderMenu.setBoundaries(0,size,true);
  rotaryEncoderMenu.disableAcceleration();

  //Quick button pin setup
  pinMode(PIN_1, INPUT_PULLDOWN);
  pinMode(PIN_2, INPUT_PULLDOWN);
  pinMode(PIN_3, INPUT_PULLDOWN);
  pinMode(PIN_4, INPUT_PULLDOWN);
  pinMode(PIN_5, INPUT_PULLDOWN);

  attachInterrupt(digitalPinToInterrupt(PIN_1), handleButton1, RISING);
  attachInterrupt(digitalPinToInterrupt(PIN_2), handleButton2, RISING);
  attachInterrupt(digitalPinToInterrupt(PIN_3), handleButton3, RISING);
  attachInterrupt(digitalPinToInterrupt(PIN_4), handleButton4, RISING);
  attachInterrupt(digitalPinToInterrupt(PIN_5), handleButton5, RISING);

  //Initiate devices from pre defined IP addresses and aliases
  //addFromConfig(devices);
  numberOfItems = kasaUtil.ScanDevicesAndAdd(1000, aliases, size);
  for(int i = 0; i < numberOfItems; i++){
    menuItems[i] = {kasaUtil.GetSmartPlugByIndex(i)->alias, 0};
  }
  menuItems[numberOfItems] = {"Reset", 3};

  //Display set up

  //Sleep check
  esp_sleep_enable_ext0_wakeup(SLEEP_WAKE_PIN, 0);
  esp_sleep_enable_ext1_wakeup(SLEEP_WAKE_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);
}

void loop() {
  if(asleep == false){
    menu_rotary_loop();
    if(displayType == 0){
      main_menu_display_loop();
      quick_rotary_loop();
    } else if (displayType == 1){
      individual_brightness_display_loop();
      individual_brightness_rotary_loop();
      handleIndividualBrightnessChange();
    } else if (displayType == 2){
      all_brightness_display_loop();
      quick_rotary_loop();
      handleBrightnessChange();
    }
    button_loop();
    sleep_loop();
    update_loop();
  }
}
 