#ifndef _OPTIONSFSM_H_
#define _OPTIONSFSM_H_

#include "mtools.h"
#include "board/mboard.h"
#include "display/moled.h"
#include "mstate.h"

namespace OptionFsm
{
    class MStart : public MState
    {       
      public:
        MStart(MTools * Tools) : MState(Tools) {
          #ifdef V22
            Board->ledsOn();
          #endif
        }
        virtual MState * fsm() override;
    };

    class MSelectBattery : public MState
    {
      public:     
        MSelectBattery(MTools * Tools) : MState(Tools) {}   
        virtual MState * fsm() override;
    };

    class MSetPostpone : public MState
    {       
      public:
        MSetPostpone(MTools * Tools) : MState(Tools) {
          Tools->postpone = Tools->readNvsInt("qulon", "postp",  0 );
          Oled->showLine4Text(" Отложить на ");
          Oled->showLine3Delay( Tools->postpone );
          Tools->showUpDn();                      // " UP/DN, В-выбор "
        }
        virtual MState * fsm() override;
    };

    class MSetCurrentOffset : public MState
    {
      public:
        MSetCurrentOffset(MTools * Tools) : MState(Tools) {
          Oled->showLine4Text("    корр I   ");       //Oled->showLine4RealVoltage();
          Oled->showLine3RealCurrent();
          Tools->showUpDn(); // " UP/DN, В-выбор "
        }
        virtual MState * fsm() override;
    };

  class MSetVoltageOffset : public MState
    {
      public:
        MSetVoltageOffset(MTools * Tools) : MState(Tools) {
          Oled->showLine4RealVoltage();
          Oled->showLine3Text("    корр V   ");   //  Oled->showLine3RealCurrent();
          Tools->showUpDn(); // " UP/DN, В-выбор "
        }
        virtual MState * fsm() override;
    };
  





    class MSetFactory : public MState
    {
      public:
        MSetFactory(MTools * Tools) : MState(Tools) {
          Oled->showLine4Text("   Factory   ");
          Oled->showLine3Text("     Y/NO    ");
          Oled->showLine2Text("  B-yes,  C-no  ");
        }
        virtual MState * fsm() override;
    };





    class MExit : public MState
    {
     public:  
        MExit(MTools * Tools) : MState(Tools) {}      
        virtual MState * fsm() override;
    };

};

#endif // !_OPTIONSFSM_H_