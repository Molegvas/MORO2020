/*
  Стадия экспериментов
*/


#ifndef _PULSEGENFSM_H_
#define _PULSEGENFSM_H_

#include "mtools.h"
#include "board/mboard.h"
#include "display/moled.h"
#include "mstate.h"

namespace PulseGenFsm
{
    class MStart : public MState
    {       
      public:
        MStart(MTools * Tools) : MState(Tools) {
        Tools->setVoltageMax( Tools->readNvsFloat("e-power", "voltMax", Tools->pows[4][0]) );   // Начальный выбор 12.0 вольт
        Tools->setCurrentMax( Tools->readNvsFloat("e-power", "currMax", Tools->pows[4][1]) );   //                  3.0 А
        Tools->powO         = Tools->readNvsFloat("e-power", "powO",      0.0f);              // Планируется для быстрого входа в режим
        Tools->setDurationOn( Tools->readNvsFloat("e-power", "duratOn",   5.0f ) );
        Tools->setDurationOff( Tools->readNvsFloat("e-power", "duratOff",  2.5f ) );
        
        #ifdef DEBUG_SUPPLY
            Serial.print("Voltage     : "); Serial.println(Tools->getVoltageMax(), 1);         // формат XX.X
            Serial.print("Max Current : "); Serial.println(Tools->getCurrentMax(), 1);
        #endif

              // Подготовка индикации
          Oled->showLine4Text("   Источник  ");
          Oled->showLine3Power( Tools->getVoltageMax(), Tools->getCurrentMax() ); // example: " 12.3В  3.3А "
          Oled->showLine2Text(" P-корр.С-старт ");        // Активны две кнопки: P-сменить настройки, и C-старт
          Oled->showLine1Time(0);                         // уточнить
          Oled->showLine1Ah(0.0);                         // уточнить
    Tools->shutdown();

          Board->ledsOn();                                // Светодиод светится белым до старта - режим выбран
          #ifdef DEBUG_SUPPLY
              Serial.println("exPower: Start");
          #endif

        }
        virtual MState * fsm() override;
    };

    class MSelectUI : public MState
    {
      public:     
        MSelectUI(MTools * Tools) : MState(Tools) {
          Oled->showLine4Text("  Выбор U&I  ");
          Tools->showUpDn();                          // Подсказка: UP/DN - листать список, В - выбрать"
        }   
        virtual MState * fsm() override;
    };
       
    class MSetCurrentMax : public MState
    {
      public:   
        MSetCurrentMax(MTools * Tools) : MState(Tools) {}     
        virtual MState * fsm() override;
      private:
        const float curr_l =  0.2f;
        const float curr_h = 12.2f;
    };
    
    class MSetVoltage : public MState
    {
      public:   
        MSetVoltage(MTools * Tools) : MState(Tools) {
          Oled->showLine4Text("  Напряжение ");
          Oled->showLine3MaxU( Tools->getVoltageMax() );
          Tools->showUpDn();                        // " UP/DN, В-выбор "
        }     
        virtual MState * fsm() override;
      private:
        const float volt_l = 1.00f;
        const float volt_h = 16.0f;
    };
    
    class MSetDurationOn : public MState
    {
        public:   
            MSetDurationOn(MTools * Tools) : MState(Tools) {
                Oled->showLine4Text("   Импульс   ");
                Oled->showLine3Sec( Tools->getDurationOn() );
                Tools->showUpDn();                      // " UP/DN, В-выбор "
            }     
            virtual MState * fsm() override;
        private:
            const float pulse_l  =  5.0f;
            const float pulse_h  = 30.0f;
    };

    class MSetDurationOff : public MState
    {
        public:   
            MSetDurationOff(MTools * Tools) : MState(Tools) {
                Oled->showLine4Text("    Пауза    ");
                Oled->showLine3Sec( Tools->getDurationOff() );
                Tools->showUpDn();                      // " UP/DN, В-выбор "
            }     
            virtual MState * fsm() override;
        private:
            const float pause_l  = 2.5f;
            const float pause_h  = 5.0f;
    };

    class MInit : public MState
    {
      public:   
        MInit(MTools * Tools) : MState(Tools) {
          Tools->clrAhCharge();                  // Обнуляются счетчики времени и отданного заряда

          // // Задаются начальные напряжение и ток
          Board->setVoltageVolt( 0.0f );
          Board->setCurrentAmp( 0.0f );
          Board->powOn();   

          // Задание отображения на экране дисплея построчно (4-я строка - верхняя)
          Oled->showLine4RealVoltage();
          Oled->showLine3RealCurrent();

          // Настройка ПИД-регулятора
          Tools->initPid( outputMin, outputMax, k_p, k_i, k_d, bangMin, bangMax, timeStep );
          Tools->saveIntegral( Tools->getVoltageMax() * 8.0f );  //10.0f );
        }     
        virtual MState * fsm() override;
      private:
        //pid settings and gains
        float outputMin        =   0.0f;
        float outputMax        = Tools->getVoltageMax() + 0.3f;
        float bangMin          =  20.0f;     // За пределами, - отключить
        float bangMax          =  20.0f;
        unsigned long timeStep = 100; //500 ?
        // Подобранные значения для ПИД
        float k_p              = 0.26f;  //= 0.13f;
        float k_i              = 0.20f; //= 0.10f;
        float k_d              = 0.08f; //= 0.04f;
    };

    class MPulse : public MState
    {
      public: 
        MPulse(MTools * Tools) : MState(Tools) {

          // Задаются начальные напряжение и ток ( эксперименты не закончены )
          Board->swOn();                                      // Включение коммутатора. 
          Board->ledsOff(); Board->ledGOn();                  // Зеленый светодиод - импульс

          // Задание отображения на экране дисплея построчно (4-я строка - верхняя)
          Oled->showLine2Text(" Импульс...     ");        // " Время:  Заряд: "

          Tools->setSetPoint( Tools->getVoltageMax() );
          Tools->saveIntegral( Tools->getVoltageMax() * 10.0f );  //10.0f );
        }
        virtual MState * fsm() override;
      private:
        // Задать длительность импульса, пересчитав секунды в число циклов
        int cycles = (int)( Tools->getDurationOn() * 10.0f );    // Всего циклов в импульсе

        // Пределы регулирования напряжения "на лету" ( вынужденное дублирование )
        const float volt_l = 1.00f;
        const float volt_h = 16.0f;

    };

    class MPause : public MState
    {
      public: 
        MPause(MTools * Tools) : MState(Tools) {
          Board->swOff();                                                     // Выключение коммутатора. 
          Board->ledsOff(); Board->ledGOn();  Board->ledROn();                // Желтый светодиод - пауза

          // Задание отображения на экране дисплея построчно (4-я строка - верхняя)
          Oled->showLine4RealVoltage();
          Oled->showLine3RealCurrent();
          Oled->showLine2Text("       ...Пауза ");        // " Время:  Заряд: "
          Oled->showLine1Time( Tools->getChargeTimeCounter() );
          Oled->showLine1Ah( Tools->getAhCharge() );
        }       
        virtual MState * fsm() override;
      private:
        // Задать длительность паузы, пересчитав секунды в число циклов
        int cycles = (int)( Tools->getDurationOff() * 10.0f );    // Всего циклов в паузе
    };
  
    class MExit : public MState
    {
      public:  
        MExit(MTools * Tools) : MState(Tools) {
          Tools->powShutdown();
        }      
        virtual MState * fsm() override;
    };

};

#endif // !_PULSEGENFSM_H_