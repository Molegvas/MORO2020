/*
 * pulsegen.cpp
 * В прототипе - Расширенный источник питания
 * 2019.01.14
 * Автомат Мили – это конечный автомат, где выходные сигналы являются функциями текущего состояния и входного сигнала.
 * Последующее состояние – это функция текущего состояния и входного сигнала. 
 */

#include "modes/pulsegenfsm.h"
#include "mtools.h"
#include "board/mboard.h"
#include "measure/mkeyboard.h"
#include "display/moled.h"
#include "mstate.h"
#include <Arduino.h>

namespace PulseGenFsm
{
    MState * MStart::fsm()
    {
        if(Keyboard->getKey(MKeyboard::C_CLICK)) 
        {
            // Старт без уточнения параметров (быстрый старт), 
            // максимальный ток и напряжение - паспортные (см таблицу в/А)
            // Восстановленные на предыдущем шаге данные заменяются (кроме индекса)
            Tools->setVoltageMax( Tools->pows[Tools->powInd][0] );
            Tools->setCurrentMax( Tools->pows[Tools->powInd][1] );

            Tools->activatePowerPh1( Tools->getVoltageMax(), Tools->getCurrentMax() );     // Активация фазы 1
            #ifdef DEBUG_SUPPLY
                Serial.println("PulseSupply: Phase1 activated");
            #endif
            return new MInit(Tools);
        }

        if(Keyboard->getKey(MKeyboard::P_CLICK)) return new MSelectUI(Tools);
        return this;
    };

    MState * MSelectUI::fsm()
    {
        if(Keyboard->getKey(MKeyboard::UP_CLICK)) { Tools->upPow(); return this; }
        if(Keyboard->getKey(MKeyboard::DN_CLICK)) { Tools->dnPow(); return this; } 

        if(Keyboard->getKey(MKeyboard::C_CLICK))
        {
            Tools->activatePowerPh1( Tools->getVoltageMax(), Tools->getCurrentMax() );     // Активация фазы 1 без сохранения выбора
            return new MInit(Tools);
        }

        if(Keyboard->getKey(MKeyboard::B_CLICK))      // Завершить выбор UI
        {
            Tools->savePowInd("qulon");
            Tools->activateSetCurrMax( false );
            return new MSetCurrentMax(Tools);
        }
        return this;
    };

    MState * MSetCurrentMax::fsm()
    {
        if(Keyboard->getKey(MKeyboard::C_CLICK)) return new MInit(Tools);

        if( Keyboard->getKey(MKeyboard::UP_CLICK)) {
            Tools->setCurrentMax( Tools->incFloatValue( Tools->getCurrentMax(), curr_l, curr_h, 0.10f ) );
            Oled->showLine3MaxI(Tools->getCurrentMax());
            return this;
        }

        if( Keyboard->getKey(MKeyboard::DN_CLICK)) {
            Tools->setCurrentMax( Tools->decFloatValue( Tools->getCurrentMax(), curr_l, curr_h, 0.10f ) );
            Oled->showLine3MaxI(Tools->getCurrentMax()); 
            return this;
        }

        if(Keyboard->getKey(MKeyboard::B_CLICK))      // Завершить коррекцию максимального тока
        {
            Tools->saveFloat( "e-power", "currMax", Tools->getCurrentMax() ); 
            return new MSetVoltage(Tools);
        }
        return this;
    };

