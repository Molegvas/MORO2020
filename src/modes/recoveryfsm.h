#ifndef _RECOVERYFSM_H_
#define _RECOVERYFSM_H_

#include "mstate.h"

namespace RecoveryFsm
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
       
    class MSetCurrentMax : public MState
    {
        public:   
            MSetCurrentMax(MTools * Tools) : MState(Tools) {}     
            virtual MState * fsm() override;
    };
    
    class MSetvoltageMax : public MState
    {
        public:   
            MSetvoltageMax(MTools * Tools) : MState(Tools) {}     
            virtual MState * fsm() override;
    };
    
    class MSetDurationOn : public MState
    {
        public:   
            MSetDurationOn(MTools * Tools) : MState(Tools) {}     
            virtual MState * fsm() override;
    };

    class MSetDurationOff : public MState
    {
        public:   
            MSetDurationOff(MTools * Tools) : MState(Tools) {}     
            virtual MState * fsm() override;
    };

    class MPreliminaryCharge : public MState
    {
        public:
            MPreliminaryCharge(MTools * Tools) : MState(Tools) {}
            virtual MState * fsm() override;
    };

    class MIdleCharge : public MState
    {
        public:
            MIdleCharge(MTools * Tools) : MState(Tools) {}
            virtual MState * fsm() override;
    };

    class MPhase1 : public MState
    {
     public:   
        MPhase1(MTools * Tools) : MState(Tools) {}     
        virtual MState * fsm() override;
    };

    class MPhase2recharge : public MState
    {
     public: 
        MPhase2recharge(MTools * Tools) : MState(Tools) {}       
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

#endif // !_RECOVERYFSM_H_