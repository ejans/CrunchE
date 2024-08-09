#include <Arduino.h>

#include "LedManager.h"
LedManager ledManager = LedManager(0, 0, 0, 1);

#include "InputManager.h"
InputManager inputManager = InputManager();

//tracker
#include "Tracker.h"
Tracker tracker = Tracker();

//Matrix keypad library
#include <Keypad.h>
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  { 'P', 'L', 'H', 'D' },
  { 'O', 'K', 'G', 'C' },
  { 'N', 'J', 'F', 'B' },
  { 'M', 'I', 'E', 'A' }
};

byte rowPins[ROWS] = { 8, 7, 6, 5 };     //connect to the row pinouts of the keypad
byte colPins[COLS] = { 9, 10, 20, 21 };  //connect to the column pinouts of the keypad
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

float lastMillis;
char oldChar;
//i2s sound
#include <I2S.h>
const int sampleRate = 22000;  // sample rate in Hz

void setup() {
  //Serial.begin(115200);
  Serial.println("In setup");
  lastMillis = millis();

  //setup I2S pins
  I2S.setDataPin(0);
  I2S.setSckPin(1);//blc
  I2S.setFsPin(2);//lrc

  if (!I2S.begin(I2S_PHILIPS_MODE, sampleRate, 16)) {
    Serial.println("Failed to initialize I2S!");
    while (1)
       ;
  }
}

void loop() {
  /*char key = keypad.getKey();
  if (key) {
    Serial.println(key);
  }
  inputManager.UpdateInput(key);*/
  inputManager.UpdateInput(keypad.getKey());
  char note = inputManager.note;
  char trackCommand = inputManager.trackCommand;
  int trackCommandArgument = inputManager.trackCommandArgument;
  char ledCommand = inputManager.ledCommand;

  if (ledCommand != ' ') {
    ledManager.SetCommand(ledCommand);
  }

  if (trackCommand != ' ') {
    tracker.SetCommand(trackCommand, trackCommandArgument);
  }

  //int sample = tracker.UpdateTracker();
  int32_t sample = tracker.UpdateTracker();
  //loundness 
  if (abs(sample) > 4000) {
    int rem = abs(sample) - 4000;
    if (sample > 0) {
      sample = 4000 + (rem / 2);
    } else {
      sample = -4000 - (rem / 2);
    }
  }
  sample /= 4;

  I2S.write(sample);
  I2S.write(sample);

  int tempoBlink = tracker.tempoBlink;
  if (tempoBlink > 0)
    ledManager.SetLit(tempoBlink, tracker.selectedTrack);

  ledManager.SetPattern(tracker.allPatternPlay, tracker.currentPattern);
  ledManager.UpdateLed();
  inputManager.EndFrame();
}
