/*


*/

#include "mtools.h"
#include "board/mboard.h"
#include "display/moled.h"
#include "modes/optionsfsm.h"
#include "measure/mkeyboard.h"
#include <Arduino.h>

namespace OptionFsm
{
    // Переменные
    float vOffset   = 0.0f;

    MStart::MStart(MTools * Tools) : MState(Tools)
    {
        #ifdef DEBUG_OPTIONS
            Serial.println("Options: Start");
        #endif
        // Индикация
        #ifdef OLED_1_3
            Oled->showLine4Text("333Батарея   ");
            Oled->showLine3Akb( Tools->getVoltageNom(), Tools->getCapacity() );          // example: "  12В  55Ач  "
            Tools->showUpDn();                  // " UP/DN, В-выбор "
        #endif
        #ifdef V22
            Board->ledsOn();
        #endif
    }
    MState * MStart::fsm()
    {
        switch ( Keyboard->getKey() )
        {
        case MKeyboard::C_LONG_CLICK :
            Tools->clearAllKeys ("qulon");     // Удаление всех старых ключей (очистка)
            break;
        case MKeyboard::P_CLICK :
            // Продолжение выбора объекта настройки
            return new MSetPostpone(Tools);
        case MKeyboard::B_CLICK :
            return new MSelectBattery(Tools);      // Выбрана регулировка voltageDeltaPwm
        default :;
        }
        return this;
    };

    MSelectBattery::MSelectBattery(MTools * Tools) : MState(Tools)
    {
        // Индикация
        #ifdef OLED_1_3
            Oled->showLine4Text("   Батарея   ");
            Oled->showLine3Akb( Tools->getVoltageNom(), Tools->getCapacity() );          // example: "  12В  55Ач  "
            Tools->showUpDn();                  // " UP/DN, В-выбор "
        #endif
    }
    MState * MSelectBattery::fsm()
    {
//         if ( Keyboard->getKey(MKeyboard::C_CLICK)) { return new MExit(Tools); }    

//         if( Keyboard->getKey(MKeyboard::UP_CLICK)) { Tools->incBattery(); return this; }
//         if( Keyboard->getKey(MKeyboard::DN_CLICK)) { Tools->decBattery(); return this; } 

//         if( Keyboard->getKey(MKeyboard::B_CLICK))              // Завершить выбор батареи
//         {
// //            Tools->saveBattery( "qulon" );           // Уточнить: общий для всех режимов?
//         Tools->writeNvsInt( "qulon", "akbInd", Tools->getAkbInd() );

//             return new MSetPostpone(Tools);
//         }

        switch ( Keyboard->getKey() )
        {
        case MKeyboard::C_LONG_CLICK :
            return new MStop(Tools);
        case MKeyboard::UP_CLICK :
            Tools->incBattery();
            break;  //    return this; 
        case MKeyboard::DN_CLICK :
            Tools->decBattery();
            break;      // return this; } 
        case MKeyboard::P_CLICK :
            // Продолжение выбора объекта настройки
            return new MSetPostpone(Tools);

        case MKeyboard::B_CLICK :
            // Завершить выбор батареи
            Tools->writeNvsInt( "qulon", "akbInd", Tools->getAkbInd() );
            return new MSetPostpone(Tools);
        default :;
        }
        return this;
    };

    MSetPostpone::MSetPostpone(MTools * Tools) : MState(Tools) 
    {
        Tools->postpone = Tools->readNvsInt("qulon", "postp",  0 );
        Oled->showLine4Text(" Отложить на ");
        Oled->showLine3Delay( Tools->postpone );
        Tools->showUpDn();                      // " UP/DN, В-выбор "
    }
    MState * MSetPostpone::fsm()
    {
        switch ( Keyboard->getKey() )
        {
        case MKeyboard::C_LONG_CLICK :
            return new MStop(Tools);
        case MKeyboard::P_CLICK :
            return new MSetCurrentOffset(Tools);
        case MKeyboard::UP_CLICK :
            Tools->incPostpone( 1 );
            break;
        case MKeyboard::DN_CLICK :
            Tools->decPostpone( 1 );
            break;
        case MKeyboard::B_CLICK :
            Tools->saveInt( "qulon", "postp", Tools->postpone );            // Выбор заносится в энергонезависимую память
            #ifdef DEBUG_OPTIONS
                Serial.println(Tools->postpone);
            #endif
            return new MSetCurrentOffset(Tools);
        default :;
        }
        return this;
    };

    MSetCurrentOffset::MSetCurrentOffset( MTools * Tools ) : MState(Tools) 
    {
        Oled->showLine4Text("    корр I   ");       //Oled->showLine4RealVoltage();
        Oled->showLine3RealCurrent();
        Tools->showUpDn(); // " UP/DN, В-выбор "
    }
    MState * MSetCurrentOffset::fsm()
    {
        switch ( Keyboard->getKey() )
        {
        case MKeyboard::C_LONG_CLICK :
            return new MStop(Tools);
        case MKeyboard::P_CLICK :
            return new MSetVoltageOffset(Tools);
        case MKeyboard::UP_CLICK :
            Tools->incCurrentOffset( 0.01f, false );
            break;
        case MKeyboard::DN_CLICK :
            Tools->decCurrentOffset( 0.01f, false );
            break;
        case MKeyboard::B_CLICK :
            Tools->saveInt( "qulon", "cOffset", Board->currentOffset );
            #ifdef DEBUG_OPTIONS
                Serial.println(Board->currentOffset);
            #endif
            return new MSetVoltageOffset(Tools);
        default :;
        }
        return this;
    };

    MSetVoltageOffset::MSetVoltageOffset( MTools * Tools ) : MState(Tools)
    {
        Oled->showLine4RealVoltage();
        Oled->showLine3Text("  V offset   ");   
        //  Oled->showLine3RealVoltage(); // в цветной!
        Tools->showUpDn();
    }
    MState * MSetVoltageOffset::fsm()
    {
        switch( Keyboard->getKey() )
        {
        case MKeyboard::C_LONG_CLICK :
            return new MStop(Tools);
        case MKeyboard::P_CLICK :
            return new MStop(Tools);
        case MKeyboard::UP_CLICK :
            Tools->incVoltageOffset( 0.01f, false );
            break;
        case MKeyboard::DN_CLICK :
            Tools->decVoltageOffset( 0.01f, false );
            break;
        case MKeyboard::B_CLICK :
            Tools->saveInt( "qulon", "vOffset", Board->voltageOffset );
            #ifdef DEBUG_OPTIONS
                Serial.println(Board->voltageOffset);
            #endif
            return new MStop(Tools);
        default :;
        }
        return this;
    };

    // MState * MSetFactory::fsm()
    // {
    //     if ( Keyboard->getKey(MKeyboard::C_CLICK)) { return new MStop(Tools); }   // Отказ

    //     // Удаление всех старых ключей
    //     if( Keyboard->getKey(MKeyboard::B_CLICK)) {
    //         Tools->clearAllKeys ("qulon");
    //         // Tools->clearAllKeys ("s-power");
    //         // Tools->clearAllKeys ("e-power"); 
    //         // Tools->clearAllKeys ("cccv");
    //         // Tools->clearAllKeys ("recovery");
    //         // Tools->clearAllKeys ("e-charge");
    //         // Tools->clearAllKeys ("storage");
    //         // Tools->clearAllKeys ("service");
    //         // Tools->clearAllKeys ("option");
    //         return new MStop(Tools);
    //     }
    //     return this;
    // };


    MStop::MStop(MTools * Tools) : MState(Tools)
    {

    }      

    MState * MStop::fsm()
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

