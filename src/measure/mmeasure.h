#ifndef _MMEASURE_H_
#define _MMEASURE_H_

#include "mstate.h"

class MTools;
class MBoard;
class MState;

class MMeasure
{
  public:
    MMeasure(MTools * tools);

    void run();
    //void delegateWork();

  private:
    MTools * Tools = nullptr;
    MBoard * Board = nullptr;

    MState * State = nullptr;
};

namespace MMeasureStates
{
    class MAdcVI : public MState
    {
        public:     
            MAdcVI(MTools * Tools) : MState(Tools) {}   
            virtual MState * fsm() override;
        private:
            int collectV    = 0;
            int collectI    = 0;
            int collectI3   = 0;
            int averageV    = 0;
            int averageI    = 0;
            int averageI3   = 0;
            int cntVI       = 0;
            float volt      = 0.0f;
    };
       
    class MAdcT : public MState
    {
        public:   
            MAdcT(MTools * Tools) : MState(Tools) {}     
            virtual MState * fsm() override;
    };
    
    class MAdcK : public MState
    {
        public:   
            MAdcK(MTools * Tools) : MState(Tools) {}     
            virtual MState * fsm() override;
    };
    
};

#endif  // _MMEASURE_H_