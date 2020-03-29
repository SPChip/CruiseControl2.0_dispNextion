void CruiseControl() {                       // функция круиз контроля`
  static long timeCalc;                      // для вычисления времени между появлением новых данных о скорости
  REGULATOR_ACCEL.setpoint = presetSpeed;          // отправляем в регулятор установленную скорость
  REGULATOR_BRAKE.setpoint = presetSpeed;          // отправляем в регулятор установленную скорость
  if (newSpeedFlag) {
    if (millis() - timeCalc < 3000) {
      REGULATOR_ACCEL.setDt(millis() - timeCalc);    // устанавилваем dt ПИДа равным времени обновления данных о скорости
      REGULATOR_BRAKE.setDt(millis() - timeCalc);    // устанавилваем dt ПИДа равным времени обновления данных о скорости
    }
    else {
       REGULATOR_ACCEL.setDt(3000);    // устанавилваем dt ПИДа равным времени обновления данных о скорости
      REGULATOR_BRAKE.setDt(3000);    // устанавилваем dt ПИДа равным времени обновления данных о скорости
    }
    timeCalc = millis();                     // засекаем время
    newSpeedFlag = 0;                        // опускаем флаг новых данных о скорости
    REGULATOR_ACCEL.input = currentSpeed;          // отправляем в регулятор текущую скорость
    REGULATOR_BRAKE.input = currentSpeed;          // отправляем в регулятор текущую скорость
    DAC_ACCEL.setVoltage(map (REGULATOR_ACCEL.getResult(), 0 , 255, 800, 2500), false);          // регулируем скорость ЦАПом  газа
    DAC_BRAKE.setVoltage(map (REGULATOR_BRAKE.getResult(), 0 , 255, 800, 2500), false);          // регулируем скорость ЦАПом газа

    //}
    //if (currentSpeed > presetSpeed){          // если текущая скорость больше установленной
    //REGULATOR.setDirection(REVERSE);          // включаем обоатное регулирование ПИДа
    // DAC_ACCEL.setVoltage(MIN_REGULATOR_OUT * 10, false);          // устанавливаем выход ЦАП газа в 0
    //DAC_BRAKE.setVoltage(map (REGULATOR.getResult(),0 , 500, 800, 3700), false);           // регулируем скорость ЦАПом тормоза
    //}

    Serial.print(presetSpeed);
    Serial.print(",");
    Serial.print(currentSpeed);
    Serial.println(",");
  }
  //if (TEST_TIMER.isReady()) {


  //}
}
