#ifndef _EXCHARGERFSM_H_
#define _EXCHARGERFSM_H_

#include "mstate.h"
#include "mtools.h"
#include "board/mboard.h"
#include "display/moled.h"

namespace ExChargerFsm
{
    class MStart : public MState
    {       
        public:
            MStart(MTools * Tools) : MState(Tools) {
            // При выборе режима основные параметры восстанавливаются такими, какими они были в крайнем
            // сеансе работы в режиме заряда АКБ из энергонезависимой памяти (qulon, e-charge namespaces).
            // В случае первого запуска с чистой памятью - значения дефолтные.
            // Выбор акб при включении прибора - см конструктор MDdispatcher.
//            Tools->postpone    = Tools->readNvsInt("qulon", "postp",  0 );
            Tools->setVoltageMax( Tools->readNvsFloat("e-charge", "voltMax", 14.8f ) );
            Tools->setCurrentMax( Tools->readNvsFloat("e-charge", "currMax",  5.5f ) );
            Tools->setVoltagePre( Tools->readNvsFloat("e-charge", "voltPre", 12.2f ) );
            Tools->setCurrentPre( Tools->readNvsFloat("e-charge", "currPre",  3.5f ) );

            Tools->setDurationOn( Tools->readNvsFloat("e-charge", "duratOn",  5.0f ) );
            Tools->setDurationOff( Tools->readNvsFloat("e-charge", "duratOff", 2.5f ) );

            // Индикация при старте
            Oled->showLine4Text("   Заряжать  ");
            Oled->showLine3Akb( Tools->getVoltageNom(), Tools->getCapacity() );       // example: "  12В  55Ач  "
            Oled->showLine2Text(" P-корр.С-старт ");        // Активны две кнопки: P-сменить настройки, и C-старт
            Oled->showLine1Time( 0 );                         // уточнить
            Oled->showLine1Ah( 0.0f );                         // уточнить
            #ifdef V22
                Board->ledsOn();                                // Светодиод светится белым до старта заряда - режим выбран
            #endif
            }
        virtual MState * fsm() override;
    };

    class MSetFactory : public MState
    {
        public:
            MSetFactory(MTools * Tools) : MState(Tools) {
                // Индикация
                Oled->showLine4Text("   Factory   ");
                Oled->showLine3Text("     Y/NO    ");
                Oled->showLine2Text("  B-yes,  C-no  ");
            }
        virtual MState * fsm() override;
    };

    class MSetCurrentPre : public MState
    {
        public:
            MSetCurrentPre(MTools * Tools) : MState(Tools) {
                // Индикация
                Oled->showLine4Text(" Предзаряд до");
                Oled->showLine3MaxI( Tools->getCurrentPre() );
                Tools->showUpDn();         // " UP/DN, В-выбор "
            }     
        virtual MState * fsm() override;
    };
    
    class MSetVoltagePre : public MState
    {
        public:   
            MSetVoltagePre(MTools * Tools) : MState(Tools) {
                Oled->showLine4Text(" Предзаряд до");
                Oled->showLine3MaxU( Tools->getVoltagePre() );
                Tools->showUpDn();      // " UP/DN, В-выбор "
            }     
        virtual MState * fsm() override;
    };

    class MSetCurrentMax : public MState
    {
        public:   
            MSetCurrentMax(MTools * Tools) : MState(Tools) {
                Oled->showLine4Text(" Ток заряда  ");
                Oled->showLine3MaxI( Tools->getCurrentMax() );
                Tools->showUpDn();      // " UP/DN, В-выбор "
            }     
        virtual MState * fsm() override;
    };
    
    class MSetVoltageMax : public MState
    {
        public:   
            MSetVoltageMax(MTools * Tools) : MState(Tools) {
                Oled->showLine4Text(" Заряжать до ");
                Oled->showLine3MaxU( Tools->getVoltageMax() );
                Tools->showUpDn();      // " UP/DN, В-выбор "
            }     
        virtual MState * fsm() override;
    };

    class MSetDurationOn : public MState
    {
        public:   
            MSetDurationOn(MTools * Tools) : MState(Tools) {
                Oled->showLine4Text("   Заряд     ");
                Oled->showLine3Sec( Tools->getDurationOn() );
                Tools->showUpDn();      // " UP/DN, В-выбор "
            }     
        virtual MState * fsm() override;
    };

    class MSetDurationOff : public MState
    {
        public:   
            MSetDurationOff(MTools * Tools) : MState(Tools) {
                Oled->showLine4Text("Разряд(Пауза)");
                Oled->showLine3Sec( Tools->getDurationOff() );
                Tools->showUpDn();      // " UP/DN, В-выбор "
            }     
        virtual MState * fsm() override;
    };

// Задержка включения (отложенный старт)
    class MPostpone : public MState
    {
        public:   
            MPostpone(MTools * Tools) : MState(Tools) {
                // Параметр задержки начала заряда из энергонезависимой памяти, при первом включении - заводское
                Tools->postpone = Tools->readNvsInt( "qulon", "postp", 0 );
                
                // Индикация
                Oled->showLine4RealVoltage();
                Oled->showLine3RealCurrent();
                Oled->showLine2Text(" До старта...   ");

                // Инициализация счетчика времени до старта
                Tools->setTimeCounter( Tools->postpone * 36000 );                // Отложенный старт ( * 0.1s )
            }     
        virtual MState * fsm() override;
    };

