/*
    devicefsm.cpp
    Конечный автомат заводских регулировок.
    Параметры DC источника и цепи разряда восстанавливаются из энергонезависимой памяти, 
    занесенные в нее при предыдущих включениях.
    При первом включении заносятся параметры, заданные разработчиком.
    27.01.2020 
*/

#include "modes/devicefsm.h"
#include "mtools.h"
#include "board/mboard.h"
#include "measure/mkeyboard.h"
#include "display/moled.h"
#include "board/moverseer.h"
#include <Arduino.h>

namespace DeviceFsm
{
    // Переменные регулирования DC источника
    int voltagePwmMin       =    0;     // Пользоваьельский предел снизу                   
    int voltagePwmMax       = 1024;     //                  
    int voltagePwm          =  512;     // Текущая установка напряжения
    int voltageDeltaPwm     =   64;     // Шаг регулирования

    int currentPwm                         =  512;     // 
    int currentDeltaPwm                    =   64;     // 
    int dischargePwm                       =  512;     //
    int dischargeDeltaPwm                  =   64;     //



    // Состояние "Старт", инициализация выбранного режима работы (Заводские регулировки).
    MStart::MStart(MTools * Tools) : MState(Tools)
    {
        // Параметры DC источника и цепи разряда восстанавливаются из энергонезависимой памяти, 
        // занесенные в нее при предыдущих включениях.
        // При первом включении заносятся параметры, заданные разработчиком.


        // currentDeltaPwm     = Tools->readNvsInt("dc", "id_pwm", MDcConsts::iPwm_h / 16);
        // dischargeDeltaPwm   = Tools->readNvsInt("dc", "rd_pwm", MDcConsts::rPwm_h / 16);

        // #ifdef DEBUG_DEVICE
        //     Serial.print("voltagePwmMin= ");        Serial.println( voltagePwmMin );
        //     Serial.print("voltagePwmMax= ");        Serial.println( voltagePwmMax );
        //     Serial.print("voltagePwm= ");           Serial.println( voltagePwm );
        //     Serial.print("voltageDeltaPwm= ");      Serial.println( voltageDeltaPwm );

        //     // Serial.print("currentPwm= ");           Serial.println( currentPwm );
        //     // Serial.print("currentDeltaPwm= ");      Serial.println( currentDeltaPwm );
        //     // Serial.print("dischargePwm= ");         Serial.println( dischargePwm );
        //     // Serial.print("dischargeDeltaPwm= ");    Serial.println( dischargeDeltaPwm );
        // #endif

        // Индикация
        #ifdef OLED_1_3
            Oled->showLine4Text(" PWM Voltage ");
            Oled->showLine3Text(" C_LONG-CLEAR");
            Oled->showLine2Text(" P-след,B-выбор ");        // Подсказка: активны две кнопки: P-следующий, и B-выбор
            Oled->showLine1Heap( ESP.getFreeHeap() );
            Oled->showLine1Celsius( Board->Overseer->getCelsius() );
        #endif
    }
    MState * MStart::fsm()
    {
        switch ( Keyboard->getKey() )    //Здесь так можно
        {
            case MKeyboard::C_LONG_CLICK :
                Tools->clearAllKeys ("dc");                 // Удаление всех старых ключей
//                return new MStop(Tools);                    // Отказ от продолжения регулировок
                break;
            case MKeyboard::P_CLICK :
                // Продолжение выбора объекта настройки
                return new MSetVoltagePwmMin(Tools);
    //        break;
            case MKeyboard::B_CLICK :
                voltagePwmMin       = Tools->readNvsInt("dc", "v_pwm_l",    MDcConsts::vPwm_l);
                voltagePwmMax       = Tools->readNvsInt("dc", "v_pwm_h",    MDcConsts::vPwm_h);
                voltageDeltaPwm     = Tools->readNvsInt("dc", "vd_pwm",     MDcConsts::vPwm_h / 16);
                return new MSetVoltagePwmDelta(Tools);      // Выбрана регулировка voltageDeltaPwm
            break;
            default:;
        }
        return this;
    };

