/*
   button.h
   Test:
   Ol.Moro 2018.08.28
*/

#ifndef _BUTTON_H_
#define _BUTTON_H_

//#include "Arduino.h"
//#include <inttypes.h>
#include "stdint.h"

//#define DEBUG_BUTTON 1

typedef enum {
  SB_NONE = 0,
  SB_CLICK,
  SB_AUTO_CLICK,
  SB_LONG_CLICK,
} SBUTTON_CLICK;

class SButton {
  private :
    uint16_t Mark;
    bool     State;
    bool     Long_press_state;
    bool     Auto_press_state;
    uint32_t Ms1;
    uint32_t Ms2;
    uint32_t Ms_auto_click;
    uint16_t TM_bounce;
    uint16_t TM_long_click;
    uint16_t TM_auto_click;
    uint16_t Period_auto_click;
    uint16_t *myInput;
  public :
    SButton( uint16_t* myInput, uint16_t mark, uint16_t tm1 = 50, uint16_t tm2 = 0, uint16_t tm3 = 0, uint16_t tm4 = 500 );

    void begin();
    SBUTTON_CLICK Loop();
};

#endif