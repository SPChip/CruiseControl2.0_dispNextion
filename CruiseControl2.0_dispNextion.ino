//SPChip 2.5
/*Круиз контроль и дополнительный дисплей для NINEBOT ES2

  Переключение дисплеев - кнопка 1
  Вход в настройки - удерживать 2 сек кнопку 1
  В настрйках вверх и вниз - кнопки 2 и 3
  Выбор настройки - кнопка 1
  Включение круиза - кнопка 2 или 3
  Изменение скоростти - кнопка 2 или 3
*/
/*
  библиотека Adafruit_MCP4725.h https://github.com/adafruit/Adafruit_MCP4725/archive/master.zip
  библиотека DS3231.h https://github.com/jarzebski/Arduino-DS3231.git
  библиотека для дисплея https://yadi.sk/d/uwlwBOyijDYYR
  библиотека для кнопок https://github.com/AlexGyver/GyverLibs/releases/download/GyverButton/GyverButton.zip
  библиотека для таймеров https://github.com/AlexGyver/GyverLibs/releases/download/GyverTimer/GyverTimer.zip
  библиотека для ПИД регулятора https://github.com/AlexGyver/GyverLibs/releases/download/GyverPID/GyverPID.zip
*/

#define BTN_PIN1 14     // кнопка1 подключена сюда (BTN_PIN --- КНОПКА --- GND)
#define BTN_PIN2 15     // кнопка2 подключена сюда (BTN_PIN --- КНОПКА --- GND)
#define BTN_PIN3 16     // кнопка3 подключена сюда (BTN_PIN --- КНОПКА --- GND)
#define BRAKE_PIN A0    // ручка тормоза подключена сюда, потом диод и потом в BLE
#define BL_LCD_PIN 4           // подсветка дисплея подключена сюда
#define NINEBOT_PORT Serial1   // порт подключения к самокату
#define TIMEOUT_QUERY 25       // пауза между запросами, мс 
#define TIMEOUT_LCD 250        // пауза между обновлениями экрана, мс 
#define ACCEL_DAC_ADDR_I2C 0x61    // адрес ЦАП управления газом по шине I2C
#define BRAKE_DAC_ADDR_I2C 0x60    // адрес ЦАП управления тормозом по шине I2C
#define MIN_REGULATOR_OUT 0       // минимум на выходе регулятора ПИД
#define MAX_REGULATOR_OUT 0      // максимум на выходе регулятора ПИД
#define ITERATION_TIME 500          // время итерации ПИДа (как часто производится расчет, в мс)


#define INIT_ADDR 1023        // номер резервной ячейки для инициализации первого запуска
#define INIT_KEY 33           // ключ первого запуска. 0-254, на выбор, надо поменять на любое значение и будет как впервый раз
#define DISPLAY_MODE_ADDR 0   // адрес в eeprom для хранения режима экрана dysplayMode
#define BACKLIGHT_ADDR 10     // адрес в eeprom для хранения режима подсветки дисплея
#define PASSIVEMODE_ADDR 20   // адрес в eeprom для хранения состояния пассивного режима
#define KP_ADDR 30            // адрес в eeprom для хранения коэффициента Kp
#define KI_ADDR 40            // адрес в eeprom для хранения коэффициента Ki
#define KD_ADDR 50            // адрес в eeprom для хранения коэффициента Kd
#define STEP_ADDR 60          // адрес в eeprom для хранения шага изменения скорости

#include <Wire.h>             // Библиотека для работы с шиной I2C
#include <EEPROM.h>           // Библиотека для eeprom
#include "lcd1202.h"          // Библиотека для дисплея
#include "DS3231.h"           // Библиотека для часов реального времени
#include "Adafruit_MCP4725.h" // Библиотека для работы с ЦАП
#include "GyverButton.h"      // Библиотека для кнопок
#include "GyverTimer.h"       // Библиотека для таймеров
//#define PID_INTEGER           // Включаем целочисленный тип вычислениий ПИДа (объявляется перед библиотекой)
#include "GyverPID.h"         // Библиотека для PID регулятора

GButton BTN1(BTN_PIN1, HIGH_PULL, NORM_OPEN);     // настраиваем кнопку 1
GButton BTN2(BTN_PIN2, HIGH_PULL, NORM_OPEN);     // настраиваем кнопку 2
GButton BTN3(BTN_PIN3, HIGH_PULL, NORM_OPEN);     // настраиваем кнопку 3
LCD1202 LCD(8, 7, 6, 5);  // RST, CS, MOSI, SCK   // создаем и подключаем дисплей
DS3231 RTC;                                       // создаем часы реального времени
Adafruit_MCP4725 DAC_ACCEL;                       // создаем ЦАП управления газом
Adafruit_MCP4725 DAC_BRAKE;                       // создаем ЦАП управления тормозом
GTimer BLINK_TIMER(MS, 500);                      // создаем таймер для мигания в меню настроек
GTimer LCD_TIMER(MS, TIMEOUT_LCD);                // создаем таймер для отрисовки дисплея
GTimer TEST_TIMER(MS, 500);                      // создаем таймер для мигания в меню настроек
GyverPID REGULATOR_ACCEL(0, 0, 0, ITERATION_TIME);      // создаем регулятор PID газа
GyverPID REGULATOR_BRAKE(0, 0, 0, ITERATION_TIME);      // создаем регулятор PID тормоза

