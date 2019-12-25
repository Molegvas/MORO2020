#include "measure/mkeyboard.h"
#include "button.h"
//#include <Arduino.h>

MKeyboard::MKeyboard()
{
    SButton * keyP  = new SButton( &KeyInput, adc_p,   0, 1000, 2000,  500);
    SButton * keyUp = new SButton( &KeyInput, adc_up, 50, 2000, 4000, 1000);
    SButton * keyDn = new SButton( &KeyInput, adc_dn, 50, 2000, 4000, 1000);
    SButton * keyC  = new SButton( &KeyInput, adc_c,   0, 1000, 2000,  500);
    SButton * keyB  = new SButton( &KeyInput, adc_b,  50, 2000, 4000, 1000);

    Buttons[0] = new MButton(keyP,  P_CLICK,  P_LONG_CLICK,  P_AUTO_CLICK,  "P");
    Buttons[1] = new MButton(keyUp, UP_CLICK, UP_LONG_CLICK, UP_AUTO_CLICK, "UP");
    Buttons[2] = new MButton(keyDn, DN_CLICK, DN_LONG_CLICK, DN_AUTO_CLICK, "DN");
    Buttons[3] = new MButton(keyC,  C_CLICK,  C_LONG_CLICK,  C_AUTO_CLICK,  "C");
    Buttons[4] = new MButton(keyB,  B_CLICK,  B_LONG_CLICK,  B_AUTO_CLICK,  "B");
}

MKeyboard::~MKeyboard()
{
   for (int i=0; i<numButtons; i++)
      delete Buttons[i];
}

bool MKeyboard::getKey( uint8_t key )
{
    if( keyBuff == key )
    {
        keyBuff = KEY_NO;
        return true;
    }
    else
        return false;
}

MKeyboard::KEY_PRESS MKeyboard::getKey()
{
    const KEY_PRESS key = static_cast<KEY_PRESS>(keyBuff);
    keyBuff = KEY_NO; 
    return key;
}

void MKeyboard::calcKeys(int adcKey)
{
  #ifdef DEBUG_KEYS
    Serial.print(F(" Conversion adcKey: "));    Serial.println( adcKey );
  #endif

  if      (abs(adcKey - adc_p ) < adc_rang)  KeyInput = adc_p;   // Кнопка Режим
  else if (abs(adcKey - adc_up) < adc_rang)  KeyInput = adc_up;  // Кнопка Вверх
  else if (abs(adcKey - adc_dn) < adc_rang)  KeyInput = adc_dn;  // Кнопка Вниз
  else if (abs(adcKey - adc_c ) < adc_rang)  KeyInput = adc_c;   // Кнопка Старт\Стоп
  else if (abs(adcKey - adc_b ) < adc_rang)  KeyInput = adc_b;   // Кнопка Выбор
  else                                       KeyInput = adc_no;

  #ifdef DEBUG_KEYS
    Serial.print(F(" KeyInput = "));   Serial.println( KeyInput );
  #endif

  for (int i=0; i<numButtons; i++)
      Buttons[i]->check(keyBuff);
}

// ---- MButton ----

MButton::MButton(SButton * Button, MKeyboard::KEY_PRESS Click, MKeyboard::KEY_PRESS LongClick, MKeyboard::KEY_PRESS AutoClick, const String & DebugName) :
    Button(Button),
    Click(Click),
    LongClick(LongClick),
    AutoClick(AutoClick),
    DebugName(DebugName)
{
    Button->begin();
}

MButton::~MButton()
{
    delete Button;
}

void MButton::check(uint8_t & keyBuff)
{
    switch ( Button->Loop() )
    {
    case SB_CLICK:
        keyBuff = Click;
			  #ifdef PRINT_KEY
      	  Serial.println("Press " + DebugName);
			  #endif
        break;
    case SB_LONG_CLICK:
        keyBuff = LongClick;
			  #ifdef PRINT_KEY
      	  Serial.println("Long press " + DebugName);
			  #endif
        break;
    case SB_AUTO_CLICK:
        keyBuff = AutoClick;
			  #ifdef PRINT_KEY
      	  Serial.println("Auto press " + DebugName);
			  #endif
        break;
    default:
        break;
    }
}