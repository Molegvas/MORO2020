/*


*/

#include "modes/optionsfsm.h"
#include "measure/mkeyboard.h"
#include <Arduino.h>

namespace OptionFsm
{
    MState * MStart::fsm()
    {
        #ifdef DEBUG_OPTIONS
            Serial.println("Options: Start");
        #endif

        Tools->activateSelBat();
 

        return new MSelectBattery(Tools);
    };

    MState * MSelectBattery::fsm()
    {
        if ( Keyboard->getKey(MKeyboard::C_CLICK)) { return new MExit(Tools); }    

        if( Keyboard->getKey(MKeyboard::UP_CLICK)) { Tools->incBattery(); return this; }
        if( Keyboard->getKey(MKeyboard::DN_CLICK)) { Tools->decBattery(); return this; } 

        if( Keyboard->getKey(MKeyboard::B_CLICK))              // Завершить выбор батареи
        {
//            Tools->saveBattery( "qulon" );           // Уточнить: общий для всех режимов?
        Tools->writeNvsInt( "qulon", "akbInd", Tools->getAkbInd() );

            return new MSetPostpone(Tools);
        }
        return this;
    };

    MState * MSetPostpone::fsm()
    {
        if ( Keyboard->getKey(MKeyboard::C_CLICK)) { return new MExit(Tools); }    

        if( Keyboard->getKey(MKeyboard::UP_CLICK)) { Tools->incPostpone( 1 ); return this; }
        if( Keyboard->getKey(MKeyboard::DN_CLICK)) { Tools->decPostpone( 1 ); return this; } 

        if( Keyboard->getKey(MKeyboard::B_CLICK))                                 // Завершить коррекцию величины задержки старта
        {
            Tools->saveInt( "qulon", "postp", Tools->postpone );            // Выбор заносится в энергонезависимую память
            return new MSetCurrentOffset(Tools);
        }
        return this;
    };

    MState * MSetCurrentOffset::fsm()
    {
        if ( Keyboard->getKey(MKeyboard::C_CLICK)) { return new MExit(Tools); }    

        if( Keyboard->getKey(MKeyboard::UP_CLICK)) { Tools->incCurrentOffset( 0.01f, false ); return this; }
        if( Keyboard->getKey(MKeyboard::DN_CLICK)) { Tools->decCurrentOffset( 0.01f, false ); return this; } 

        if( Keyboard->getKey(MKeyboard::B_CLICK))                                 // Завершить коррекцию величины смещения по току
        {
            Tools->saveFloat( "qulon", "cOffset", Board->currentOffset );   // Выбор заносится в энергонезависимую память
            return new MSetVoltageOffset(Tools);
        }
        return this;
    };

    MState * MSetVoltageOffset::fsm()
    {
        if ( Keyboard->getKey(MKeyboard::C_CLICK)) { return new MExit(Tools); }    

        if( Keyboard->getKey(MKeyboard::UP_CLICK)) { Tools->incVoltageOffset( 0.01f, false ); return this; }
        if( Keyboard->getKey(MKeyboard::DN_CLICK)) { Tools->decVoltageOffset( 0.01f, false ); return this; } 

        if( Keyboard->getKey(MKeyboard::B_CLICK))                                 // Завершить коррекцию величины
        {
            Tools->saveFloat( "qulon", "vOffset", Board->voltageOffset );   // Выбор заносится в энергонезависимую память
            return new MSetFactory(Tools);
        }
        return this;
    };

    MState * MSetFactory::fsm()
    {
        if ( Keyboard->getKey(MKeyboard::C_CLICK)) { return new MExit(Tools); }   // Отказ

        // Удаление всех старых ключей
        if( Keyboard->getKey(MKeyboard::B_CLICK)) {
            Tools->clearAllKeys ("qulon");
            // Tools->clearAllKeys ("s-power");
            // Tools->clearAllKeys ("e-power"); 
            // Tools->clearAllKeys ("cccv");
            // Tools->clearAllKeys ("recovery");
            // Tools->clearAllKeys ("e-charge");
            // Tools->clearAllKeys ("storage");
            // Tools->clearAllKeys ("service");
            // Tools->clearAllKeys ("option");
            return new MExit(Tools);
        }
        return this;
    };



    MState * MExit::fsm()
    {
//        if(Keyboard->getKey(MKeyboard::C_CLICK)) 
//        {
            Oled->showLine4RealVoltage();
            Oled->showLine3RealCurrent();
            Oled->showLine2Text("    Настройки   "); 
            
            Oled->showLine1Heap(ESP.getFreeHeap());
            #ifdef V22
                Board->ledsOff();      
            #endif
            #ifdef DEBUG_OPTIONS
                //Serial.println("Charger: Exit"); 
                //Serial.print("\t charge = ");   Serial.println( Tools->getAhCharge() );
            #endif  
            return 0;   // Возврат к выбору режима
//        }
//        return this;
    };


};