    MState * MSetVoltage::fsm()
    {
        if(Keyboard->getKey(MKeyboard::C_CLICK)) return new MInit(Tools);

        if( Keyboard->getKey(MKeyboard::UP_CLICK)) {
            Tools->setVoltageMax( Tools->incFloatValue( Tools->getVoltageMax(), volt_l, volt_h, 0.10f ) );
            Oled->showLine3MaxU(Tools->getVoltageMax());
            return this;
        }

        if( Keyboard->getKey(MKeyboard::DN_CLICK)) {
            Tools->setVoltageMax( Tools->decFloatValue( Tools->getVoltageMax(), volt_l, volt_h, 0.10f ) );
            Oled->showLine3MaxU(Tools->getVoltageMax()); 
            return this;
        }

        if(Keyboard->getKey(MKeyboard::B_CLICK))      // Завершить коррекцию величины напряжения
        {
            Tools->saveFloat( "e-power", "voltMax", Tools->getVoltageMax() ); 
            return new MSetDurationOn(Tools);
        }
        return this;
    };

// Выбор продолжительности импульса
    MState * MSetDurationOn::fsm()
    {
        if(Keyboard->getKey(MKeyboard::C_CLICK)) return new MInit(Tools);

        if( Keyboard->getKey(MKeyboard::UP_CLICK)) {
            Tools->setDurationOn( Tools->incFloatValue( Tools->getDurationOn(), pulse_l, pulse_h, 0.5f ) );
            Oled->showLine3Sec(Tools->getDurationOn());
            return this;
        }

        if( Keyboard->getKey(MKeyboard::DN_CLICK)) {
            Tools->setDurationOn( Tools->decFloatValue( Tools->getDurationOn(), pulse_l, pulse_h, 0.5f ) );
            Oled->showLine3Sec(Tools->getDurationOn()); 
            return this;
        }

        if( Keyboard->getKey(MKeyboard::B_CLICK))
        {
            Tools->saveFloat( "e-power", "duratOn", Tools->getDurationOn() ); 
            return new MSetDurationOff(Tools);
        }
        return this;
    };

// Выбор продолжительности паузы
    MState * MSetDurationOff::fsm()
    {
        if(Keyboard->getKey(MKeyboard::C_CLICK)) return new MInit(Tools);

        if( Keyboard->getKey(MKeyboard::UP_CLICK)) {
            Tools->setDurationOff( Tools->incFloatValue( Tools->getDurationOff(), pause_l, pause_h, 0.5f ) );
            Oled->showLine3Sec(Tools->getDurationOff());
            return this;
        }

        if( Keyboard->getKey(MKeyboard::DN_CLICK)) {
            Tools->setDurationOff( Tools->decFloatValue( Tools->getDurationOff(), pause_l, pause_h, 0.5f ) );
            Oled->showLine3Sec(Tools->getDurationOff()); 
            return this;
        }

        if( Keyboard->getKey(MKeyboard::B_CLICK))
        {
            Tools->saveFloat( "e-power", "duratOff", Tools->getDurationOff() ); 
            return new MInit(Tools);                // Последний параметр скорректирован, старт автоматически
        }
        return this;
    };

// Инициализация регулятора
    MState * MInit::fsm()
    {
        // Здесь может быть отложенный старт 

        return new MPulse(Tools);
    };

// Генерация импульса заданной длительности
    MState * MPulse::fsm()
    {
        Tools->chargeCalculations();
        // Регулировка напряжения "на лету" по 100 или 500 мв
        if( Keyboard->getKey(MKeyboard::UP_CLICK)) {
            Tools->setVoltageMax( Tools->incFloatValue( Tools->getVoltageMax(), volt_l, volt_h, 0.10f ) );
            Oled->showLine3MaxU(Tools->getVoltageMax());
            return this;
        }

        if( Keyboard->getKey(MKeyboard::DN_CLICK)) {
            Tools->setVoltageMax( Tools->decFloatValue( Tools->getVoltageMax(), volt_l, volt_h, 0.10f ) );
            Oled->showLine3MaxU(Tools->getVoltageMax()); 
            return this;
        }

        if( Keyboard->getKey(MKeyboard::UP_LONG_CLICK)) {
            Tools->setVoltageMax( Tools->incFloatValue( Tools->getVoltageMax(), volt_l, volt_h, 0.50f ) );
            Oled->showLine3MaxU(Tools->getVoltageMax());
            return this;
        }

        if( Keyboard->getKey(MKeyboard::DN_LONG_CLICK)) {
            Tools->setVoltageMax( Tools->decFloatValue( Tools->getVoltageMax(), volt_l, volt_h, 0.50f ) );
            Oled->showLine3MaxU(Tools->getVoltageMax()); 
            return this;
        }

        if(Keyboard->getKey(MKeyboard::B_CLICK))
        { 
            Tools->saveFloat( "e-power", "voltMax", Tools->getVoltageMax() );
            Oled->showLine3RealCurrent();
        }   

        if(Keyboard->getKey(MKeyboard::C_CLICK)) return new MExit(Tools); 

        if( --cycles <= 0 )
        {
            return new MPause(Tools);
        }
//    Serial.println(cycles);

        Tools->runPidVoltage();
        return this;
    };



// Генерация паузы заданной длительности
    MState * MPause::fsm()
    {
        if(Keyboard->getKey(MKeyboard::C_CLICK)) return new MExit(Tools); 

        // Во время паузы регулировка напряжения невозможна, запись - да
        if(Keyboard->getKey(MKeyboard::B_CLICK))
        { 
            Tools->saveFloat( "e-power", "voltMax", Tools->getVoltageMax() );
            Oled->showLine3RealCurrent();
        }   


        if( --cycles <= 0 )
        {
            Tools->saveIntegral( Tools->getIntegral() );
            return new MPulse(Tools);
        }
    //Serial.println(-cycles);

//Serial.println( Board->getVoltage() );

        return this;
    };

    // Процесс выхода из режима - до нажатия кнопки "С" удерживается индикация
    // о продолжительности и отданном заряде.
    MState * MExit::fsm()
    {
        if(Keyboard->getKey(MKeyboard::C_CLICK)) 
        {
            Tools->activateExit(" PULSE источник ");
            #ifdef DEBUG_SUPPLY
                Serial.println("PulseGen: Exit"); 
                Serial.print("\t charge = ");   Serial.println( Tools->getAhCharge() );
            #endif  
            return 0;   // Возврат к выбору режима
        }
        return this;
    };


    float tttt;

};
