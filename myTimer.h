#ifndef MyTimer_h
#define MyTimer_h

class MyTimer {
  private:
    long hours, seconds, minutes, milliseconds;
    short int h, m, s, ms;
    unsigned long mls, ticks;
    bool flag = false, flag2 = false, flag3 = false;
    void (*callback)();
    void (*ont)(unsigned long ticks);

  public:
    void setCallback(void (*y)()) {
      callback = y;
    }
    
    void setOnTickCallback(void (*y)(unsigned long ticks)) {
      ont = y;
      flag3 = true;
    }

    void setTimeout(String str) {
      short int firstColon = str.indexOf(':');
      short int secondColon = str.indexOf(':', firstColon + 1);
      short int thirdColon = str.indexOf(':', secondColon + 1);

      h = str.substring(0, firstColon).toInt();
      m = str.substring(firstColon + 1, secondColon).toInt();
      s = str.substring(secondColon + 1, thirdColon).toInt();
      ms = str.substring(thirdColon + 1).toInt();

      flag = true;
      flag2 = true;
    }

    void setTimeout(long time_ms) {
      milliseconds = time_ms;
      flag = true;
      flag2 = false;
    }

    void run() {
      if (millis() - mls >= (flag2 ? 1000 : milliseconds) && flag == true) {
        ticks++;
        mls = millis();
        if (flag2) {
          if (flag3) ont(ticks);
          seconds++;
          if (seconds > 59) {
            seconds = 0;
            minutes++;
            if (minutes > 59) {
              minutes = 0;
              hours++;
            }
          }
        } else {
          flag = false;
          callback();
        }
      }

      if (seconds == s && minutes == m && hours == h && flag == true && flag2 == true) {
        flag = false;
        callback();
      }
    }

    void reset() {
      flag = true;
      seconds = minutes = hours = ticks = 0;
    }
};

String Time(int sec, int mnt = 0, int hrs = 0) {
  return (String(hrs) + ":" + String(mnt) + ":" + String(sec));
}

#endif
