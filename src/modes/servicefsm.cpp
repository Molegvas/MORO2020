/*
 * servicefsm.cpp      - экперименты
 * В прототипе - сервис, в этой версии SERVICE, Service
 * На дисплее при выборе режима отображается как "Сервис" 
 * Проект: Q921-Andr 02.01.2019         !!! Местами настройки только для 12-вольтовой батареи !!!
 * Автор: Олег Морозов 
 * Консультант: Андрей Морозов
*/

#include "modes/servicefsm.h"
#include "mtools.h"
#include "board/mboard.h"
#include "measure/mkeyboard.h"
#include "display/moled.h"
#include "mstate.h"
#include <Arduino.h>

namespace ServiceFsm
{
    MState * MInvitation::fsm()
    {

//Tools->clearAllKeys ("service"); // Удаление всех старых ключей

        #ifdef DEBUG_SERVICE
            Serial.println("Service: Invitation");
        #endif
        Board->ledsOn();
        // При выборе режима основные параметры восстанавливаются такими, какими они были в крайнем
        // сеансе работы в режиме сервиса АКБ из энергонезависимой памяти (qulon, service namespaces).
        // В случае первого запуска с чистой памятью - значения дефолтные.
        // Выбор акб при включении прибора - см конструктор MDdispatcher.
        
//        Tools->akbInd        = Tools->readNvsInt  ("service", "akbInd", 3);                   // Индекс массива с набором батарей
//    Tools->akbInd = Tools->readNvsInt  ("qulon", "akbInd", 3);      // Индекс массива с набором батарей из "qulon"

//        Tools->voltageNom    = Tools->readNvsFloat("service", "akbU",   Tools->akb[3][0]);    // Начальный выбор 12 вольт
//        Tools->capacity      = Tools->readNvsFloat("service", "akbAh",  Tools->akb[3][1]);    //                 55 Ач
        Tools->setVoltageMax( Tools->readNvsFloat("service", "voltMax", 14.7f) );
        Tools->setCurrentMax( Tools->readNvsFloat("service", "currMax",  5.4f) );
        Tools->setNumCycles(  Tools->readNvsInt  ("service", "numCycl",  3) );                  //  Число циклов разряд-заряд
        Tools->setVoltageDis( Tools->readNvsFloat("service", "voltDis", 10.7f) );
        Tools->setCurrentDis( Tools->readNvsFloat("service", "currDis",  2.8f) );
        Tools->setPause(      Tools->readNvsInt  ("service", "pause",    1) );                  // Пауза перед разрядом, ч

        #ifdef DEBUG_SERVICE
            Serial.print("Akb Voltage: "); Serial.println(Tools->voltageNom,      0);   // формат XX
            Serial.print("Akb Ah     : "); Serial.println(Tools->getCapacity(),   0);
            Serial.print("Voltage End: "); Serial.println(Tools->getVoltageMax(), 2);   // формат XX.XX
            Serial.print("Current Max: "); Serial.println(Tools->getCurrentMax(), 2);
            Serial.print("Num Cycles : "); Serial.println(Tools->getNumCycles()    );   // формат XX
            Serial.print("Voltage Dis: "); Serial.println(Tools->getVoltageDis(), 2);   // формат XX.XX
            Serial.print("Current Dis: "); Serial.println(Tools->getCurrentDis(), 2);   // формат XX.XX
            Serial.print("Pause      : "); Serial.println(Tools->getPause()        );   // формат XX
        #endif

        Tools->activateChargeStart();    

        #ifdef DEBUG_SERVICE
            Serial.println("Service: Start");
        #endif
        return new MStart(Tools);
    };

    MState * MStart::fsm()
    {
        if(Keyboard->getKey(MKeyboard::C_CLICK)) 
        {
            // Старт без уточнения параметров (по дефолту), 
            #ifdef DEBUG_SERVICE
                Serial.println("Service: Start by default");
            #endif
            Tools->setVoltageMax( Tools->getVoltageNom() * 1.234f );                        // Например, voltageMax = 14.8;
            Tools->setCurrentMax( Tools->getCapacity() / 10.0f );

            Tools->clrTimeCounter();
            Tools->clrAhCharge();
            Tools->activateChargePh1( Tools->getVoltageMax(), Tools->getCurrentMax(), false );    // Активация фазы 1 заряда
            #ifdef DEBUG_SERVICE
                Serial.println("Service: Phase1 activated");
            #endif
            return new MPhase1(Tools);
        }

        if(Keyboard->getKey(MKeyboard::P_CLICK)) 
        {
            Tools->activateSelBat();
            return new MSelectBattery(Tools);   // Переход к выбору
        }
        return this;    
    };

