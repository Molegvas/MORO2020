/*
 * dcsupplyfsm.cpp
 * Конечный автомат источника постоянного тока
 * В прототипе - Простой источник питания
 * 2019.05.10 2019.12.23
 */

#include "modes/dcsupplyfsm.h"
#include "mtools.h"
#include "board/mboard.h"
#include "measure/mkeyboard.h"
#include "display/moled.h"
#include <Arduino.h>

namespace DcSupplyFsm
{
    MStart::MStart(MTools * Tools) : MState(Tools) 
    {

//Tools->clearAllKeys ("s-power"); // Удаление всех старых ключей

        Tools->setVoltageMax( Tools->readNvsFloat("s-power", "voltMax", Tools->pows[4][0]) );   // Начальный выбор 12.0 вольт
        Tools->setCurrentMax( Tools->readNvsFloat("s-power", "currMax", Tools->pows[4][1]) );   //                  3.0 А
        Tools->powO   = Tools->readNvsFloat("s-power", "powO", 0.0f);                // Планируется для быстрого входа в режим, nu
        
        #ifdef DEBUG_SUPPLY
            Serial.print("Voltage     : "); Serial.println(Tools->getVoltageMax(), 2);         // формат XX.XX
            Serial.print("Max Current : "); Serial.println(Tools->getCurrentMax(), 2);
        #endif

            // Подготовка индикации
        Oled->showLine4Text("   Источник  ");
        Oled->showLine3Power( Tools->getVoltageMax(), Tools->getCurrentMax() ); // example: " 12.3В  3.3А "
        Oled->showLine2Text(" P-корр.С-старт ");        // Активны две кнопки: P-сменить настройки, и C-старт
        Oled->showLine1Time(0);                         // уточнить
        Oled->showLine1Ah(0.0);                         // уточнить
        #ifdef V22
            Board->ledsOn();                                // Светодиод светится белым до старта - режим выбран
        #endif
        #ifdef DEBUG_SUPPLY
            Serial.println("DcSupply: Start");
        #endif
    }
    MState * MStart::fsm()
    {
        switch ( Keyboard->getKey() )    //Здесь так можно
        {
            case MKeyboard::C_CLICK :
                // Старт без уточнения параметров (быстрый старт), 
                // максимальный ток и напряжение - в соответствии с выбором (см таблицу в/А)
                // Восстановленные на предыдущем шаге данные заменяются (кроме индекса)
                Tools->setVoltageMax( Tools->pows[Tools->powInd][0] );
                Tools->setCurrentMax( Tools->pows[Tools->powInd][1] );

                #ifdef DEBUG_SUPPLY
                    Serial.println("DcSupply: activated");
                #endif
                return new MExecution(Tools);

            case MKeyboard::P_CLICK :
                return new MSelectUI(Tools);
            default:;
        }
        return this;
    };


    MSelectUI::MSelectUI(MTools * Tools) : MState(Tools) 
    {
        Oled->showLine4Text("  Выбор U&I  ");
                                                    //  line3 - без изменения
        //Oled->showLine2Text(" UP/DN, В-выбор ");    // Подсказка: UP/DN - листать список, В - выбрать
        Tools->showUpDn();                          // Подсказка: UP/DN - листать список, В - выбрать"

        #ifdef DEBUG_SUPPLY
        Serial.println("DcSupply: Select U&I");
        #endif

    }   
    MState * MSelectUI::fsm() 
    {
    switch ( Keyboard->getKey() )
        {
            case MKeyboard::C_CLICK :
                return new MExecution(Tools);
            case MKeyboard::B_CLICK :           // Завершить выбор UI
                Tools->savePowInd("qulon");
                return new MSetCurrentMax(Tools);
            case MKeyboard::UP_CLICK :
                Tools->upPow();                 // Перейти по списку вверх
                break;
            case MKeyboard::DN_CLICK :          // Перейти по списку вниз
                Tools->dnPow();
                break;
            default:;
        }
        Oled->showLine3Power( Tools->getVoltageMax(), Tools->getCurrentMax() );
        return this;
    };

