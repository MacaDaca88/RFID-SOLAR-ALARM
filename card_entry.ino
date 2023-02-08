
/*
  Typical pin layout used:
  -----------------------------------------------------------------------------------------
              MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
              Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
  Signal      Pin          Pin           Pin       Pin        Pin              Pin
  -----------------------------------------------------------------------------------------
  RST/Reset   RST          9             46         D9         RESET/ICSP-5     RST
  SPI SS      SDA(SS)      10            53        D10        10               10
  SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
  SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
  SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15

  More pin layouts for other boards can be found here: https://github.com/miguelbalboa/rfid#pin-layout
*/

#include <DHT.h>
#include <DHT_U.h>
#include <SPI.h>
#include <MFRC522.h>
#include <LCD_I2C.h>
#include <Servo.h>
#include <EEPROM.h>
#include <FastLED_NeoPixel.h>
#define reed 7
#define RST_PIN 43
#define SS_PIN 53
#define red A3
#define green A4
#define blue A5
#define led A2
#define usb 4
#define DHTPIN 2  

#define DATA_PIN A1  // data line for led
#define NUM_LEDS 32  // leds used on this line

byte readCard[4];
char* myTags[100] = {};
int tagsCount = 0;
int loginCount = 0;
int a;  //LED strip
String tagID = "";
boolean successRead = false;
boolean correctTag = false;
int proximitySensor;
boolean doorOpened = false;
int trigger_pin = 2;
int echo_pin = 3;
int reedsw = digitalRead(7);
int masterbuttonval = digitalRead(5);

long distance, pulse_duration;
int i = 0;
unsigned long previousMillis = 0;
unsigned long currentMillis = millis();
const long interval = 1000;
uint32_t delayMS;

#define DHTTYPE    DHT11     // DHT 11
Adafruit_NeoPixel strip(NUM_LEDS, DATA_PIN, NEO_GRB);
MFRC522 mfrc522(SS_PIN, RST_PIN);
DHT_Unified dht(DHTPIN, DHTTYPE);
LCD_I2C lcd(0x27, 20, 4);
Servo myServo;             // Servo motor



void setup() {
  myServo.attach(22);  // Servo motor

  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  strip.begin();
  strip.setBrightness(255);
  lcd.begin();
  lcd.backlight();


  pinMode(trigger_pin, OUTPUT);
  pinMode(echo_pin, INPUT);
  pinMode(led, OUTPUT);
  pinMode(reed, INPUT_PULLUP);
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(blue, OUTPUT);
  pinMode(usb, OUTPUT);
  pinMode(20, INPUT_PULLUP);
  pinMode(21, INPUT_PULLUP);

  digitalWrite(trigger_pin, LOW);
  digitalWrite(red, HIGH);
  digitalWrite(green, HIGH);
  digitalWrite(blue, HIGH);
  digitalWrite(usb, LOW);
  digitalWrite(led, LOW);
    dht.begin();
  Serial.println(F("DHTxx Unified Sensor Example"));
  // Print temperature sensor details.
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  Serial.println(F("------------------------------------"));
  Serial.println(F("Temperature Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("°C"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("°C"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("°C"));
  Serial.println(F("------------------------------------"));
  // Print humidity sensor details.
  dht.humidity().getSensor(&sensor);
  Serial.println(F("Humidity Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("%"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("%"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("%"));
  Serial.println(F("------------------------------------"));
  // Set delay between sensor readings based on sensor details.
  delayMS = sensor.min_delay / 1000;

  myServo.write(0);  // Initiate locked position of the servo motor
  int lockbuttonval = digitalRead(6);
  lcd.setCursor(0, 0);
  lcd.print("Booting Up.....");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Scan Master");
  delay(1000);
  if (lockbuttonval == 0) {
    halt();
  } else {
    if (lockbuttonval == 1) {
    }
  }
}

void halt() {
  int masterbuttonval = digitalRead(5);
  int lockbuttonval = digitalRead(6);
  Serial.print(masterbuttonval);
  if (lockbuttonval == 0) {
    lcd.clear();
    lcd.print("Waiting For GOD");
    delay(1000);
  }
  if (masterbuttonval == 0) {
    master();
  }
}


void master() {


  int masterbuttonval = digitalRead(5);
  int lockbuttonval = digitalRead(6);
  delay(100);

  lcd.clear();
  lcd.print("-No Master Tag!-");
  lcd.setCursor(0, 1);
  lcd.print("SCAN NOW");
  // Waits until a master card is scanned
  while (!successRead) {
    successRead = getID();
    if (successRead == true) {
      myTags[tagsCount] = strdup(tagID.c_str());  // Sets the master tag into position 0 in the array
      lcd.clear();
      lcd.setCursor(0, 0);
      Serial.println("Master Tag Set!");
      lcd.print("Master Tag Set!");
      delay(1000);
      tagsCount++;
    }
  }
  successRead = false;
}



void unlock() {

  lcd.setCursor(0, 1);
  lcd.print(" NO NIGGERS!");
  for (int a = 0; a < 32; a++) {                         // data count for how many leds inline (change to suit how many leds you are using)
    strip.setPixelColor(a, strip.Color(255, 255, 255));  // set STRIP to red
    // needed to show changes
  }
  strip.show();
  myServo.write(180);  // unlocks the door
  delay(5000);
  for (int a = 0; a < 32; a++) {                   // data count for how many leds inline (change to suit how many leds you are using)
    strip.setPixelColor(a, strip.Color(0, 0, 0));  // set STRIP to red
    // needed to show changes
  }
  strip.show();
  scan();
}

void lock() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" DOOR LOCKED!");
  lcd.setCursor(0, 1);
  lcd.print(" NO NIGGERS!");
  myServo.write(0);  // locks the door
  lockenable();
}
void lockout() {

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" NO NIGGERS!");
  lcd.setCursor(0, 1);
  lcd.print(" NO DOGS!");
}
void lockenable() {
  lcd.clear();
  lcd.print("scan card");
  lcd.setCursor(0, 1);
  lcd.print(" to LOCK!");
  if (!mfrc522.PICC_IsNewCardPresent()) {  //If a new PICC placed to RFID reader continue
    return;
  }
  if (!mfrc522.PICC_ReadCardSerial()) {  //Since a PICC placed get Serial and continue
    return;
  }
  tagID = "";
  // The MIFARE PICCs that we use have 4 byte UID
  for (uint8_t i = 0; i < 4; i++) {  //
    readCard[i] = mfrc522.uid.uidByte[i];
    tagID.concat(String(mfrc522.uid.uidByte[i], HEX));  // Adds the 4 bytes in a single String variable
  }
  tagID.toUpperCase();
  mfrc522.PICC_HaltA();  // Stop reading

  successRead = false;
  // Checks whether the scanned tag is authorized
  for (int i = 0; i < 100; i++) {
    if (tagID == myTags[i]) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(" DOOR LOCKABLE!");
      lcd.setCursor(0, 1);
      lcd.print(tagID.c_str());
      digitalWrite(usb, LOW);
      correctTag = true;
      lock();
    }
  }
  if (correctTag == false) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(" Access LOCKOUT!");
    digitalWrite(usb, HIGH);
    lcd.setCursor(0, 1);
    lcd.print(tagID.c_str());
    lockout();
    daze();
  }
}

