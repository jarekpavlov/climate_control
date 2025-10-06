#define but_1 11                      // button 1 / Pin D11
#define but_2 12                      // button 2 / Pin D12

float humidity;
float temprature;
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

#include <avr/wdt.h>
#include <AHT20.h>
#include<math.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

AHT20 aht20;


void setup() {
  Serial.begin(9600);
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

  if (aht20.begin() == false)
  {
    Serial.println("AHT20 not detected. Please check wiring. Freezing.");
    while (1);
  }
  wdt_enable(WDTO_8S);
}


void loop() {

  humidity = aht20.getHumidity();
  temprature = aht20.getTemperature();

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
  display.print(round(temprature));
  display.display();
  wdt_reset();
}
