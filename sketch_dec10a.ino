byte sensorPin = 0;
byte lightSensorPin = 2;		  
int sensorValue = 0;  
byte minHumidity = 0;
byte pumpOut = 10;
byte lampOut = 9;
byte buttonPin = 2;
unsigned long delayPump = 5000;
unsigned long delayPausePump = 60000;
unsigned long delayPauseLamp = 60000;
unsigned long lampPauseTime = 0;
unsigned long startTime = 0;
unsigned long pauseTime = 0;
bool pumpIsOn = false;
bool lampIsOn = false;
int currentButtonRegime = 0;
bool buttonPressed = false;
int loopDelay = 100;

#include <avr/wdt.h>
#include <LiquidCrystal_I2C.h>
#include <AHT20.h>
#include<math.h>
AHT20 aht20;
LiquidCrystal_I2C lcd(0x27,20,4);


void setup() {
  Serial.begin(9600);
  pinMode(pumpOut, OUTPUT);
  pinMode(lampOut, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  lcd.init();                     
  lcd.backlight();
  lcd.clear();
  if (aht20.begin() == false)
  {
    Serial.println("AHT20 not detected. Please check wiring. Freezing.");
    while (1);
  }
  setScreenStatic();
  wdt_enable(WDTO_8S);
}


void loop() {
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
  sensorValue = analogRead(sensorPin);
  minHumidity = sensorValue/10;
  if (minHumidity >= 100) {
    minHumidity = 99;
  }
   if (minHumidity <= 30) {
    minHumidity = 30;
  }
  if ((aht20.getHumidity() < minHumidity) && !pumpIsOn && (pauseTime + delayPausePump) < millis()) {
    pumpIsOn = true;
    startTime = millis();
    pauseTime = 0;
  }
  lcd.setCursor(15, 0);
  lcd.print(round(aht20.getHumidity()));
  lcd.setCursor(15, 1);
  lcd.print(minHumidity);

  lcd.setCursor(15, 2);
  lcd.print(round(aht20.getTemperature()));
  setScreenStatic();
  lcd.setCursor(0, 3);
  lcd.print(millis());
  delay(loopDelay);
  wdt_reset();
  //checkRegime();
}

void setScreenStatic() {
  lcd.setCursor(0, 0);
  lcd.print("Humidity:");
  lcd.setCursor(17, 0);
  lcd.print("%");
  lcd.setCursor(0, 1);
  lcd.print("Min. humidity:");
  lcd.setCursor(17, 1);
  lcd.print("%");
  lcd.setCursor(0, 2);
  lcd.print("Temprature:");
  lcd.setCursor(17, 2);
  lcd.print("C");
}

void checkRegime() {
  
  if (!buttonPressed && !digitalRead(buttonPin) == true) {
    buttonPressed = true;
  }
  if ((!digitalRead(buttonPin) == false) && buttonPressed) {
    buttonPressed = false;
    if (currentButtonRegime == 1) {
      currentButtonRegime = 0;
    } else {
      currentButtonRegime++;
    }
  }
  
}