    MSetCurrentMax::MSetCurrentMax(MTools * Tools) : MState(Tools)
    {
        Oled->showLine4Text(" Ток не более");
        Oled->showLine3MaxI( Tools->getCurrentMax() );
        Tools->showUpDn();                          // " UP/DN, В-выбор "
    } 
    MState * MSetCurrentMax::fsm()
    {
    switch ( Keyboard->getKey() )
        {
            case MKeyboard::C_CLICK :
                return new MExecution(Tools);
            case MKeyboard::B_CLICK :
                Tools->saveFloat( "s-power", "currMax", Tools->getCurrentMax() ); 
                return new MSetVoltage(Tools);
            case MKeyboard::UP_CLICK :
                Tools->incCurrentMax( 0.1f, false );
                break;
            case MKeyboard::DN_CLICK:
                Tools->decCurrentMax( 0.1f, false );
                break;
            case MKeyboard::UP_LONG_CLICK:
                Tools->incCurrentMax( 0.5f, false );
                break;
            case MKeyboard::DN_LONG_CLICK:
                Tools->decCurrentMax( 0.5f, false );
                break;
            default:;
        }
        Oled->showLine3MaxI( Tools->getCurrentMax() );
        return this;
    };

    MSetVoltage::MSetVoltage(MTools * Tools) : MState(Tools)
    {
        Oled->showLine4Text("  Напряжение ");
        Oled->showLine3MaxU( Tools->getVoltageMax() );
        Tools->showUpDn();                          // " UP/DN, В-выбор "
    }     
    MState * MSetVoltage::fsm()
    {
        switch ( Keyboard->getKey() )
        {
            case MKeyboard::C_CLICK :
                return new MExecution(Tools);
            case MKeyboard::B_CLICK :
                Tools->saveFloat( "s-power", "voltMax", Tools->getVoltageMax() ); 
                return new MExecution(Tools);
            case MKeyboard::UP_CLICK :
                Tools->incVoltageMax( 0.1f, false );
                break;
            case MKeyboard::DN_CLICK:
                Tools->decVoltageMax( 0.1f, false );
                break;
            case MKeyboard::UP_LONG_CLICK:
                Tools->incVoltageMax( 1.0f, false );
                break;
            case MKeyboard::DN_LONG_CLICK:
                Tools->decVoltageMax( 1.0f, false );
                break;
            default:;
        }
        Oled->showLine3MaxU( Tools->getVoltageMax() );
        return this;
    };