void add() {
  if (!mfrc522.PICC_IsNewCardPresent()) {  //If a new PICC placed to RFID reader continue
    return;
  }
  if (!mfrc522.PICC_ReadCardSerial()) {  //Since a PICC placed get Serial and continue
    return;
  }
  tagID = "";
  // The MIFARE PICCs that we use have 4 byte UID
  for (uint8_t i = 0; i < 4; i++) {  //
    readCard[i] = mfrc522.uid.uidByte[i];
    tagID.concat(String(mfrc522.uid.uidByte[i], HEX));  // Adds the 4 bytes in a single String variable
  }
  tagID.toUpperCase();
  mfrc522.PICC_HaltA();  // Stop reading
  if (tagID == myTags[0]) {
    lcd.clear();
    lcd.print("Program mode:");
    lcd.setCursor(0, 1);
    lcd.print("Add/Remove Tag");
    while (!successRead) {
      successRead = getID();
      if (successRead == true) {
        add();
      }
    }
    correctTag = false;
    for (int i = 0; i < 100; i++) {
      if (tagID == myTags[i]) {
        myTags[i] = "";
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("  Tag Removed!");
        delay(1000);
        printNormalModeMessage();
        return;
      }
    }
    myTags[tagsCount] = strdup(tagID.c_str());
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("   Tag Added!");
    lcd.setCursor(0, 1);
    lcd.print(tagID.c_str());
    delay(1000);
    printNormalModeMessage();
    tagsCount++;
    return;
  }
}
void scan() {
  if (!mfrc522.PICC_IsNewCardPresent()) {  //If a new PICC placed to RFID reader continue
    return;
  }
  if (!mfrc522.PICC_ReadCardSerial()) {  //Since a PICC placed get Serial and continue
    return;
  }
  tagID = "";
  // The MIFARE PICCs that we use have 4 byte UID
  for (uint8_t i = 0; i < 4; i++) {  //
    readCard[i] = mfrc522.uid.uidByte[i];
    tagID.concat(String(mfrc522.uid.uidByte[i], HEX));  // Adds the 4 bytes in a single String variable
  }
  tagID.toUpperCase();
  mfrc522.PICC_HaltA();  // Stop reading

  correctTag = false;
  // Checks whether the scanned tag is the master tag

  while (!successRead) {
    successRead = getID();
    if (successRead == true) {

      // Checks whether the scanned tag is authorized
      for (int i = 0; i < 100; i++) {
        if (tagID == myTags[0]) {
          lcd.clear();
          lcd.print("THE BOSS");
          lcd.setCursor(0, 1);
          lcd.print("AKA GOD");
          delay(1000);
        }
        if (tagID == myTags[i]) {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print(" Access Granted!");
          lcd.setCursor(0, 1);
          lcd.print(tagID.c_str());
          digitalWrite(usb, LOW);
          correctTag = true;
          unlock();
        }
      }
      if (correctTag == false) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(" Access Denied!");
        digitalWrite(usb, HIGH);
        lcd.setCursor(0, 1);
        lcd.print(tagID.c_str());
        lock();
        daze();
      }
    }
    successRead = false;
  }
}
void loop() {


  int masterbuttonval = digitalRead(5);
  int lockbuttonval = digitalRead(6);
  int reedsw = digitalRead(7);

  digitalWrite(trigger_pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigger_pin, LOW);

  pulse_duration = pulseIn(echo_pin, HIGH);

  distance = round(pulse_duration * 0.0171);
  // Delay between measurements.
  delay(delayMS);
  // Get temperature event and print its value.
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));
  }
  else {
    Serial.print(F("Temperature: "));
    Serial.print(event.temperature);
    Serial.println(F("°C"));
  }
  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println(F("Error reading humidity!"));
  }
  else {
    Serial.print(F("Humidity: "));
    Serial.print(event.relative_humidity);
    Serial.println(F("%"));
  }

  Serial.print("distance = ");
  Serial.print(distance);
  Serial.println("CM");
  Serial.print("reedsw = ");
  Serial.println(reedsw);
  Serial.print("lockbutton = ");
  Serial.println(lockbuttonval);
  Serial.print("masterbutton = ");
  Serial.println(masterbuttonval);
  Serial.println(EEPROM.read(1));

  if (reedsw == 1) {
    digitalWrite(led, HIGH);
  } else {
    if (reedsw == 0) {
      digitalWrite(led, LOW);
    }
  }
  if (lockbuttonval == 0) {
    lcd.clear();
    lcd.print("Ready ADD");
    add();
  }
  if (lockbuttonval == 1) {
    lcd.clear();
    lcd.print("Ready Scan");
    scan();
  }
}


