#include <Arduino.h>

const int ledPin = 23;
const int relayPin = 22;
const int ldrPin = 32;
const int buttonPins[] = {21, 12, 14, 27};
int secretCode[] = {1, 2, 3, 4};
int enteredCode[4] = {0, 0, 0, 0};
int codeIndex = 0;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;
int lastButtonState[] = {HIGH, HIGH, HIGH, HIGH};
int buttonState[] = {HIGH, HIGH, HIGH, HIGH};

bool isDoorOpen = false;
unsigned long doorOpenTime = 0;
const unsigned long doorOpenDuration = 10000;
bool isBlinking = false;
unsigned long lastBlinkTime = 0;
const unsigned long blinkInterval = 500;
bool codeWasReset = false;
void checkButtons();
void resetCode();
void checkCode();

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  pinMode(relayPin, OUTPUT);
  digitalWrite(ledPin, HIGH);
  digitalWrite(relayPin, LOW);
  for (int i = 0; i < 4; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
    lastButtonState[i] = digitalRead(buttonPins[i]);
  }
}
void loop() {
  int ldrValue = analogRead(ldrPin);
  if (isDoorOpen) {
    if (millis() - doorOpenTime >= doorOpenDuration) {
      digitalWrite(relayPin, LOW);
      isDoorOpen = false;
      isBlinking = false;
      digitalWrite(ledPin, HIGH);
      Serial.println("дверь закрыта");
    } else {
      if (millis() - lastBlinkTime >= blinkInterval) {
        lastBlinkTime = millis();
        digitalWrite(ledPin, !digitalRead(ledPin));
      }
    }
  }
  if (!isDoorOpen) {
    if (ldrValue > 2000) {
      if (!isBlinking) {
        digitalWrite(ledPin, HIGH);
      }
      checkButtons();
      codeWasReset = false;
    } else {
      digitalWrite(ledPin, LOW);
      if (!codeWasReset) {
        resetCode();
        codeWasReset = true;
      }
    }
  }
  delay(100);
}
void checkButtons() {
  for (int i = 0; i < 4; i++) {
    int reading = digitalRead(buttonPins[i]);
    if (reading != lastButtonState[i]) {
      lastDebounceTime = millis();
    }
    if ((millis() - lastDebounceTime) > debounceDelay) {
      if (reading != buttonState[i] && reading == LOW) {
        buttonState[i] = reading;
        Serial.print("нажата кнопка ");
        Serial.println(i + 1);
        enteredCode[codeIndex] = i + 1;
        codeIndex++;
        while (digitalRead(buttonPins[i]) == LOW) {
          delay(10);
        }
        Serial.print("~~текущий ввод: ");
        for (int j = 0; j < codeIndex; j++) {
          Serial.print(enteredCode[j]);
        }
        Serial.println();
        if (codeIndex >= 4) {
          checkCode();
        }
      }
      buttonState[i] = reading;
    }
    lastButtonState[i] = reading;
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
    Serial.println("дверь открыта на 10 секунд");
    digitalWrite(relayPin, HIGH);
    isDoorOpen = true;
    doorOpenTime = millis();
    isBlinking = true;
    lastBlinkTime = millis();
  } else {
    Serial.println("неверный код");
    resetCode();
  }
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