/*
    cccvfsm.cpp
    Конечный автомат заводских регулировок.
    27.01.2020 
*/

#include "modes/productfsm.h"
#include "mtools.h"
#include "board/mboard.h"
#include "measure/mkeyboard.h"
#include "display/moled.h"
#include <Arduino.h>

namespace ProductFsm
{
    // Состояние "Старт", инициализация выбранного режима работы (Заводские регулировки).
    MStart::MStart(MTools * Tools) : MState(Tools)
    {
        // Параметры заряда из энергонезависимой памяти, Занесенные в нее при предыдущих включениях, как и
        // выбранные ранее номинальные параметры батареи (напряжение, емкость).
        // При первом включении, как правило заводском, номиналы батареи задаются в mdispather.h. 
        // Tools->setVoltageMax( Tools->readNvsFloat("cccv", "voltMax", MChConsts::voltageMaxFactor * Tools->getVoltageNom()) );
        // Tools->setVoltageMin( Tools->readNvsFloat("cccv", "voltMin", MChConsts::voltageMinFactor * Tools->getVoltageNom()) );
        // Tools->setCurrentMax( Tools->readNvsFloat("cccv", "currMax", MChConsts::currentMaxFactor * Tools->getCapacity()) );
        // Tools->setCurrentMin( Tools->readNvsFloat("cccv", "currMin", MChConsts::currentMinFactor * Tools->getCapacity()) );

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
                return new MChargeCurrent(Tools);
            break;
            case MKeyboard::B_CLICK :
            //     return new MSetVoltage(Tools);      // Выбрана регулировка напряжения.
            break;
            default:
            break;
        }

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