    MState * MSelectBattery::fsm()
    {
        if(Keyboard->getKey(MKeyboard::UP_CLICK)) { Tools->incBattery(); return this; }
        if(Keyboard->getKey(MKeyboard::DN_CLICK)) { Tools->decBattery(); return this; } 

        if(Keyboard->getKey(MKeyboard::B_CLICK))          // Завершить выбор батареи
        {
            Tools->writeNvsInt( "qulon", "akbInd", Tools->getAkbInd() );

    //        Tools->activateSetCurrMax();
    //        return new MSetCurrentMax(Tools);
            Tools->activateSetCapacity();
            return new MSetCapacity(Tools);
        }
        return this;
    };

    MState * MSetCapacity::fsm()
    {
    if(Keyboard->getKey(MKeyboard::UP_CLICK)) { Tools->incCapacity( 1.0f, false ); return this; }
    if(Keyboard->getKey(MKeyboard::DN_CLICK)) { Tools->decCapacity( 1.0f, false ); return this; } 
        if(Keyboard->getKey(MKeyboard::B_CLICK))          // Завершить выбор ёмкости батареи
        {
            Tools->writeNvsFloat( "qulon", "akbAh", Tools->getCapacity() );
            Tools->activateSetCurrMax( true );
            return new MSetCurrentMax(Tools);
        }
        return this;
    };

    MState * MSetCurrentMax::fsm()
    {
        if(Keyboard->getKey(MKeyboard::UP_CLICK)) { Tools->incCurrentMax( 0.1f, false ); return this; }     // ++ 100mA
        if(Keyboard->getKey(MKeyboard::DN_CLICK)) { Tools->decCurrentMax( 0.1f, false ); return this; }     // -- 100mA  

        if(Keyboard->getKey(MKeyboard::B_CLICK))          // Завершить коррекцию максимального тока заряда
        {
            Tools->writeNvsFloat( "service", "currMax", Tools->getCurrentMax() );
            Tools->activateSetVoltMax( true );               // Activate next step
            return new MSetVoltageMax(Tools);
        }
        return this;
    };

    MState * MSetVoltageMax::fsm()
    {
        if(Keyboard->getKey(MKeyboard::UP_CLICK)) { Tools->incVoltageMax( 0.1f, false ); return this; }
        if(Keyboard->getKey(MKeyboard::DN_CLICK)) { Tools->decVoltageMax( 0.1f, false ); return this; } 

        if(Keyboard->getKey(MKeyboard::B_CLICK))          // Завершить коррекцию величины напряжения окончания заряда
        {
            Tools->writeNvsFloat( "service", "voltMax", Tools->getVoltageMax() );

            Tools->activateSetNumCycl();
            return new MSetCycles(Tools);
        }
        return this;
    };

    MState * MSetCycles::fsm()
    {
        if(Keyboard->getKey(MKeyboard::UP_CLICK)) { Tools->incCycles(); return this; }
        if(Keyboard->getKey(MKeyboard::DN_CLICK)) { Tools->decCycles(); return this; } 

        if(Keyboard->getKey(MKeyboard::B_CLICK))          // Завершить коррекцию счетчика циклов
        {
            Tools->writeNvsInt( "service", "numCycl", Tools->getNumCycles() );    
            Tools->activateSetCurrDis();
            return new MSetCurrentDis(Tools);
        }
        return this;
    };

    MState * MSetCurrentDis::fsm()
    {
        if(Keyboard->getKey(MKeyboard::UP_CLICK)) { Tools->incCurrentDis( 0.1f, false ); return this; }
        if(Keyboard->getKey(MKeyboard::DN_CLICK)) { Tools->decCurrentDis( 0.1f, false ); return this; } 

        if(Keyboard->getKey(MKeyboard::B_CLICK))          // Завершить коррекцию величины тока разряда
        {
            Tools->writeNvsFloat( "service", "currDis", Tools->getCurrentDis() );
            Tools->activateSetVoltDis();
            return new MSetVoltageDis(Tools);
        }
        return this;
    };

    MState * MSetVoltageDis::fsm()
    {
        if(Keyboard->getKey(MKeyboard::UP_CLICK)) { Tools->incVoltageDis( 0.1f, false ); return this; }
        if(Keyboard->getKey(MKeyboard::DN_CLICK)) { Tools->decVoltageDis( 0.1f, false ); return this; } 

        if(Keyboard->getKey(MKeyboard::B_CLICK))          // Завершить коррекцию величины напряжения окончания заряда
        {
            Tools->writeNvsFloat( "service", "voltDis", Tools->getVoltageDis() );

            Tools->activateSetPause();
            return new MSetPause(Tools);
        }
        return this;
    };
    
