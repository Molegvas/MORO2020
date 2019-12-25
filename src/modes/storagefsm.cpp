/*
 * storagefsm.cpp
 * В прототипе - опция расширенного заряда, в этой версии STORAGE, Storage
 * На дисплее при выборе режима отображается как "    Хранение    "
 * Проект: Q921-502 06.06.2019         !!! Тестовый разряд - эксперимент по определению эффективности теплоотвода !!!
 * 
*/

#include "modes/storagefsm.h"
#include "measure/mkeyboard.h"
#include <Arduino.h>

namespace StorageFsm
{
    // Старт тестового разряда
    MState * MStart::fsm()
    {
        if( Keyboard->getKey(MKeyboard::C_CLICK))
        {
            // Старт без уточнения параметров (быстрый старт),
            // максимальный ток и напряжение окончания - паспортные, исходя из параметров АКБ
            // Восстановленные на предыдущем шаге данные заменяются на паспортные
            Tools->setVoltageMin( Tools->getVoltageNom() * 0.96f );
            Tools->setCurrentMax( Tools->getCapacity() / 10.0f );

            return new MExecution(Tools);
        }

        if( Keyboard->getKey(MKeyboard::P_CLICK)) return new MSetCurrentDis(Tools);
        return this;                                    // Состояние не меняется
    };

    // Выбор максимального тока разряда
    MState * MSetCurrentDis::fsm()
    {
        // Выход без сохранения корректируемого параметра
        if( Keyboard->getKey(MKeyboard::C_LONG_CLICK)) { Tools->shutdownCharge(); return new MStop(Tools); }

        if( Keyboard->getKey(MKeyboard::UP_CLICK))      { Tools->incCurrentDis( 0.05, false ); return this; }     // Добавить 100 мА
        if( Keyboard->getKey(MKeyboard::DN_CLICK))      { Tools->decCurrentDis( 0.05f, false ); return this; }
        //if( Keyboard->getKey(MKeyboard::UP_LONG_CLICK)) { Tools->incCurrentDis( 0.25f, false ); return this; }     // Добавить 500 мА
        //if( Keyboard->getKey(MKeyboard::DN_LONG_CLICK)) { Tools->decCurrentDis( 0.25f, false ); return this; }

        if( Keyboard->getKey(MKeyboard::B_CLICK))
        {
            Tools->writeNvsFloat( "storage", "currDis", Tools->getCurrentDis() );
            return new MSetVoltageMin(Tools);
        }
        return this;
    };

    // Выбор напряжения окончания разряда
    MState * MSetVoltageMin::fsm()
    {
        // Выход без сохранения корректируемого параметра
        if( Keyboard->getKey(MKeyboard::C_LONG_CLICK)) { Tools->shutdownCharge(); return new MStop(Tools); }

        if( Keyboard->getKey(MKeyboard::UP_CLICK))      { Tools->incVoltageMin( 0.1f, false ); return this; }
        if( Keyboard->getKey(MKeyboard::DN_CLICK))      { Tools->decVoltageMin( 0.1f, false ); return this; }
        if( Keyboard->getKey(MKeyboard::UP_LONG_CLICK)) { Tools->incVoltageMin( 1.0f, false ); return this; }
        if( Keyboard->getKey(MKeyboard::DN_LONG_CLICK)) { Tools->decVoltageMin( 1.0f, false ); return this; }

        if( Keyboard->getKey(MKeyboard::B_CLICK))
        {   // Выбор заносится в энергонезависимую память
            Tools->writeNvsFloat( "storage", "voltMin", Tools->getVoltageMin() );
            return new MExecution(Tools);
        }
        return this;
    };

    MState * MExecution::fsm()
    {
        Tools->chargeCalculations();

        if(Keyboard->getKey(MKeyboard::C_CLICK)) { return new MStop(Tools); } 

        // Проверка напряжения и переход на поддержание напряжения.
        if( Board->getVoltage() <= Tools->getVoltageMin() ) { return new MStop(Tools); }

        // Do nothing
        return this;
    };

    // Процесс выхода из тестового режима - до нажатия кнопки "С" удерживается индикация
    // о продолжительности и отданном заряде.
    MState * MStop::fsm()
    {
        if(Keyboard->getKey(MKeyboard::C_CLICK)) 
        {
            Tools->activateExit("    Хранение    ");
            // #ifdef DEBUG_SUPPLY
            //     Serial.println("DcSupply: Exit"); 
            //     Serial.print("\t charge = ");   Serial.println( Tools->getAhCharge() );
            // #endif  
            return 0;   // Возврат к выбору режима
        }
        return this;
    };

};