uint8_t getID() {
  // Getting ready for Reading PICCs
  if (!mfrc522.PICC_IsNewCardPresent()) {  //If a new PICC placed to RFID reader continue
    return 0;
  }
  if (!mfrc522.PICC_ReadCardSerial()) {  //Since a PICC placed get Serial and continue
    return 0;
  }
  tagID = "";
  for (uint8_t i = 0; i < 4; i++) {  // The MIFARE PICCs that we use have 4 byte UID
    readCard[i] = mfrc522.uid.uidByte[i];
    tagID.concat(String(mfrc522.uid.uidByte[i], HEX));  // Adds the 4 bytes in a single String variable
  }
  tagID.toUpperCase();
  mfrc522.PICC_HaltA();  // Stop reading
  return 1;
}

void printNormalModeMessage() {

  lcd.clear();
  lcd.print("-Access Control-");
  lcd.setCursor(0, 1);
  lcd.print(" Scan Your Tag!");
}

void daze() {

  for (int a = 0; a < 32; a++) {                     // data count for how many leds inline (change to suit how many leds you are using)
    strip.setPixelColor(a, strip.Color(255, 0, 0));  // set STRIP to red
    strip.show();                                    // needed to show changes
  }
  delay(100);
  for (int a = 0; a < 32; a++) {                     // data count for how many leds inline (change to suit how many leds you are using)
    strip.setPixelColor(a, strip.Color(0, 0, 255));  // set STRIP to red
    strip.show();                                    // needed to show changes
  }
  delay(100);

  for (int a = 0; a < 32; a++) {                     // data count for how many leds inline (change to suit how many leds you are using)
    strip.setPixelColor(a, strip.Color(255, 0, 0));  // set STRIP to red
    strip.show();                                    // needed to show changes
  }
  delay(100);
  for (int a = 0; a < 32; a++) {                     // data count for how many leds inline (change to suit how many leds you are using)
    strip.setPixelColor(a, strip.Color(0, 0, 255));  // set STRIP to red
    strip.show();                                    // needed to show changes
  }
  delay(100);

  for (int a = 0; a < 32; a++) {                     // data count for how many leds inline (change to suit how many leds you are using)
    strip.setPixelColor(a, strip.Color(255, 0, 0));  // set STRIP to red
    strip.show();                                    // needed to show changes
  }
  delay(100);
  for (int a = 0; a < 32; a++) {                     // data count for how many leds inline (change to suit how many leds you are using)
    strip.setPixelColor(a, strip.Color(0, 0, 255));  // set STRIP to red
    strip.show();                                    // needed to show changes
  }
  delay(100);
  for (int a = 0; a < 32; a++) {                   // data count for how many leds inline (change to suit how many leds you are using)
    strip.setPixelColor(a, strip.Color(0, 0, 0));  // set STRIP to red
    strip.show();                                  // needed to show changes
  }
  delay(100);
}