    MExecution::MExecution(MTools * Tools) : MState(Tools)
    {
        Tools->clrAhCharge();                                       // Обнуляются счетчики времени и отданного заряда

        // Задаются начальные напряжение и ток
        Board->setVoltageVolt( Tools->getVoltageMax() );            // Voltage limit
        Board->setCurrentAmp( Tools->getCurrentMax() * MDcConst::outputMaxFactor );     // 1.05f );     // 
        Board->powOn();   Board->swOn();                            // Включение преобразователя и коммутатора.
        #ifdef V22
            Board->ledsOff(); Board->ledGOn();                          // Зеленый светодиод - процесс заряда запущен
        #endif
        // Задаются отображения на экране дисплея построчно (4-я строка - верхняя)
        Oled->showLine4RealVoltage();
        Oled->showLine3RealCurrent();
        Oled->showLine2Text("   Запитано...  ");                    // 
        Oled->showLine1Time( Tools->getChargeTimeCounter() );
        Oled->showLine1Ah( Tools->getAhCharge() );

        Tools->setSetPoint( Tools->getVoltageMax() );

        #ifdef DEBUG_SUPPLY
            Serial.println(" Источник питания стартовал с параметрами:");
            Serial.print("  Voltage max, B  *** = ");  Serial.println( Tools->getVoltageMax() );
            Serial.print("  Current max, A  *** = ");  Serial.println( Tools->getCurrentMax() );
        #endif

        // Настройка ПИД-регулятора
        Tools->initPid( MDcConst::outputMin,
                        MDcConst::outputMaxFactor * Tools->getCurrentMax(),
                        MDcConst::k_p,
                        MDcConst::k_i,
                        MDcConst::k_d,
                        MDcConst::bangMin,
                        MDcConst::bangMax,
                        MDcConst::timeStep );
        Tools->setIntegral( MDcConst::integralVoltFactor * Tools->getVoltageMax() );  // Напряжение при пуске будет
                                                                // выставлено с погрешностью задающей напряжение цепи
    }     
    MState * MExecution::fsm()
    {
        Tools->chargeCalculations();
        // //Регулировка напряжения "на лету"
        // if(Keyboard->getKey(MKeyboard::UP_CLICK)) { Tools->incVoltagePow( 0.1f, false ); Tools->liveU(); }  // Добавить 100 мB
        // if(Keyboard->getKey(MKeyboard::DN_CLICK)) { Tools->decVoltagePow( 0.1f, false ); Tools->liveU(); }  // Убавить  100 мB
        // if(Keyboard->getKey(MKeyboard::UP_AUTO_CLICK)) { Tools->incVoltagePow( 0.1f, false ); Tools->liveU(); }  // Добавить по 100 мB
        // if(Keyboard->getKey(MKeyboard::DN_AUTO_CLICK)) { Tools->decVoltagePow( 0.1f, false ); Tools->liveU(); }  // Убавить по 100 мB

        // if(Keyboard->getKey(MKeyboard::B_CLICK))
        // { 
        //     Tools->saveFloat( "s-power", "voltMax", Tools->getVoltageMax() ); 
        //     Oled->showLine3RealCurrent();
        // }   

        // if(Keyboard->getKey(MKeyboard::C_CLICK)) { return new MExit(Tools); } 

        switch ( Keyboard->getKey() )
        {
            case MKeyboard::C_CLICK :
                return new MExit(Tools);

            case MKeyboard::B_CLICK :
                Tools->saveFloat( "s-power", "voltMax", Tools->getVoltageMax() );
                Oled->showLine3RealCurrent();
                break;

            case MKeyboard::UP_CLICK :
                Tools->incVoltageMax( 0.1f, false );
                Oled->showLine3MaxU( Tools->getVoltageMax() );
                Tools->liveU();
                break;  //return new MExecution(Tools);

            case MKeyboard::DN_CLICK:
                Tools->decVoltageMax( 0.1f, false );
                Tools->liveU();   
                Oled->showLine3MaxU( Tools->getVoltageMax() );
                break;  //                return new MExecution(Tools);

            case MKeyboard::UP_AUTO_CLICK:
                Tools->incVoltageMax( 0.1f, false );
                Tools->liveU();   
                Oled->showLine3MaxU( Tools->getVoltageMax() );
                break;  //                return new MExecution(Tools);

            case MKeyboard::DN_AUTO_CLICK:
                Tools->decVoltageMax( 0.1f, false );
                Tools->liveU();   
                Oled->showLine3MaxU( Tools->getVoltageMax() );
                break;  //                return new MExecution(Tools);
            default:;
        }


        Tools->runPidVoltage();

        return this;
    };

    // Процесс выхода из режима - до нажатия кнопки "С" удерживается индикация
    // о продолжительности и отданном заряде.
    MExit::MExit(MTools * Tools) : MState(Tools)
    {
        Tools->powShutdown();
    }      
    MState * MExit::fsm()
    {
        if(Keyboard->getKey(MKeyboard::C_CLICK)) 
        {
            Tools->activateExit("  DC источник   ");
            #ifdef DEBUG_SUPPLY
                Serial.println("DcSupply: Exit"); 
                Serial.print("\t charge = ");   Serial.println( Tools->getAhCharge() );
            #endif  
            return 0;   // Возврат к выбору режима
        }
        return this;
    };
};