byte curs = 0;                       // текущий элемент массива данных, количество данных
byte data[64] = {0};                 // массив для данных, нулевой элемент = 0
int dysplayMode = 3;                 // в этом режиме запрашиваются нужные данные для начальных % и емкости
bool passiveMode;                    // флаг пассивного режима
bool BL_Lcd;                         // флаг подсветки дисплея вкл/выкл
bool cruiseControlFlag = 0;          // флаг включения круиз контроля
bool newSpeedFlag = 0;               // флаг новых данных о скорости


unsigned int startBatCharge = 0;      // % акб при включении
unsigned int startBatCapacityLeft = 0;// остаточная емкость обоих батарей при включении
unsigned int batCharge = 0;           // % акб
int currentSpeed;                     // текущая скорость
int presetSpeed;                      // установленная скорость для круиз контроля
int stepChangeSpeed;                  // шаг изменения скорости
int averageSpeed;                     // средняя скорость
int maxSpeed;                         // максимальная скорость
unsigned int remainingMileage;        // оставшийся пробег
unsigned long totalMileage;           // общий пробег
unsigned long currentMileage;         // текущий пробег
unsigned int ridingTime;              // время вождения
int escTemp;                          // температура контроллера
unsigned int inBatCapacityLeft = 0;   // остаточная емкость внутренней батареи
unsigned int exBatCapacityLeft = 0;   // остаточная емкость внешней батареи
unsigned int inBatCharge;             // заряд внутренней батареи
unsigned int exBatCharge;             // заряд внешней батареи
int inBatCurent;                      // ток внутренней батареи
int exBatCurent;                      // ток внешней батареи
unsigned int inBatVoltage;            // напряжение внутренней батареи
unsigned int exBatVoltage;            // напряжение  внешней батареи
int inBatTemp;                        // напряжениетемпература внутренней батареи
int exBatTemp;                        // температура  внешней батареи
unsigned int inBatCell[10];           // напряжение на банках внутренней батареи
unsigned int exBatCell[10];           // напряжение на банках внешней батареи
int phaseCurentMotor;                 // ток фазы мотора
int powerMotor;                       // мощность на двигателе


void setup() {
  Serial.begin(115200);                     // включаем и настраиваем последовательный порт к компу (в мониторе порта поставить тоже 115200)
  NINEBOT_PORT.begin(115200);               // включаем и настраиваем последовательный порт к скутеру
  pinMode(BL_LCD_PIN, OUTPUT);              // настраиваем пин на подсветку дисплея
  LCD.Inicialize();                         // инициализация дисплея
  LCD.Clear_LCD();                          // очистка дисплея
  DisplayLogo();                            // рисуем лого
  BTN1.setTimeout(2000);                    // настраиваем таймаут для долгого нажатия кнопки 1
  BTN2.setStepTimeout(200);                 // установка таймаута между инкрементами ( с какой скоростью будут меняться цифры при удержании кнопки)
  BTN3.setStepTimeout(200);                 // установка таймаута между инкрементами ( с какой скоростью будут меняться цифры при удержании кнопки)
  REGULATOR_ACCEL.setLimits(0, 4095);             // пределы, которые будем подавать на входы ЦАПов
  REGULATOR_BRAKE.setLimits(0, 4095);             // пределы, которые будем подавать на входы ЦАПов
  REGULATOR_ACCEL.setDirection(NORMAL);
  REGULATOR_BRAKE.setDirection(REVERSE);
  //REGULATOR.setMode(ON_RATE);
  RTC.begin();                              // включаем часы реального времени
  DAC_ACCEL.begin(ACCEL_DAC_ADDR_I2C);      // включаем ЦАП газа по адресу ACCEL_DAC_ADDR_I2C
  DAC_BRAKE.begin(BRAKE_DAC_ADDR_I2C);      // включаем ЦАП тормоза по адресу BRAKE_DAC_ADDR_I2C
  DAC_ACCEL.setVoltage(MIN_REGULATOR_OUT, false); // устанавливаем выход ЦАП газа в 0
  DAC_BRAKE.setVoltage(MIN_REGULATOR_OUT, false); // устанавливаем выход ЦАП тормоза в 0
  if (EEPROM.read(INIT_ADDR) != INIT_KEY) { // первый запуск
    EEPROM.write(INIT_ADDR, INIT_KEY);      // записали ключ
    EEPROM.put(DISPLAY_MODE_ADDR, 1);       // режим экрана 1
    EEPROM.put(BACKLIGHT_ADDR, 0);          // подсветка выключена
    EEPROM.put(PASSIVEMODE_ADDR, 0);        // пассивный режим выключен
    EEPROM.put(KP_ADDR, 1.0);               // начальное значение Kp
    EEPROM.put(KI_ADDR, 1.0);               // начальное значение Ki
    EEPROM.put(KD_ADDR, 1.0);               // начальное значение Kd
    EEPROM.put(STEP_ADDR, 10);              // начальное значение шага изменения скорости
  }
  EEPROM.get(BACKLIGHT_ADDR, BL_Lcd);            // читаем режим подсветки
  digitalWrite(BL_LCD_PIN, BL_Lcd);              // включаем/выключаем подсветку дисплея
  EEPROM.get(PASSIVEMODE_ADDR, passiveMode);     // читаем состояние пассивного режима

  while (millis () < 3000 ) {               // ждем 3 сек, чтобы наверняка получить все данные
    ReceivingData ();                       // получаем данные о начальном % и емкости батарей
    if (batCharge > 0 && inBatCapacityLeft > 0 && exBatCapacityLeft > 0 ) break; // если нужные данные получены выходим из пока досрочно
  }
  startBatCharge = batCharge;                                    // засекаем начальный заряд батареи
  startBatCapacityLeft = inBatCapacityLeft + exBatCapacityLeft;  // засекаем начальную емкость батарей
  if (startBatCharge == 0 || startBatCapacityLeft == 0) {        // если данные не получили
    DisplayNoData();                                             // выводим сообщение нет данных
    delay (1000);                                                // подождем 1 сек
  }
  EEPROM.get(KP_ADDR, REGULATOR_ACCEL.Kp);             // читаем и устанавливаем значение Kp
  EEPROM.get(KI_ADDR, REGULATOR_ACCEL.Ki);             // читаем и устанавливаем значение Ki
  EEPROM.get(KD_ADDR, REGULATOR_ACCEL.Kd);             // читаем и устанавливаем значение Kd
  REGULATOR_BRAKE.Kp =  REGULATOR_ACCEL.Kp;
  REGULATOR_BRAKE.Ki =  REGULATOR_ACCEL.Ki;
  REGULATOR_BRAKE.Kd =  REGULATOR_ACCEL.Kd;

  EEPROM.get(STEP_ADDR, stepChangeSpeed);        // читаем и устанавливаем значение шага изменения скорости
  EEPROM.get(DISPLAY_MODE_ADDR, dysplayMode);    // читаем режим экрана из eeprom
}



