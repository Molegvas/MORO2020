#ifndef _OPTIONSFSM_H_
#define _OPTIONSFSM_H_

// #include "mtools.h"           //-
// #include "board/mboard.h"     //-
// #include "display/moled.h"    //-
#include "mstate.h"

namespace OptionFsm
{
    struct MOptConsts
    {
        // Примеры
        static constexpr float offsetV  = 0.0f;
        static constexpr float multV    = 1.0f;
    };

    class MStart : public MState
    {       
        public:
            MStart(MTools * Tools);
            virtual MState * fsm() override;
    };

    class MSelectBattery : public MState
    {
        public:     
            MSelectBattery(MTools * Tools);   
            virtual MState * fsm() override;
    };

    class MSetPostpone : public MState
    {       
        public:
            MSetPostpone(MTools * Tools);
            virtual MState * fsm() override;
    };

    class MSetCurrentOffset : public MState
    {
        public:
            MSetCurrentOffset(MTools * Tools);
            virtual MState * fsm() override;
    };

  class MSetVoltageOffset : public MState
    {
        public:
            MSetVoltageOffset(MTools * Tools);
            virtual MState * fsm() override;
    };
  





    // class MSetFactory : public MState
    // {
    //   public:
    //     MSetFactory(MTools * Tools) : MState(Tools) {
    //       Oled->showLine4Text("   Factory   ");
    //       Oled->showLine3Text("     Y/NO    ");
    //       Oled->showLine2Text("  B-yes,  C-no  ");
    //     }
    //     virtual MState * fsm() override;
    // };





    class MStop : public MState
    {
        public:  
          MStop(MTools * Tools); // : MState(Tools) {}      
          virtual MState * fsm() override;
    };

};

#endif // !_OPTIONSFSM_H_