    class MPreliminaryCharge : public MState
    {
        public:
            MPreliminaryCharge(MTools * Tools) : MState(Tools) {
                // Обнуляются счетчики времени и отданного заряда
                Tools->clrTimeCounter();
                Tools->clrAhCharge();

                // Включение преобразователя и коммутатора (уточнить порядок?)
                Board->setDischargeAmp( 0.0f );
                Board->setCurrentAmp( 0.0f );               // Ток в начале будет ограничен
                Board->powOn();     Board->swOn();          // Включение преобразователя и коммутатора. 
                #ifdef V22
                    Board->ledsOff();   Board->ledGOn();        // Зеленый светодиод - процесс заряда запущен
                #endif
                // Индикация построчно (4-я строка - верхняя)
                Oled->showLine4RealVoltage();
                Oled->showLine3RealCurrent();
                Oled->showLine2Text("  Предзаряд...  ");
                Oled->showLine1Time( Tools->getChargeTimeCounter() );
                Oled->showLine1Ah( Tools->getAhCharge() );

                // Настройка ПИД-регулятора
                Tools->initPid( outputMin, outputMax, k_p, k_i, k_d, bangMin, bangMax, timeStep );
                Tools->setSetPoint( Tools->getVoltagePre() );                                      //лишнее?

                // ???
                Board->setVoltageVolt( Tools->getVoltagePre() * 1.05f );     // Voltage limit
            }
        virtual MState * fsm() override;

        private:
            //pid settings and gains
            float outputMin        =  0.0f;
            float outputMax        = Tools->getCurrentPre() * 1.05f;         // Current limit
            float bangMin          = 20.0f;     // За пределами, - отключить
            float bangMax          = 20.0f;
            unsigned long timeStep  =  100;
            // Подобранные значения для ПИД, и в последующих фазах, если эти настройки будут устраивать
            float k_p              = 0.13f;
            float k_i              = 0.10f;
            float k_d              = 0.04f;
    };

    class MPulseUp : public MState
    {
        public:   
            MPulseUp(MTools * Tools) : MState(Tools) {
                range = Tools->getCurrentAvr() + delta;                     // Приращение импульса тока

                // Проверка корректности задания тока
                if( range > currentLimit ) range = currentLimit;
                if( range > Tools->getCurrentMax() ) range = Tools->getCurrentMax();
                Tools->initPulse( range );                                  // 

                #ifdef PID_CHARGE
                    Tools->setSetPoint( Tools->getVoltageMax() * 0.96f );     //_setpoint * 0.96;            // Выброс напряжения возможен
                #else
                    Tools->setCurrentAvr( range );                              // Полагаемся на калибровку генератора (без ПИДа)
                #endif

                // Индикация величины тока при импульсе заряда: среднее или текущее
                #ifdef AVR_CURRENT
                    Oled->showLine3Avr( Tools->getCurrentAvr() );               // Среднее за предыдущий импульс
                #elif
                    Oled->showLine3RealCurrent();
                #endif
                Oled->showLine2Text(" Время:  Заряд: ");
            }     
        virtual MState * fsm() override;

        private:
            float range;                            // Задает ток для импульса
            const float currentLimit = 12.0f;       // The instrument current limit (Здесь может быть другой предел)
            const float delta = 0.250f;             // приращение тока за один импульс заряда, А
    };
 
    class MDischUp : public MState
    {
        public:   
            MDischUp(MTools * Tools) : MState(Tools) {

                Tools->initPulse( -Tools->getCurrentAvr() / 10.0f );    // Ток отрицательный – инициируется разряд заданным током

                // Индикация при импульсе разряда
                Oled->showLine3RealCurrent();
                Oled->showLine2Text(" Время: Разряд: ");
            }     
        virtual MState * fsm() override;
    };

// **************************************************** июнь 2019  
    class MImpulsDischarge2 : public MState
    {
        public:   
            MImpulsDischarge2(MTools * Tools) : MState(Tools) {}     
            virtual MState * fsm() override;
    };

    class MImpulsCharge2 : public MState
    {
        public:   
            MImpulsCharge2(MTools * Tools) : MState(Tools) {}     
            virtual MState * fsm() override;
    };

    class MStop : public MState
    {
        public:  
            MStop(MTools * Tools) : MState(Tools) {
                Tools->shutdownCharge();            // Включение красного светодиода здесь
            }      
         virtual MState * fsm() override;
    };
};

#endif // !_EXCHARGERFSM_H_
