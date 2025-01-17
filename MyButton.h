/*Library written by VIJIL JOY*/
#ifndef MYBUTTON_H
#define MYBUTTON_h
class Button {
  bool b = 1;
  unsigned long lastDebounceTime = 0;
  unsigned long debounceDelay = 1;
  bool buttonState = 0;
  bool lastButtonState = 0;
  unsigned long timer;
  int t;
  char pin;

 public:
  void init(char p, char mode) {
    pin = p;
    pinMode(p, mode);
  }
  void ShortPress(void (*callback)()) {
    bool reading = digitalRead(pin);
    if (reading != lastButtonState) {
      t = 0;
      lastDebounceTime = millis();
    }
    if ((millis() - lastDebounceTime) > debounceDelay) {
      if (reading != buttonState) {
        buttonState = reading;
        if (buttonState == 1) {
          if (b == 0) {
            t = 0;
            callback();
          }
          b = 0;
        }
      }
    }
    lastButtonState = reading;
  }

  void longPress(void (*callback)(), int d) {
    if (digitalRead(pin) == 0) {
      if (millis() - timer >= 1000) {
        timer = millis();
        t++;
        if (t > d) {
          t = 0;
          b = 1;
          callback();
        }
      }
    }
  }
};
#endif