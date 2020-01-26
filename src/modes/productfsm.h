#ifndef _PRODUCTFSM_H_
#define _PRODUCTFSM_H_

#include "mstate.h"

namespace ProductFsm
{
    struct MChConsts
    {
        // Параметры условий заряда (здесь – для батарей типа AGM)
        static constexpr float voltageMaxFactor     = 1.234f;    // 12v  * 1.234 = 14.8v
        static constexpr float voltageMinFactor     = 0.890f;    // 12v  * 0.89  = 10.7v
        static constexpr float currentMaxFactor     = 0.200f;    // 55ah * 0.1   = 5,5A 
        static constexpr float currentMinFactor     = 0.050f;    // 55ah * 0.05  = 2.75A
    };

    //pid settings and gains
    struct MPidConstants
    {

        // Параметры регулирования
        static constexpr float outputMin            = 0.0f;
        static constexpr float outputMaxFactor      = 1.05f;     // factor for current limit
        static constexpr float bangMin              = 20.0f;     // За пределами, - отключить
        static constexpr float bangMax              = 20.0f;
        static constexpr unsigned long timeStep     = 100;
        // Подобранные значения для ПИД, фаза подъёма тока и далее, если в последующих фазах эти настройки будут устраивать
        static constexpr float k_p                  = 0.13f;
        static constexpr float k_i                  = 0.10f;
        static constexpr float k_d                  = 0.04f;
    };

    class MStart : public MState
    {       
        public:
            MStart(MTools * Tools);
            virtual MState * fsm() override;
    };

    // class MSetFactory : public MState
    // {
    //     public:
    //         MSetFactory(MTools * Tools);
    //         virtual MState * fsm() override;
    // };

    class MChargeCurrent : public MState
    {
        public:   
            MChargeCurrent(MTools * Tools);
            virtual MState * fsm() override;
    };
    
    // class MSetVoltageMax : public MState
    // {
    //     public:   
    //         MSetVoltageMax(MTools * Tools);
    //         virtual MState * fsm() override;
    // };

    class MDisChargeCurrent : public MState
    {
        public:     
            MDisChargeCurrent(MTools * Tools);
            virtual MState * fsm() override;
    };

    class MVoltageUp4v : public MState
    {
        public:     
            MVoltageUp4v(MTools * Tools);
            virtual MState * fsm() override;
    };

    class MVoltageLov6v : public MState
    {
        public:   
            MVoltageLov6v(MTools * Tools);
            virtual MState * fsm() override;
    };

    class MCurrent712 : public MState
    {
        public:   
            MCurrent712(MTools * Tools);
            virtual MState * fsm() override;
    };

    class MCurrentShunt : public MState
    {
        public: 
            MCurrentShunt(MTools * Tools);
            virtual MState * fsm() override;
    };

    // class MKeepVmin : public MState
    // {
    //     public:   
    //         MKeepVmin(MTools * Tools);
    //         virtual MState * fsm() override;
    // };
  
    class MStop : public MState
    {
        public:  
            MStop(MTools * Tools);
            virtual MState * fsm() override;
    };
};

#endif  // !_PRODUCTFSM_H_