    // Коррекция шага приращения pwm для регулировки напряжения.
    MSetVoltagePwmDelta::MSetVoltagePwmDelta(MTools * Tools) : MState(Tools)
    {
        // Индикация
        Oled->showLine4Text("  Vpwm delta ");
        Oled->showLine3Num( voltageDeltaPwm );
        Tools->showUpDn(); // " UP/DN, В-выбор "

        Board->powOff();     Board->swOff();          // Выключение преобразователя и коммутатора.
    }
    MState * MSetVoltagePwmDelta::fsm()
    {
        switch ( Keyboard->getKey() )
        {
            case MKeyboard::C_LONG_CLICK :                  // Отказ от продолжения ввода параметров - стоп
                return new MStop(Tools);
            case MKeyboard::C_CLICK :
                return new MVoltagePwmExe(Tools);
            case MKeyboard::P_CLICK :                       // Продолжение выбора объекта настройки
                return new MSetVoltagePwmMin(Tools);
            case MKeyboard::B_CLICK :                       // Сохранить и перейти к следующему параметру
                Tools->saveInt( "dc", "vd_pwm", voltageDeltaPwm ); 
                return new MSetVoltagePwmMin(Tools);
            case MKeyboard::UP_CLICK :
                voltageDeltaPwm = Tools->incNum( voltageDeltaPwm, MDcConsts::vPwm_h, 1 );
                break;
            case MKeyboard::DN_CLICK :
                voltageDeltaPwm = Tools->decNum( voltageDeltaPwm, MDcConsts::vPwm_l, 1 );
                break;
            case MKeyboard::UP_AUTO_CLICK:
                voltageDeltaPwm = Tools->incNum( voltageDeltaPwm, MDcConsts::vPwm_h, 8 );
                break;
            case MKeyboard::DN_AUTO_CLICK :
                voltageDeltaPwm = Tools->decNum( voltageDeltaPwm, MDcConsts::vPwm_l, 8 );
                break;
            default:;
        }
        #ifdef OLED_1_3
            Oled->showLine3Num( voltageDeltaPwm );
        #endif

        return this;
    };

    // Коррекция pwm для минимального напряжения.
    MSetVoltagePwmMin::MSetVoltagePwmMin(MTools * Tools) : MState(Tools)
    {
        // Индикация
        Oled->showLine4Text("   Vpwm min  ");
        Oled->showLine3Num( voltagePwmMin );
        Tools->showUpDn(); // " UP/DN, В-выбор "
    }
    MState * MSetVoltagePwmMin::fsm()
    {
        switch ( Keyboard->getKey() )
        {
            case MKeyboard::C_LONG_CLICK :                  // Отказ от продолжения ввода параметров - стоп
                return new MStop(Tools);
            case MKeyboard::C_CLICK :
                return new MVoltagePwmExe(Tools);
            case MKeyboard::P_CLICK :                       // Продолжение выбора объекта настройки
                return new MSetVoltagePwmMax(Tools);
            case MKeyboard::B_CLICK :                       // Сохранить и перейти к следующему параметру
                Tools->saveInt( "dc", "v_pwm_l", voltagePwmMin ); 
                return new MSetVoltagePwmMax(Tools);
            case MKeyboard::UP_CLICK :
                voltagePwmMin = Tools->incNum( voltagePwmMin, MDcConsts::vPwm_h, 1 );
                break;
            case MKeyboard::DN_CLICK :
                voltagePwmMin = Tools->decNum( voltagePwmMin, MDcConsts::vPwm_l, 1 );
                break;
            case MKeyboard::UP_AUTO_CLICK:
                voltagePwmMin = Tools->incNum( voltagePwmMin, MDcConsts::vPwm_h, voltageDeltaPwm );
                break;
            case MKeyboard::DN_AUTO_CLICK :
                voltagePwmMin = Tools->decNum( voltagePwmMin, MDcConsts::vPwm_l, voltageDeltaPwm );
                break;
            default:;
        }
        #ifdef OLED_1_3
            Oled->showLine3Num( voltagePwmMin );
        #endif

        return this;
    };

    // Коррекция pwm для максимального напряжения.
    MSetVoltagePwmMax::MSetVoltagePwmMax(MTools * Tools) : MState(Tools)
    {
        // Индикация
        Oled->showLine4Text("   Vpwm max  ");
        Oled->showLine3Num( voltagePwmMax );
        Tools->showUpDn(); // " UP/DN, В-выбор "
    }
    MState * MSetVoltagePwmMax::fsm()
    {
        switch ( Keyboard->getKey() )
        {
            case MKeyboard::C_LONG_CLICK :                  // Отказ от продолжения ввода параметров - стоп
                return new MStop(Tools);
            case MKeyboard::C_CLICK :
                return new MVoltagePwmExe(Tools);
            case MKeyboard::P_CLICK :                       // Продолжение выбора объекта настройки
                //return new MSetCurrentPwmDelta(Tools);
    break;
            case MKeyboard::B_CLICK :                       // Сохранить и перейти к следующему параметру
                Tools->saveInt( "dc", "v_pwm_h", voltagePwmMax ); 
                return new MVoltagePwmExe(Tools);           // Выполнять
            case MKeyboard::UP_CLICK :
                voltagePwmMax = Tools->incNum( voltagePwmMax, MDcConsts::vPwm_h, 1 );
                break;
            case MKeyboard::DN_CLICK :
                voltagePwmMax = Tools->decNum( voltagePwmMax, MDcConsts::vPwm_l, 1 );
                break;
            case MKeyboard::UP_AUTO_CLICK:
                voltagePwmMax = Tools->incNum( voltagePwmMax, MDcConsts::vPwm_h, voltageDeltaPwm );
                break;
            case MKeyboard::DN_AUTO_CLICK :
                voltagePwmMax = Tools->decNum( voltagePwmMax, MDcConsts::vPwm_l, voltageDeltaPwm );
                break;
            default:;
        }
        #ifdef OLED_1_3
            Oled->showLine3Num( voltagePwmMax );
        #endif

        return this;
    };

