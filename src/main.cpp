#include <Arduino.h>

const int ledPin = 2;
const int relayPin = 22;
const int ldrPin = 32;
const int buttonPins[] = {21, 27, 26, 25};
int secretCode[] = {1, 2, 3, 4};
int enteredCode[4] = {0, 0, 0, 0};
int codeIndex = 0;
bool isDoorOpen = false;
unsigned long doorOpenTime = 0;
const unsigned long doorOpenDuration = 10000;
unsigned long lastBlinkTime = 0;
const unsigned long blinkInterval = 500;
bool ledState = true;
const int darkThreshold = 1000;
bool isDark = false;

void checkButtons();
void handleButtonPress(int buttonIndex);
void checkCode();
void openDoor();
void closeDoor();
void resetCode();

void setup() {
  Serial.begin(9600);
  Serial.println("Работает");
  pinMode(ledPin, OUTPUT);
  pinMode(relayPin, OUTPUT);
  pinMode(ldrPin, INPUT);
  digitalWrite(ledPin, HIGH);
  digitalWrite(relayPin, LOW);
  for (int i = 0; i < 4; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }
}

void loop() {
  int ldrValue = analogRead(ldrPin);
  isDark = (ldrValue > darkThreshold);
  static unsigned long lastLdrPrint = 0;
  if (millis() - lastLdrPrint > 2000) {
    Serial.print("LDR: ");
    Serial.print(ldrValue);
    Serial.print(", Темно: ");
    Serial.println(isDark);
    lastLdrPrint = millis();
  }
  if (isDoorOpen) {
    if (millis() - doorOpenTime >= doorOpenDuration) {
      closeDoor();
    } else {
      if (millis() - lastBlinkTime >= blinkInterval) {
        lastBlinkTime = millis();
        ledState = !ledState;
        digitalWrite(ledPin, ledState);
      }
    }
  } else {
    if (isDark) {
      digitalWrite(ledPin, HIGH);
      checkButtons();
    } else {
      digitalWrite(ledPin, LOW);
      resetCode();
    }
  }
  delay(50);
}
void checkButtons() {
  for (int i = 0; i < 4; i++) {
    int reading = digitalRead(buttonPins[i]);
    if (reading == LOW) {
      delay(50);
      if (digitalRead(buttonPins[i]) == LOW) {
        handleButtonPress(i);
        while (digitalRead(buttonPins[i]) == LOW) {
          delay(10);
        }
        delay(50);
      }
    }
  }
}

void handleButtonPress(int buttonIndex) {
  Serial.print("Нажата кнопка: ");
  Serial.println(buttonIndex + 1);
  enteredCode[codeIndex] = buttonIndex + 1;
  codeIndex++;
  Serial.print("Текущий ввод: ");
  for (int j = 0; j < codeIndex; j++) {
    Serial.print(enteredCode[j]);
  }
  Serial.println();
  if (codeIndex >= 4) {
    checkCode();
  }
}
void checkCode() {
  Serial.print("проверка кода: ");
  for (int i = 0; i < 4; i++) {
    Serial.print(enteredCode[i]);
  }
  Serial.println();
  bool correct = true;
  for (int i = 0; i < 4; i++) {
    if (enteredCode[i] != secretCode[i]) {
      correct = false;
      break;
    }
  }
  if (correct) {
    Serial.println("верный код");
    openDoor();
  } else {
    Serial.println("неверный код");
    resetCode();
  }
}

void openDoor() {
  isDoorOpen = true;
  doorOpenTime = millis();
  digitalWrite(relayPin, HIGH);
  Serial.println("Реле включено");
  resetCode();
}

void closeDoor() {
  isDoorOpen = false;
  digitalWrite(relayPin, LOW);
  digitalWrite(ledPin, HIGH);
  Serial.println("Реле вЫключено, дверь закрыта");
}

void resetCode() {
  if (codeIndex > 0) {
    codeIndex = 0;
    for (int i = 0; i < 4; i++) {
      enteredCode[i] = 0;
    }
    Serial.println("сброс ввода");
  }
}