    MState * MSetPause::fsm()
    {
        if(Keyboard->getKey(MKeyboard::UP_CLICK)) { Tools->incPause(); return this; }
        if(Keyboard->getKey(MKeyboard::DN_CLICK)) { Tools->decPause(); return this; } 

        if(Keyboard->getKey(MKeyboard::B_CLICK))          // Завершить коррекцию счетчика паузы
        {
//            Tools->savePause( "service" );          // Выбор заносится в энергонезависимую память
    Tools->writeNvsInt  ( "service", "pause", Tools->getPause() );

            #ifdef DEBUG_SERVICE
                Tools->pause = Tools->readNvsInt("service", "pause", 1 );    // Тестовая проверка
                Serial.print("New Pause = "); Serial.println(Tools->pause);
                Serial.println("Service: Phase1 activated");
            #endif

            Tools->clrTimeCounter();
            Tools->clrAhCharge();
            Tools->activateChargePh1( Tools->getVoltageMax(), Tools->getCurrentMax(), false );    // Активация фазы 1 заряда

            #ifdef DEBUG_SERVICE
                Serial.println("Service: Phase1 activated");
            #endif
            return new MPhase1(Tools);
        }
        return this;
    };

    // Фаза 1 - ток поднимается не выше заданного уровня,
    // при достижении напряжения выше заданного уровня - переход ко второй фазе.
    // Здесь и далее подсчитывается время и отданный заряд, а также
    // сохраняется возможность прекращения заряда оператором.
    MState * MPhase1::fsm()
    {
        Tools->chargeCalculations();
        if (Keyboard->getKey(MKeyboard::C_CLICK)) { Tools->shutdownCharge();  return new MExit(Tools); }    

        // Здесь будет проверка напряжения - setpoint по напряжению
        //float setpoint = Tools->getVoltageMax() * 0.96f;      // Для второй фазы
        //if( Board->getVoltage() >= setpoint )           // 96% = 14.2В
        if( Board->getVoltage() >= Tools->getVoltageMax() )      // или voltageMax * 0.96 ?
        {
            if( Tools->getNumCycles() == Tools->getCycle() )                     // Последний цикл?
            {
                Tools->shutdownCharge();
                return new MExit(Tools);
            }

            Tools->activatePause();                           // Активировать паузу перед разрядом

    
            #ifdef DEBUG_SERVICE
                Serial.println("Charger: Phase2 activated");
            #endif
            return new MPhase2(Tools);                      // если достигли, иначе return this;
        }

        Tools->runChargePh1();
        return this;
    };

// Вторая фаза сервиса - Выдерживается пауза перед разрядом
// отображается обратный отсчет времени 
    MState * MPhase2::fsm()
    {
        if (Keyboard->getKey(MKeyboard::C_CLICK)) { Tools->shutdownCharge(); return new MExit(Tools); }    

        if( Tools->pauseCalculations() )
        {
            Tools->activateDischarge();                     // Пауза закончилась, активировать разряд 
            return new MPhase3(Tools);
        }
        return this;
    };

// Разряд
    MState * MPhase3::fsm()
    {
        //Tools->disChargeCalculations();      // Считать заряд в обратную сторону?
                Tools->chargeCalculations();

        if (Keyboard->getKey(MKeyboard::C_CLICK)) { Tools->shutdownCharge(); return new MExit(Tools); }    

        if( Board->getVoltage() <= Tools->getVoltageDis() )
        {
            Tools->clrTimeCounter();

            if( Tools->getCycle() == 0 ) Tools->clrChargeTimeCounter();   // Сброс показаний заряда перед первым полным циклом
            Tools->activateChargePh1( Tools->getVoltageMax(), Tools->getCurrentMax(), false );    // Активация фазы 1 заряда
            return new MPhase1(Tools);
        }
        return this;
    };

    // Процесс выхода из режима - до нажатия кнопки "С" удерживается индикация
    // о продолжительности и отданном заряде.
    MState * MExit::fsm()
    {
        if(Keyboard->getKey(MKeyboard::C_CLICK)) 
        {
            // Oled->showLine4RealVoltage();
            // Oled->showLine3RealCurrent();
            // Oled->showLine2Text("   Сервис АКБ   "); 
            
            // Oled->showLine1Heap(ESP.getFreeHeap());
            // Board->ledsOff();      

            #ifdef DEBUG_SERVICE
                Serial.println("Charger: Exit"); 
                Serial.print("\t charge = ");   Serial.println( Tools->getAhCharge() );
            #endif  
            Tools->activateExit("   Сервис АКБ   ");
            return 0;   // Возврат к выбору режима
        }
        return this;
    };

};
