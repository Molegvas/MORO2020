/*
    devicefsm.cpp
    Конечный автомат заводских регулировок.
    27.01.2020 
*/

#include "modes/devicefsm.h"
#include "mtools.h"
#include "board/mboard.h"
#include "measure/mkeyboard.h"
#include "display/moled.h"
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

        voltagePwmMin       = Tools->readNvsInt("dc", "v_pwm_l",    MDcConsts::vPwm_l);
        voltagePwmMax       = Tools->readNvsInt("dc", "v_pwm_h",    MDcConsts::vPwm_h);
        voltagePwm          = Tools->readNvsInt("dc", "v_pwm",      MDcConsts::vPwm_h /  2);
        voltageDeltaPwm     = Tools->readNvsInt("dc", "vd_pwm",     MDcConsts::vPwm_h / 16);

        currentPwm          = Tools->readNvsInt("dc", "i_pwm",  MDcConsts::iPwm_h /  2);
        currentDeltaPwm     = Tools->readNvsInt("dc", "id_pwm", MDcConsts::iPwm_h / 16);
        dischargePwm        = Tools->readNvsInt("dc", "r_pwm",  MDcConsts::rPwm_h /  2);     // r - реверс тока
        dischargeDeltaPwm   = Tools->readNvsInt("dc", "rd_pwm", MDcConsts::rPwm_h / 16);

        #ifdef DEBUG_DEVICE
            Serial.print("voltagePwmMin= ");        Serial.println( voltagePwmMin );
            Serial.print("voltagePwmMax= ");        Serial.println( voltagePwmMax );
            Serial.print("voltagePwm= ");           Serial.println( voltagePwm );
            Serial.print("voltageDeltaPwm= ");      Serial.println( voltageDeltaPwm );
            
            Serial.print("currentPwm= ");           Serial.println( currentPwm );
            Serial.print("currentDeltaPwm= ");      Serial.println( currentDeltaPwm );
            Serial.print("dischargePwm= ");         Serial.println( dischargePwm );
            Serial.print("dischargeDeltaPwm= ");    Serial.println( dischargeDeltaPwm );
        #endif

        // Индикация
        #ifdef OLED_1_3
            Oled->showLine4Text(" Set Voltage ");
            // Oled->showLine3Akb( Tools->getVoltageNom(), Tools->getCapacity() );              // example: "  12В  55Ач  "
            Oled->showLine3Text("             ");
            Oled->showLine2Text(" P-след,B-выбор ");        // Подсказка: активны две кнопки: P-следующий, и B-выбор
            // Oled->showLine1Time(0);                         // уточнить
            // Oled->showLine1Ah(0.0);                         // уточнить
            
        #endif

        #ifdef V22
            Board->ledsOn();                                // Светодиод светится белым до старта заряда - режим выбран
        #endif

        #ifdef TFT_1_44
            // no leds
        #endif
        #ifdef TFT_1_8
            // no leds
        #endif
    }
    MState * MStart::fsm()
    {
        switch ( Keyboard->getKey() )    //Здесь так можно
        {
            case MKeyboard::C_LONG_CLICK :
                return new MStop(Tools);                    // Отказ от продолжения регулировок

            case MKeyboard::P_CLICK :
                // Продолжение выбора объекта настройки
                return new MSetVoltagePwmMin(Tools);
            break;
            case MKeyboard::B_CLICK :
            //     return new MSetVoltage(Tools);      // Выбрана регулировка напряжения.
            break;
            default:
            break;
        }

        return this;
    };

    // Коррекция pwm для минимального напряжения.
    MSetVoltagePwmMin::MSetVoltagePwmMin(MTools * Tools) : MState(Tools)
    {
        // Индикация
        Oled->showLine4Text("   Vpwm min  ");
        Oled->showLine3Num( voltagePwm );
        Tools->showUpDn(); // " UP/DN, В-выбор "
    }
    MState * MSetVoltagePwmMin::fsm()
    {
        switch ( Keyboard->getKey() )
        {
            case MKeyboard::C_LONG_CLICK :                  // Отказ от продолжения ввода параметров - стоп
                return new MStop(Tools);
            case MKeyboard::P_CLICK :                       // Продолжение выбора объекта настройки
                //return new MSetVoltageMax(Tools);
    break;
            // case MKeyboard::B_CLICK :                       // Сохранить и перейти к следующему параметру
            //     Tools->saveInt( "dc", "v_pwm_l", Tools->getVoltageMax() ); 
            //     return new MSetCurrentMin(Tools);

            // case MKeyboard::UP_CLICK :
            //     Tools->incVoltageMax( 0.1f, false );        // По кольцу? - Нет
            //     break;
            // case MKeyboard::DN_CLICK:
            //     Tools->decVoltageMax( 0.1f, false );
            //     break;
            // case MKeyboard::UP_AUTO_CLICK:
            //     Tools->incVoltageMax( 0.1f, false );
            //     break;
            // case MKeyboard::DN_AUTO_CLICK:
            //     Tools->decVoltageMax( 0.1f, false );
            //     break;
            default:;
        }
        #ifdef OLED_1_3
            //Oled->showLine3MaxU( Tools->getVoltageMax() );
        #endif

        return this;
    };









    // Состояние " "
    MChargeCurrent::MChargeCurrent(MTools * Tools) : MState(Tools)
    {
        // Индикация
        #ifdef OLED_1_3
            Oled->showLine4Text(" Set Current ");
            // Oled->showLine3Akb( Tools->getVoltageNom(), Tools->getCapacity() );              // example: "  12В  55Ач  "
            Oled->showLine3Text("             ");
            Oled->showLine2Text(" P-след,B-выбор ");        // Подсказка: активны две кнопки: P-следующий, и B-выбор
            // Oled->showLine1Time(0);                         // уточнить
            // Oled->showLine1Ah(0.0);                         // уточнить
            
        #endif


    }
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
