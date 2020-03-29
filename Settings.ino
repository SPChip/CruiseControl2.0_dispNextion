void Settings() {                               // меню настроек
  byte setting = 1;                             // номер текущей настройки
  byte levelMenu = 1;                           // подпараметры в меню
  bool inputSettings = 0;                       // флаг выбора установки (вход в установку выполнен или нет)
  bool flagBlink = 1;                           // флаг для мигания изменяемого параметра
  
  
  LCD.Clear_LCD();                              // очищаем дисплей
  RTCDateTime dt;                               // объявляем стурктуру данных для времени и даты
  while (!BTN1.isHolded()) {                    // пока не будет удержана кнопка 1
    BTN1.tick();                                // постоянно проверяем все три кнопки
    BTN2.tick();
    BTN3.tick();
    dt = RTC.getDateTime();                      // записываем время и дату в dt
    if (inputSettings == 0 && BTN2.isClick()) {  // если не выбрана установка и нажата кнопка 2
      LCD.Clear_LCD();
      setting--;                                 // переходим к предъидущей настройке
      if (setting < 1) {                         // максимум 5 настроек
        setting = 5;
      }
    }
    if (inputSettings == 0 && BTN3.isClick()) {  // если не выбрана установка и нажата кнопка 2
      LCD.Clear_LCD();
      setting++;                                 // переходим к следующей настройке
      if (setting > 5) {
        setting = 1;
      }
    }
    if (inputSettings == 0 && BTN1.isClick()) {  // если не выбрана установка и нажата кнопка 1
      LCD.Clear_LCD();
      inputSettings = 1;                          // входим в установку
    }
    if (inputSettings) {                           // если вошли в установку
      switch (setting) {                           // установка
        case 1:                                    // подсветка
          if (BTN1.isClick()) {                    // если нажата кнопка 1 выходим из установки
            LCD.Clear_LCD();
            inputSettings = 0;
          }
          if (((BTN2.isClick()) || (BTN3.isClick()))) {
            LCD.Clear_LCD();
            BL_Lcd = !BL_Lcd;
            digitalWrite(BL_LCD_PIN, BL_Lcd);
            EEPROM.put(BACKLIGHT_ADDR, BL_Lcd);  // записываем в eeprom режим подсветки
          }
          break;
        case 2:
          if (BTN1.isClick()) {
            LCD.Clear_LCD();
            levelMenu++;
            if (levelMenu > 3) {
              inputSettings = 0;
              levelMenu = 1;
            }
          }
          switch (levelMenu) {
            case 1:                     //установка часов
              if (BTN2.isClick() || BTN2.isStep()) {
                LCD.Clear_LCD();
                char hour = dt.hour - 1;
                if (hour < 0) hour = 23;
                RTC.setDateTime(dt.year, dt.month, dt.day, hour, dt.minute, dt.second);
              }
              if (BTN3.isClick() || BTN3.isStep()) {
                LCD.Clear_LCD();
                char hour = dt.hour + 1;
                if (hour >= 24) hour = 0;
                RTC.setDateTime(dt.year, dt.month, dt.day, hour, dt.minute, dt.second);
              }
              break;
            case 2:                     //установка минут
              if (BTN2.isClick() || BTN2.isStep()) {
                LCD.Clear_LCD();
                char minutes = dt.minute - 1;
                if (minutes < 0) minutes = 59;
                RTC.setDateTime(dt.year, dt.month, dt.day, dt.hour, minutes, dt.second);
              }
              if (BTN3.isClick() || BTN3.isStep()) {
                LCD.Clear_LCD();
                char minutes = dt.minute + 1;
                if (minutes >= 60) minutes = 0;
                RTC.setDateTime(dt.year, dt.month, dt.day, dt.hour, minutes, dt.second);
              }
              break;
            case 3:                     //установка секунд
              if (BTN2.isClick() || BTN2.isStep()) {
                LCD.Clear_LCD();
                char sec = dt.second - 1;
                if (sec < 0) sec = 59;
                RTC.setDateTime(dt.year, dt.month, dt.day, dt.hour, dt.minute, sec);
              }
              if (BTN3.isClick() || BTN3.isStep()) {
                LCD.Clear_LCD();
                char sec = dt.second + 1;
                if (sec >= 60) sec = 0;
                RTC.setDateTime(dt.year, dt.month, dt.day, dt.hour, dt.minute, sec);
              }
              break;
          }
          break;
        case 3:                                             //пассивный режим
          if (BTN1.isClick()) {
            LCD.Clear_LCD();
            inputSettings = 0;
          }
          if (((BTN2.isClick()) || (BTN3.isClick()))) {
            LCD.Clear_LCD();
            passiveMode = !passiveMode;
            EEPROM.put(PASSIVEMODE_ADDR, passiveMode);  // записываем в eeprom состояние пассивного режима
          }
          break;
        case 4:                                         // настройка PID
          if (BTN1.isClick()) {
            LCD.Clear_LCD();
            levelMenu++;
            if (levelMenu > 3) {
              inputSettings = 0;
              levelMenu = 1;
            }
          }
          switch (levelMenu) {
            case 1:                     //Kp
              if (BTN2.isClick() || BTN2.isStep()) {
                LCD.Clear_LCD();
                float _Kp = REGULATOR_ACCEL.Kp - 0.1;
                if (_Kp < 0) _Kp = 0.0;
                REGULATOR_ACCEL.Kp = _Kp;
                EEPROM.put(KP_ADDR, REGULATOR_ACCEL.Kp);  // записываем в eeprom значение Kp
              }
              if (BTN3.isClick() || BTN3.isStep()) {
                LCD.Clear_LCD();
                float _Kp = REGULATOR_ACCEL.Kp + 0.1;
                if (_Kp >  99.0) _Kp =  99.0;
                REGULATOR_ACCEL.Kp = _Kp;
                EEPROM.put(KP_ADDR, REGULATOR_ACCEL.Kp);  // записываем в eeprom значение Kp
              }
              break;
            case 2:                     //Ki
              if (BTN2.isClick() || BTN2.isStep()) {
                LCD.Clear_LCD();
                float _Ki = REGULATOR_ACCEL.Ki - 0.1;
                if (_Ki < 0) _Ki = 0;
                REGULATOR_ACCEL.Ki = _Ki;
                EEPROM.put(KI_ADDR, REGULATOR_ACCEL.Ki);  // записываем в eeprom значение Ki
              }
              if (BTN3.isClick() || BTN3.isStep()) {
                LCD.Clear_LCD();
                float _Ki = REGULATOR_ACCEL.Ki + 0.1;
                if (_Ki > 9.9) _Ki = 9.9;
                REGULATOR_ACCEL.Ki = _Ki;
                EEPROM.put(KI_ADDR, REGULATOR_ACCEL.Ki);  // записываем в eeprom значение Ki
              }
              break;
            case 3:                    //Kd
              if (BTN2.isClick() || BTN2.isStep()) {
                LCD.Clear_LCD();
                float _Kd = REGULATOR_ACCEL.Kd - 0.1;
                if (_Kd < 0) _Kd = 0;
                REGULATOR_ACCEL.Kd = _Kd;
                EEPROM.put(KD_ADDR, REGULATOR_ACCEL.Kd);  // записываем в eeprom значение Kd
              }
              if (BTN3.isClick() || BTN3.isStep()) {
                LCD.Clear_LCD();
                float _Kd = REGULATOR_ACCEL.Kd + 0.1;
                if (_Kd > 9.9 ) _Kd = 9.9;
                REGULATOR_ACCEL.Kd = _Kd;
                EEPROM.put(KD_ADDR, REGULATOR_ACCEL.Kd);  // записываем в eeprom значение Kd
              }
              break;
          }
        case 5:                       // настройка шага изменения скорости
          if (BTN1.isClick()) {
            LCD.Clear_LCD();
            inputSettings = 0;
          }
          if (BTN2.isClick() || BTN2.isStep()) {
            LCD.Clear_LCD();
            stepChangeSpeed = stepChangeSpeed - 5;
            if (stepChangeSpeed < 5) stepChangeSpeed = 5;
            EEPROM.put(STEP_ADDR, stepChangeSpeed);  // записываем в eeprom значение шага изменения скорости
          }
          if (BTN3.isClick() || BTN3.isStep()) {
            LCD.Clear_LCD();
            stepChangeSpeed = stepChangeSpeed + 5;
            if (stepChangeSpeed > 95) stepChangeSpeed = 95;
            EEPROM.put(STEP_ADDR, stepChangeSpeed);  // записываем в eeprom значение шага изменения скорости
          }
          break;
      }
    }


    // ОТРИСОВКА
    // заголовок
    LCD.print(25, 0, 1, "Settings");
    LCD.drawFastHLine(0, 8, 96, 1);
    // курсор
    LCD.print(0, setting * 10 + 1, 1, ">");

    //подсветка
    LCD.print(6, 11, 1, "Backlight");
    if (inputSettings && setting == 1) {
      if (BLINK_TIMER.isReady()) {
        if (BL_Lcd) {
          if (flagBlink) {
            LCD.print(85, 11, 1,  "  ");
          }
          else {
            LCD.print(85, 11, 1,  "ON");
          }
        }
        else {
          if (flagBlink) {
            LCD.print(79, 11, 1,  "   ");
          }
          else {
            LCD.print(79, 11, 1,  "OFF");
          }
        }
        flagBlink = !flagBlink;
      }
    }
    else {
      if (BL_Lcd) {
        LCD.print(85, 11, 1, "ON");
      }
      else {
        LCD.print(79, 11, 1, "OFF");
      }
    }


    //установка часов
    LCD.print(6, 21, 1, "Time");
    //часы
    if (inputSettings && setting == 2 && levelMenu == 1 && !BTN2.state() && !BTN3.state()) {
      if (BLINK_TIMER.isReady()) {
        if (flagBlink) {
          LCD.print(48, 21, 1,  "  ");
        }
        else {

          if (dt.hour < 10) {
            LCD.print(54, 21, 1,  dt.hour);
          }
          else {
            LCD.print(48, 21, 1,  dt.hour);
          }
        }
        flagBlink = !flagBlink;
      }
    }
    else {
      if (dt.hour < 10) {
        LCD.print(54, 21, 1,  dt.hour);
      }
      else {
        LCD.print(48, 21, 1,  dt.hour);
      }
    }
    LCD.print(60, 21, 1, ":");
    //минуты
    if (inputSettings && setting == 2 && levelMenu == 2 && !BTN2.state() && !BTN3.state()) {
      if (BLINK_TIMER.isReady()) {
        if (flagBlink) {
          LCD.print(66, 21, 1,  "  ");
        }
        else {

          if (dt.minute < 10) {
            LCD.print(66, 21, 1,  "0");
            LCD.print(72, 21, 1,  dt.minute);
          }
          else {
            LCD.print(66, 21, 1,  dt.minute);
          }
        }
        flagBlink = !flagBlink;
      }
    }
    else {
      if (dt.minute < 10) {
        LCD.print(66, 21, 1,  "0");
        LCD.print(72, 21, 1,  dt.minute);
      }
      else {
        LCD.print(66, 21, 1,  dt.minute);
      }
    }
    LCD.print(79, 21, 1, ":");
    //секунды
    if (inputSettings && setting == 2 && levelMenu == 3 && !BTN2.state() && !BTN3.state()) {
      if (BLINK_TIMER.isReady()) {
        if (flagBlink) {
          LCD.print(85, 21, 1,  "  ");
        }
        else {

          if (dt.second < 10) {
            LCD.print(85, 21, 1,  "0");
            LCD.print(91, 21, 1,  dt.second);
          }
          else {
            LCD.print(85, 21, 1,  dt.second);
          }
        }
        flagBlink = !flagBlink;
      }
    }
    else {
      if (dt.second < 10) {
        LCD.print(85, 21, 1,  "0");
        LCD.print(91, 21, 1,  dt.second);
      }
      else {
        LCD.print(85, 21, 1, dt.second);
      }
    }
    // пассивный режим
    LCD.print(6, 31, 1, "PassiveMode");
    if (inputSettings && setting == 3) {
      if (BLINK_TIMER.isReady()) {
        if (passiveMode) {
          if (flagBlink) {
            LCD.print(85, 31, 1,  "  ");
          }
          else {
            LCD.print(85, 31, 1,  "ON");
          }
        }
        else {
          if (flagBlink) {
            LCD.print(79, 31, 1,  "   ");
          }
          else {
            LCD.print(79, 31, 1,  "OFF");
          }
        }
        flagBlink = !flagBlink;
      }
    }
    else {
      if (passiveMode) {
        LCD.print(85, 31, 1, "ON");
      }
      else {
        LCD.print(79, 31, 1, "OFF");
      }
    }

    // PID
    LCD.print(6, 41, 1, "PID");
    // Kp
    if (inputSettings && setting == 4 && levelMenu == 1 && !BTN2.state() && !BTN3.state()) {
      if (BLINK_TIMER.isReady()) {
        if (flagBlink) {
          LCD.print(25, 41, 1,  "    ");
        }
        else {
          if (REGULATOR_ACCEL.Kp >= 10) {
            LCD.print(25, 41, 1, (int) REGULATOR_ACCEL.Kp);
            LCD.print(37, 41, 1, ".");
            LCD.print(42, 41, 1, (int) (REGULATOR_ACCEL.Kp * 10) % 10);
          }
          else {
            LCD.print(31, 41, 1, (int) REGULATOR_ACCEL.Kp);
            LCD.print(37, 41, 1, ".");
            LCD.print(42, 41, 1, (int) (REGULATOR_ACCEL.Kp * 10) % 10);
          }
        }
        flagBlink = !flagBlink;
      }
    }
    else {
      if (REGULATOR_ACCEL.Kp >= 10) {
            LCD.print(25, 41, 1, (int) REGULATOR_ACCEL.Kp);
            LCD.print(37, 41, 1, ".");
            LCD.print(42, 41, 1, (int) (REGULATOR_ACCEL.Kp * 10) % 10);
          }
          else {
            LCD.print(31, 41, 1, (int) REGULATOR_ACCEL.Kp);
            LCD.print(37, 41, 1, ".");
            LCD.print(42, 41, 1, (int) (REGULATOR_ACCEL.Kp * 10) % 10);
          }
    }
    // Ki
    if (inputSettings && setting == 4 && levelMenu == 2 && !BTN2.state() && !BTN3.state()) {
      if (BLINK_TIMER.isReady()) {
        if (flagBlink) {
          LCD.print(54, 41, 1,  "   ");
        }
        else {
          LCD.print(54, 41, 1, (int) REGULATOR_ACCEL.Ki);
          LCD.print(60, 41, 1, ".");
          LCD.print(65, 41, 1, (int) (REGULATOR_ACCEL.Ki * 10) % 10);
        }
        flagBlink = !flagBlink;
      }
    }
    else {
      LCD.print(54, 41, 1, (int) REGULATOR_ACCEL.Ki);
      LCD.print(60, 41, 1, ".");
      LCD.print(65, 41, 1, (int) (REGULATOR_ACCEL.Ki * 10) % 10);
    }
    // Kd
    if (inputSettings && setting == 4 && levelMenu == 3 && !BTN2.state() && !BTN3.state()) {
      if (BLINK_TIMER.isReady()) {
        if (flagBlink) {
          LCD.print(80, 41, 1,  "   ");
        }
        else {
          LCD.print(80, 41, 1, (int) REGULATOR_ACCEL.Kd);
          LCD.print(86, 41, 1, ".");
          LCD.print(91, 41, 1, (int) (REGULATOR_ACCEL.Kd * 10) % 10);
        }
        flagBlink = !flagBlink;
      }
    }
    else {
      LCD.print(80, 41, 1, (int) REGULATOR_ACCEL.Kd);
      LCD.print(86, 41, 1, ".");
      LCD.print(91, 41, 1, (int) (REGULATOR_ACCEL.Kd * 10) % 10);
    }
    // шаг изменения скорости
    LCD.print(6, 51, 1, "SpeedStep");
    if (inputSettings && setting == 5 && !BTN2.state() && !BTN3.state()) {
      if (BLINK_TIMER.isReady()) {
        if (flagBlink) {
          LCD.print(80, 51, 1,  "   ");
        }
        else {
          LCD.print(80, 51, 1,  stepChangeSpeed / 10);
          LCD.print(86, 51, 1, ".");
          LCD.print(91, 51, 1,  stepChangeSpeed % 10);
        }
        flagBlink = !flagBlink;
      }
    }
    else {
      LCD.print(80, 51, 1,  stepChangeSpeed / 10);
      LCD.print(86, 51, 1, ".");
      LCD.print(91, 51, 1,  stepChangeSpeed % 10);
    }
    LCD.Update();
  }
}
