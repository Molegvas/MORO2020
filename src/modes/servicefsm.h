#ifndef _SERVICEFSM_H_
#define _SERVICEFSM_H_

#include "mstate.h"

namespace ServiceFsm
{
    class MInvitation : public MState
    {
     public:
        MInvitation(MTools * Tools) : MState(Tools) {}
        virtual MState * fsm() override;
    };

    class MStart : public MState
    {       
     public:
        MStart(MTools * Tools) : MState(Tools) {}
        virtual MState * fsm() override;
    };

    class MSelectBattery : public MState
    {
     public:     
        MSelectBattery(MTools * Tools) : MState(Tools) {}   
        virtual MState * fsm() override;
    };

    class MSetCapacity : public MState
    {
        public:
            MSetCapacity(MTools * Tools) : MState(Tools) {}
            virtual MState * fsm() override;
    };


    // Параметры заряда   
    class MSetCurrentMax : public MState
    {
     public:   
        MSetCurrentMax(MTools * Tools) : MState(Tools) {}     
        virtual MState * fsm() override;
    };
    
    class MSetVoltageMax : public MState
    {
     public:   
        MSetVoltageMax(MTools * Tools) : MState(Tools) {}     
        virtual MState * fsm() override;
    };
    
    class MSetCycles : public MState
    {
     public:   
        MSetCycles(MTools * Tools) : MState(Tools) {}     
        virtual MState * fsm() override;
    };
  
    // Параметры разряда   
    class MSetCurrentDis : public MState
    {
     public:   
        MSetCurrentDis(MTools * Tools) : MState(Tools) {}     
        virtual MState * fsm() override;
    };
    
    class MSetVoltageDis : public MState
    {
     public:   
        MSetVoltageDis(MTools * Tools) : MState(Tools) {}     
        virtual MState * fsm() override;
    };
    
    class MSetPause : public MState
    {
     public:   
        MSetPause(MTools * Tools) : MState(Tools) {}     
        virtual MState * fsm() override;
    };

    // Процесс
    class MPhase1 : public MState
    {
     public:   
        MPhase1(MTools * Tools) : MState(Tools) {}     
        virtual MState * fsm() override;
    };
 
    class MPhase2 : public MState
    {
     public: 
        MPhase2(MTools * Tools) : MState(Tools) {}       
        virtual MState * fsm() override;
    };

    class MPhase3 : public MState
    {
     public:   
        MPhase3(MTools * Tools) : MState(Tools) {}     
        virtual MState * fsm() override;
    };

    class MExit : public MState
    {
     public:  
        MExit(MTools * Tools) : MState(Tools) {}      
        virtual MState * fsm() override;
    };
};

#endif // !_SERVICEFSM_H_