    // Регулировка DC источника по напряжению.
    MVoltagePwmExe::MVoltagePwmExe(MTools * Tools) : MState(Tools)
    {
        // Индикация
        Oled->showLine4RealVoltage();       //  4Text("   Vpwm exe  ");
        Oled->showLine3Num( voltagePwm );
        Tools->showUpDn(); // " UP/DN,В-повтор "
        Board->powOn();     Board->swOn();          // Включение преобразователя и коммутатора.
        Board->setCurrentPwm( 100 );                // Иначе не даст напряжения

        voltagePwm = voltagePwmMin;
        Board->setVoltagePwm( voltagePwm );
    }
    MState * MVoltagePwmExe::fsm()
    {
        switch ( Keyboard->getKey() )
        {
            case MKeyboard::C_LONG_CLICK :                  // Отказ от продолжения ввода параметров - стоп
                return new MStop(Tools);
            case MKeyboard::P_CLICK :                       // Продолжение выбора объекта настройки
                //return new MSetCurrentPwmDelta(Tools);
    break;
            case MKeyboard::B_CLICK :                       // Повторить регулировку по напряжению
                return new MSetVoltagePwmDelta(Tools);
            case MKeyboard::UP_CLICK :
            case MKeyboard::C_CLICK :
                voltagePwm = Tools->incNum( voltagePwm, voltagePwmMax, voltageDeltaPwm );
                Board->setVoltagePwm( voltagePwm );
                break;
            case MKeyboard::DN_CLICK :
                voltagePwm = Tools->decNum( voltagePwm, voltagePwmMin, voltageDeltaPwm );
                Board->setVoltagePwm( voltagePwm );
                break;
            default:;
        }
        #ifdef OLED_1_3
            Oled->showLine4RealVoltage();
            Oled->showLine3Num( voltagePwm );
        #endif

        return this;
    };








    // // Состояние " "
    // MChargeCurrent::MChargeCurrent(MTools * Tools) : MState(Tools)
    // {
    //     // Индикация
    //     #ifdef OLED_1_3
    //         Oled->showLine4Text(" Set Current ");
    //         // Oled->showLine3Akb( Tools->getVoltageNom(), Tools->getCapacity() );              // example: "  12В  55Ач  "
    //         Oled->showLine3Text("             ");
    //         Oled->showLine2Text(" P-след,B-выбор ");        // Подсказка: активны две кнопки: P-следующий, и B-выбор
    //         // Oled->showLine1Time(0);                         // уточнить
    //         // Oled->showLine1Ah(0.0);                         // уточнить
            
    //     #endif


    // }


    MState * MChargeCurrent::fsm()
    {
        switch ( Keyboard->getKey() )    //Здесь так можно
        {
            case MKeyboard::C_LONG_CLICK :
                return new MStop(Tools);                    // Отказ от продолжения регулировок

            case MKeyboard::P_CLICK :
                // Продолжение выбора объекта настройки
      return new MStart(Tools);
            break;
            case MKeyboard::B_CLICK :
            //     return new MSetFactory(Tools);      // Выбрано уточнение настроек заряда.
            break;
            default:
            break;
        }

        return this;
    };











    // Процесс выхода из режима заряда - до нажатия кнопки "С" удерживается индикация о продолжительности и отданном заряде.
    MStop::MStop(MTools * Tools) : MState(Tools)
    {
        Tools->shutdownCharge();                        // Включение красного светодиода здесь
    }    
    MState * MStop::fsm()
    {
        if(Keyboard->getKey(MKeyboard::C_CLICK)) 
        { 
            Tools->activateExit("  Регулировки   "); 
            return nullptr;                             // Возврат к выбору режима
        }
        return this;
    };
};
