/*
  button.cpp
  Test:
  Ol.Moro 2018.08.28
  http://samopal.pro/arduino-button-2/
*/

#include "Arduino.h"
#include "button.h"

//   Конструктор класса кнопки
//   Кнопки подключены к делителю напряжения, подтянутого к питанию, и замыкаются на землю
//   Событие срабатывания происходит по нажатию кнопки (возвращается код adc)
//   и отпусканию кнопки (возвращается время нажатия кнопки, мсек)
//   tm1 - таймаут дребезга контактов. По умолчанию 50мс  - требует уточнения - moro: проверено с =0, ОК
//   tm2 - время длинного нажатия клавиши. По умолчанию 2000мс
//   tm3 - время перевода кнопки в генерацию серии нажатий. По умолчанию отключено
//   tm4 - время между кликами в серии. По умолчанию 500 мс. Если tm3 = 0 то не работает
SButton::SButton( uint16_t* input, uint16_t mark, uint16_t tm1, uint16_t tm2, uint16_t tm3, uint16_t tm4 ) {
  myInput           = input;
  Mark              = mark;
  State             = false;
  Long_press_state  = false;
  Auto_press_state  = false;
  Ms1               = 0;
  Ms2               = 0;
  Ms_auto_click     = 0;
  TM_bounce         = tm1;
  TM_long_click     = tm2;
  TM_auto_click     = tm3;
  Period_auto_click = tm4;
}

/**
   Инициализация кнопки
*/
void SButton::begin() {

#ifdef DEBUG_BUTTON
  Serial.print("Init button mark: ");
  Serial.println(Mark);
//  Serial.println(*myInput);
#endif
}

//   Действие производимое в цикле или по таймеру
//   возвращает SB_NONE если кнопка не нажата и событие "нажатие" или "длинное нажатие" кнопки
SBUTTON_CLICK SButton::Loop() {

  uint32_t ms = millis();
  bool key_state = ( *myInput == Mark );

  // Фиксируем нажатие кнопки
  if ( key_state == true && State == false && (ms - Ms1) > TM_bounce ) {

    uint16_t dt = ms - Ms1;
    Long_press_state = false;
    Auto_press_state = false;
#ifdef DEBUG_BUTTON
    Serial.print(">>>Event button, pin=");
    Serial.print(Mark);
    Serial.print(",press ON, tm=");
    Serial.print(dt);
    Serial.println(" ms");
#endif
    State = true;
    Ms2   = ms;
    if ( TM_long_click == 0 && TM_auto_click == 0 )return SB_CLICK;
  }

  // Фиксируем длинное нажатие кнопки
  if ( key_state == true && !Long_press_state && TM_long_click > 0 && ( ms - Ms2 ) > TM_long_click ) {

    uint16_t dt      = ms - Ms2;
    Long_press_state = true;
#ifdef DEBUG_BUTTON
    Serial.print(">>>Event button, pin=");
    Serial.print(Mark);
    Serial.print(",long press, tm=");
    Serial.print(dt);
    Serial.println(" ms");
#endif
    return SB_LONG_CLICK;
  }

  // Фиксируем авто нажатие кнопки
  if ( key_state == true && TM_auto_click > 0
       && ( ms - Ms2 ) > TM_auto_click
       && ( ms - Ms_auto_click ) > Period_auto_click ) {
    uint16_t dt      = ms - Ms2;
    Auto_press_state = true;
    Ms_auto_click    = ms;
#ifdef DEBUG_BUTTON
    Serial.print(">>>Event button, mark=");
    Serial.print(Mark);
    Serial.print(",auto press, tm=");
    Serial.print(dt);
    Serial.println(" ms");
#endif
    return SB_AUTO_CLICK;
  }

  // Фиксируем отпускание кнопки
  if ( key_state == false && State == true && (ms - Ms2) > TM_bounce ) {

    uint16_t dt      = ms - Ms2;
    // Сбрасываем все флаги
    State            = false;
#ifdef DEBUG_BUTTON
    Serial.print(">>>Event button, mark=");
    Serial.print(Mark);
    Serial.print(",press OFF, tm=");
    Serial.print(dt);
    Serial.println(" ms");
#endif
    Ms1    = ms;
    // Возвращаем короткий клик
    if ( (TM_long_click != 0 || TM_auto_click != 0) && !Long_press_state && !Auto_press_state )return SB_CLICK;

  }

  return SB_NONE;
}
