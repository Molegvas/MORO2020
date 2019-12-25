#ifndef _STORAGEFSM_H_
#define _STORAGEFSM_H_

#include "mstate.h"
#include "mtools.h"
#include "board/mboard.h"
#include "board/moverseer.h"
#include "display/moled.h"


namespace StorageFsm
{
    class MStart : public MState
    {       
     public:
        MStart(MTools * Tools) : MState(Tools) {
            // Параметры заряда из энергонезависимой памяти, при первом включении - заводские
            Tools->setVoltageMin( Tools->readNvsFloat("storage", "voltMin", 11.8f) );
            Tools->setCurrentDis( Tools->readNvsFloat("storage", "currDis",  1.0f) );

            // Индикация
            Oled->showLine4Text("    Разряд   ");
            Oled->showLine3Akb( Tools->getVoltageNom(), Tools->getCapacity() );              // example: "  12В  55Ач  "
            Oled->showLine2Text(" P-корр.С-старт ");        // Активны две кнопки: P-сменить настройки, и C-старт
            Oled->showLine1Time(0);                         // уточнить
            //Oled->showLine1Ah(0.0);                         // уточнить
            Oled->showLine1Celsius( Board->Overseer->getCelsius() );
            Board->ledsOn();                                // Светодиод светится белым до старта заряда - режим выбран
        }
        virtual MState * fsm() override;
    };

    class MSetCurrentDis : public MState
    {
     public:     
        MSetCurrentDis(MTools * Tools) : MState(Tools) {
            // Индикация
            Oled->showLine4Text("  Imax разр. ");
            Oled->showLine3MaxI( Tools->getCurrentDis() );
            Tools->showUpDn(); // " UP/DN, В-выбор "
        }   
        virtual MState * fsm() override;
    };
       
    class MSetVoltageMin : public MState
    {
     public:   
        MSetVoltageMin(MTools * Tools) : MState(Tools) {
            // Индикация
            Oled->showLine4Text("  Vmin разр. ");
            Oled->showLine3MaxU( Tools->getVoltageMin() );
            Tools->showUpDn(); // " UP/DN, В-выбор "
        }     
        virtual MState * fsm() override;
    };
      
    class MExecution : public MState
    {
     public:   
        MExecution(MTools * Tools) : MState(Tools) {
            //Tools->clrAhCharge();                  // Обнуляются счетчики времени и отданного заряда

            // Задаются начальные напряжение и ток
            Board->setVoltageVolt( 0.0f );        //Tools->getVoltageMax() );         // Voltage limit
            Board->setCurrentAmp( 0.0f );
            Board->setDischargeAmp( Tools->getCurrentDis() );  // 
            Board->powOn();   Board->swOn();                    // Включение преобразователя и коммутатора. 
            Board->ledsGreen();                  // Зеленый светодиод - процесс разряда запущен

            // Задание отображения на экране дисплея построчно (4-я строка - верхняя)
            Oled->showLine4RealVoltage();
            Oled->showLine3RealCurrent();
            Oled->showLine2Text(" Идёт разряд... ");            // 
            Oled->showLine1Time( Tools->getChargeTimeCounter() );
            //Oled->showLine1Ah( Tools->getAhCharge() );
            Oled->showLine1Celsius( Board->Overseer->getCelsius() );


    //      Tools->setSetPoint( Tools->getVoltageMax() );

        //   #ifdef DEBUG_SUPPLY
        //       Serial.println(" Источник питания стартовал с параметрами:");
        //       Serial.print("  Voltage max, B  *** = ");  Serial.println( Tools->getVoltageMax() );
        //       Serial.print("  Current max, A  *** = ");  Serial.println( Tools->getCurrentMax() );
        //   #endif

//                                                            // задающей напряжение цепи

        }     
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

#endif // !_STORAGEFSM_H_