void loop() {
  BTN1.tick();                                   // постоянно проверяем первую кнопку
  BTN2.tick();                                   // постоянно проверяем вторую кнопку
  BTN3.tick();                                   // постоянно проверяем третью кнопку
  if (BTN1.isClick()) {                          // если кнопка 1 нажата переключаем режим отображения
    dysplayMode++;                               // переходим к следующему режиму экрана
    if (dysplayMode > 6) dysplayMode = 1;        // максимум 6 экранов
    EEPROM.put(DISPLAY_MODE_ADDR, dysplayMode);  // записываем в eeprom режим экрана
  }
  if (BTN1.isHolded()) {                         // если кнопка 1 зажата входим в меню установок
    DAC_ACCEL.setVoltage(MIN_REGULATOR_OUT * 10, false);   // устанавливаем выход ЦАП газа в 0
    DAC_BRAKE.setVoltage(MIN_REGULATOR_OUT * 10, false);   // устанавливаем выход ЦАП тормоза в 0
    cruiseControlFlag = 0;
    Settings();
  }
  if (BTN2.isClick() || BTN2.isStep()) {         // если кнопка 2 нажата
    if (!cruiseControlFlag) {                    // если круиз контроль выключен
      cruiseControlFlag  = 1;                    // включаем круиз контроль
      presetSpeed = currentSpeed;                // запоминаем текущую скорость
    }
    else {                                       // если круиз контроль включен

      presetSpeed = presetSpeed - stepChangeSpeed; // увеличиваем установленную скорость на заданный шаг
    }
  }
  if (BTN3.isClick() || BTN3.isStep()) {         // если кнопка 3 нажата
    if (!cruiseControlFlag) {                    // если круиз контроль выключен
      cruiseControlFlag  = 1;                    // включаем круиз контроль
      presetSpeed = currentSpeed;                // запоминаем текущую скорость
    }
    else {                                       // если круиз контроль включен

      presetSpeed = presetSpeed + stepChangeSpeed; // увеличиваем установленную скорость на заданный шаг
    }
  }
  if (analogRead(BRAKE_PIN) > 225) {             // постоянно проверяем ручку томоза и если она нажата на ...%
    cruiseControlFlag = 0;                       // выключаем круиз контроль
    DAC_ACCEL.setVoltage(MIN_REGULATOR_OUT * 10, false);   // устанавливаем выход ЦАП газа в 0
    DAC_BRAKE.setVoltage(MIN_REGULATOR_OUT * 10, false);   // устанавливаем выход ЦАП тормоза в 0
  }
  if (cruiseControlFlag) {                       // если флаг круиза поднят
    CruiseControl();                             // вызываем функцию круиза
  }
  ReceivingData();                               // постоянно обмениваемся данными
  Display();                                     // постоянно вызываем функцию отрисовки экрана
}
