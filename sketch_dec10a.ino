#define but_1 11                      // button 1 / Pin D11
#define but_2 12                      // button 2 / Pin D12

float humidity;
float temperature;
byte sensorPin = 0;
byte lightSensorPin = 2;		  
int sensorValue = 0;  
byte minHumidity = 50;
byte pumpOut = 10;
byte lampOut = 9;
byte buttonPin = 2;
unsigned long delayPump = 1500;
unsigned long delayPausePump = 60000;
unsigned long delayPauseLamp = 60000;
unsigned long lampPauseTime = 0;
unsigned long startTime = 0;
unsigned long pauseTime = 0;
bool pumpIsOn = false;
bool lampIsOn = false;
int currentButtonRegime = 0;
int timeToCheck = 0;

#include <avr/wdt.h>
#include<math.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define AHT10_ADDRESS 0x38

void setup() {
  Serial.begin(9600);
  Wire.begin();
  delay(100); // Allow sensor to boot

  // Initialize AHT10
  Wire.beginTransmission(AHT10_ADDRESS);
  Wire.write(0xE1); // Initialization command
  Wire.write(0x08); // Normal mode
  Wire.write(0x00); // Calibration enable
  Wire.endTransmission();
  
  delay(500); // Wait for sensor to calibrate

  pinMode(pumpOut, OUTPUT);
  pinMode(lampOut, OUTPUT);
  pinMode(but_1, INPUT_PULLUP); 
  pinMode(but_2, INPUT_PULLUP);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  // Clear the buffer
  display.clearDisplay();
  wdt_enable(WDTO_8S);
}


void loop() {

  getSensorDataAndUpdateScreen();

  if (lampIsOn) {
    digitalWrite(lampOut, HIGH);
    if (analogRead(lightSensorPin) > 750) {
      lampIsOn = false;
      digitalWrite(lampOut, LOW);
      lampPauseTime = millis();
    }
  } else {
    digitalWrite(lampOut, LOW);
  }
  if (analogRead(lightSensorPin) < 750 && (lampPauseTime + delayPauseLamp) < millis()) {
    lampIsOn = true;
  }

  if (pumpIsOn) {
    digitalWrite(pumpOut, HIGH);
    if (millis() > startTime + delayPump) {
      pumpIsOn = false;
      digitalWrite(pumpOut, LOW);
      pauseTime = millis();
    }
  } else {
    digitalWrite(pumpOut, LOW);
  }

  if(digitalRead(but_2)==LOW) {
    minHumidity=minHumidity+1;
    if (minHumidity > 100) {
      minHumidity = 0;
    } 
    //EEPROM.write(1,rollTrimMiddle/4); 
  }   

    if(digitalRead(but_1)==LOW) {
    if (minHumidity == 0) {
      minHumidity = 100;
    }
    if (minHumidity != 0) {
      minHumidity=minHumidity-1;
    }
    //EEPROM.write(1,rollTrimMiddle/4); 
  }   

  if ((humidity < minHumidity) && !pumpIsOn && (pauseTime + delayPausePump) < millis()) {
    pumpIsOn = true;
    startTime = millis();
    pauseTime = 0;
  }
  wdt_reset();
}

void getSensorDataAndUpdateScreen() {
// Trigger measurement
  Wire.beginTransmission(AHT10_ADDRESS);
  Wire.write(0xAC); // Trigger measurement command
  Wire.write(0x33); // Typical trigger values
  Wire.write(0x00);
  Wire.endTransmission();

  delay(80); // Wait for measurement to complete (typical ~75ms)

    // Request 6 bytes of data
  Wire.requestFrom(AHT10_ADDRESS, 6);
  if (Wire.available() == 6) {
    byte data[6];
    for (int i = 0; i < 6; i++) {
      data[i] = Wire.read();
    }

    // Check status bit (bit 7 of first byte)
    if ((data[0] & 0x80) == 0) {
      // Combine bytes into raw values
      uint32_t rawHumidity = ((uint32_t)(data[1]) << 12) |
                             ((uint32_t)(data[2]) << 4) |
                             ((data[3] & 0xF0) >> 4);
                             
      uint32_t rawTemperature = ((uint32_t)(data[3] & 0x0F) << 16) |
                                ((uint32_t)(data[4]) << 8) |
                                (uint32_t)(data[5]);

      // Convert to physical values
      humidity = ((float)rawHumidity / 1048576.0) * 100.0;
      temperature = ((float)rawTemperature / 1048576.0) * 200.0 - 50.0;
    } else {
      Serial.println("Sensor is busy or not ready.");
    }
  } else {
    Serial.println("Failed to read data from AHT10");
  }
  display.clearDisplay();
  display.setTextSize(1); 
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,0);
  display.print(F("Hum./Min, %: "));
  display.print(round(humidity));
  display.print("/");
  display.print(minHumidity);
  display.setCursor(0,10);
  display.print(F("Temp., C:     "));
  display.print(round(temperature));
  display.display();
  delay(200);
}
