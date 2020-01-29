/*
    Файл: devicefsm.cpp
    Конечный автомат заводских регулировок - арсенал разработчика (ручной режим).
    Получение данных для аппроксимации функций одной переменной (линеаризация характеристик
    силового DC-источника).
    Используется online калькулятор https://planetcalc.ru/5992/ .
    Параметры регулировки DC источника и цепи разряда восстанавливаются из энергонезависимой памяти, 
    занесенные в нее при предыдущих включениях.
    При первом включении заносятся параметры, заданные разработчиком.
    Вычисленные коэффициенты преобразований заносятся в соответствующие функции board.cpp 
    Тепловой режим - на ответственности оператора.
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
    int voltagePwmMin       =    0;     // Пользовательский предел снизу                   
    int voltagePwmMax       = 1024;     //                  
    int voltagePwm          =  512;     // Текущая установка напряжения
    int voltageDeltaPwm     =   64;     // Шаг регулирования

    int currentPwmMin       =    0;
    int currentPwmMax       = 1024;
    int currentPwm          =  512;     // 
    int currentDeltaPwm     =   64;     // 

    int dischargePwmMin     =    0;
    int dischargePwmMax     = 1024;
    int dischargePwm        =  512;     //
    int dischargeDeltaPwm   =   64;     //

    // Состояние "Старт", инициализация выбранного режима работы (Заводские регулировки).
    MStart::MStart(MTools * Tools) : MState(Tools)
    {
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
                Tools->clearAllKeys ("dc");     // Удаление всех старых ключей (очистка)
                break;
            case MKeyboard::P_CLICK :
                // Продолжение выбора объекта настройки
                return new MSetVoltagePwmMin(Tools);
            case MKeyboard::B_CLICK :
                // Восстановление из энергонезависимой памяти, при первом включении
                // или очистки - табличные.
                voltagePwmMin   = Tools->readNvsInt("dc", "v_pwm_l", MDcConsts::vPwm_l);
                voltagePwmMax   = Tools->readNvsInt("dc", "v_pwm_h", MDcConsts::vPwm_h);
                voltageDeltaPwm = Tools->readNvsInt("dc", "vd_pwm",  MDcConsts::vPwm_h / 16);
                return new MSetVoltagePwmDelta(Tools);      // Выбрана регулировка voltageDeltaPwm
            break;
            default :;
        }
        return this;
    };

    // Коррекция шага приращения pwm для регулировки напряжения.
    MSetVoltagePwmDelta::MSetVoltagePwmDelta(MTools * Tools) : MState(Tools)
    {
        // Индикация
        Oled->showLine4Text("  Vpwm delta ");
        Oled->showLine3Num( voltageDeltaPwm );
        Tools->showUpDn();                          // Подсказка " UP/DN, В-выбор "

        // Выключение преобразователя и коммутатора, обнуление задающих величин PWM.
        Tools->shutdownDC();
    }
    MState * MSetVoltagePwmDelta::fsm()
    {
        switch ( Keyboard->getKey() )
        {
            case MKeyboard::C_LONG_CLICK :              // Отказ от продолжения ввода - стоп и выход
                return new MStop(Tools);
            case MKeyboard::C_CLICK :
                return new MVoltagePwmExe(Tools);
            case MKeyboard::P_CLICK :                   // Продолжение выбора объекта настройки
                return new MSetVoltagePwmMin(Tools);
            case MKeyboard::UP_CLICK :
                voltageDeltaPwm = Tools->incNum( voltageDeltaPwm, MDcConsts::vPwm_h, 1 );
                break;
            case MKeyboard::DN_CLICK :
                voltageDeltaPwm = Tools->decNum( voltageDeltaPwm, MDcConsts::vPwm_l, 1 );
                break;
            case MKeyboard::UP_AUTO_CLICK :
                voltageDeltaPwm = Tools->incNum( voltageDeltaPwm, MDcConsts::vPwm_h, 8 );
                break;
            case MKeyboard::DN_AUTO_CLICK :
                voltageDeltaPwm = Tools->decNum( voltageDeltaPwm, MDcConsts::vPwm_l, 8 );
                break;
            case MKeyboard::B_CLICK :
                // Сохранить и перейти к следующему параметру
                Tools->saveInt( "dc", "vd_pwm", voltageDeltaPwm ); 
                return new MSetVoltagePwmMin(Tools);
            default :;
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
            case MKeyboard::C_LONG_CLICK :
                return new MStop(Tools);
            case MKeyboard::C_CLICK :
                return new MVoltagePwmExe(Tools);
            case MKeyboard::P_CLICK :
                return new MSetVoltagePwmMax(Tools);
            case MKeyboard::UP_CLICK :
                voltagePwmMin = Tools->incNum( voltagePwmMin, MDcConsts::vPwm_h, 1 );
                break;
            case MKeyboard::DN_CLICK :
                voltagePwmMin = Tools->decNum( voltagePwmMin, MDcConsts::vPwm_l, 1 );
                break;
            case MKeyboard::UP_AUTO_CLICK :
                voltagePwmMin = Tools->incNum( voltagePwmMin, MDcConsts::vPwm_h, voltageDeltaPwm );
                break;
            case MKeyboard::DN_AUTO_CLICK :
                voltagePwmMin = Tools->decNum( voltagePwmMin, MDcConsts::vPwm_l, voltageDeltaPwm );
                break;
            case MKeyboard::B_CLICK :
                // Сохранить и перейти к следующему параметру
                Tools->saveInt( "dc", "v_pwm_l", voltagePwmMin ); 
                return new MSetVoltagePwmMax(Tools);
            default :;
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
        Tools->showUpDn();                      // " UP/DN, В-выбор "
    }
    MState * MSetVoltagePwmMax::fsm()
    {
        switch ( Keyboard->getKey() )
        {
            case MKeyboard::C_LONG_CLICK :
                return new MStop(Tools);
            case MKeyboard::C_CLICK :
                return new MVoltagePwmExe(Tools);
            case MKeyboard::P_CLICK :
                return new MSetCurrentPwmDelta(Tools);
            case MKeyboard::UP_CLICK :
                voltagePwmMax = Tools->incNum( voltagePwmMax, MDcConsts::vPwm_h, 1 );
                break;
            case MKeyboard::DN_CLICK :
                voltagePwmMax = Tools->decNum( voltagePwmMax, MDcConsts::vPwm_l, 1 );
                break;
            case MKeyboard::UP_AUTO_CLICK :
                voltagePwmMax = Tools->incNum( voltagePwmMax, MDcConsts::vPwm_h, voltageDeltaPwm );
                break;
            case MKeyboard::DN_AUTO_CLICK :
                voltagePwmMax = Tools->decNum( voltagePwmMax, MDcConsts::vPwm_l, voltageDeltaPwm );
                break;
            case MKeyboard::B_CLICK :
                // Сохранить и перейти к выполнению
                Tools->saveInt( "dc", "v_pwm_h", voltagePwmMax ); 
                return new MVoltagePwmExe(Tools);           // Выполнять
            default :;
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
        Oled->showLine4RealVoltage();           // Показывать напряжение на клеммах (как есть)
        Oled->showLine3Num( voltagePwm );
        Tools->showUpDn();                      // Подсказка " UP/DN,В-повтор "
        Board->powOn();     Board->swOn();      // Включение преобразователя и коммутатора.
        Board->setCurrentPwm( 100 );            // Иначе не даст напряжения

        voltagePwm = voltagePwmMin;             // Заданное начальное значение PWM
        Board->setVoltagePwm( voltagePwm );
        // Печать данных для online калькулятора: просто скормите ему эту строку.
        // Если доверяете встроенному датчику напряжения (ранее откалиброванному) - 
        // раскомментируйте строку //*** ниже - данные pwm и измеренные 
        // напряжения будут перемежаться. Потребуется, естественно, некоторое
        // редактирование строк для калькулятора: pwm - это будет X, напряжения - Y.
        // Лишние пробелы и данные убрать. 
        #ifdef PRINT_PWM
            Serial.println();
            Serial.print( voltagePwm );     Serial.print( " " );    
        #endif
    }
    MState * MVoltagePwmExe::fsm()
    {
        switch ( Keyboard->getKey() )
        {
            case MKeyboard::C_LONG_CLICK :
                return new MStop(Tools);
            case MKeyboard::P_CLICK :
                return new MSetCurrentPwmDelta(Tools);
            case MKeyboard::B_CLICK :                   // Повторить регулировку по напряжению
                return new MSetVoltagePwmDelta(Tools);
            case MKeyboard::UP_CLICK :                  // Можно использовать UP и C,
            case MKeyboard::C_CLICK :                   // а DN записывать не будет.
                voltagePwm = Tools->incNum( voltagePwm, voltagePwmMax, voltageDeltaPwm );
                Board->setVoltagePwm( voltagePwm );
                #ifdef PRINT_PWM
                    Serial.print( Board->getVoltage() );    Serial.print( " " );   //***
                    Serial.print( voltagePwm );             Serial.print( " " );                                         
                #endif
                break;
            case MKeyboard::DN_CLICK :
                voltagePwm = Tools->decNum( voltagePwm, voltagePwmMin, voltageDeltaPwm );
                Board->setVoltagePwm( voltagePwm );
                break;
            default :;
        }
        #ifdef OLED_1_3
            Oled->showLine4RealVoltage();
            Oled->showLine3Num( voltagePwm );
        #endif
        return this;
    };

    // Коррекция шага приращения pwm для регулировки тока заряда.
    MSetCurrentPwmDelta::MSetCurrentPwmDelta(MTools * Tools) : MState(Tools)
    {
        currentPwmMin       = Tools->readNvsInt("dc", "i_pwm_l",    MDcConsts::iPwm_l);
        currentPwmMax       = Tools->readNvsInt("dc", "i_pwm_h",    MDcConsts::iPwm_h);
        currentDeltaPwm     = Tools->readNvsInt("dc", "id_pwm",     MDcConsts::iPwm_h / 16);

        // Индикация
        Oled->showLine4Text("  Ipwm delta ");
        Oled->showLine3Num( currentDeltaPwm );
        Tools->showUpDn();
        // Выключение преобразователя и коммутатора.
        Tools->shutdownDC();
    }
    MState * MSetCurrentPwmDelta::fsm()
    {
        switch ( Keyboard->getKey() )
        {
            case MKeyboard::C_LONG_CLICK :
                return new MStop(Tools);
            case MKeyboard::C_CLICK :
                return new MCurrentPwmExe(Tools);
            case MKeyboard::P_CLICK :
                return new MSetCurrentPwmMin(Tools);
            case MKeyboard::UP_CLICK :
                currentDeltaPwm = Tools->incNum( currentDeltaPwm, MDcConsts::iPwm_h, 1 );
                break;
            case MKeyboard::DN_CLICK :
                currentDeltaPwm = Tools->decNum( currentDeltaPwm, MDcConsts::iPwm_l, 1 );
                break;
            case MKeyboard::UP_AUTO_CLICK :
                currentDeltaPwm = Tools->incNum( currentDeltaPwm, MDcConsts::iPwm_h, 8 );
                break;
            case MKeyboard::DN_AUTO_CLICK :
                currentDeltaPwm = Tools->decNum( currentDeltaPwm, MDcConsts::iPwm_l, 8 );
                break;
            case MKeyboard::B_CLICK :
                // Сохранить и перейти к следующему параметру
                Tools->saveInt( "dc", "id_pwm", currentDeltaPwm ); 
                return new MSetCurrentPwmMin(Tools);
            default :;
        }
        #ifdef OLED_1_3
            Oled->showLine3Num( currentDeltaPwm );
        #endif
        return this;
    };

    // Коррекция pwm для минимального тока заряда.
    MSetCurrentPwmMin::MSetCurrentPwmMin(MTools * Tools) : MState(Tools)
    {
        // Индикация
        Oled->showLine4Text("   Ipwm min  ");
        Oled->showLine3Num( currentPwmMin );
        Tools->showUpDn();
    }
    MState * MSetCurrentPwmMin::fsm()
    {
        switch ( Keyboard->getKey() )
        {
            case MKeyboard::C_LONG_CLICK :
                return new MStop(Tools);
            case MKeyboard::C_CLICK :
                return new MCurrentPwmExe(Tools);
            case MKeyboard::P_CLICK :
                return new MSetCurrentPwmMax(Tools);
            case MKeyboard::UP_CLICK :
                currentPwmMin = Tools->incNum( currentPwmMin, MDcConsts::iPwm_h, 1 );
                break;
            case MKeyboard::DN_CLICK :
                currentPwmMin = Tools->decNum( currentPwmMin, MDcConsts::iPwm_l, 1 );
                break;
            case MKeyboard::UP_AUTO_CLICK :
                currentPwmMin = Tools->incNum( currentPwmMin, MDcConsts::iPwm_h, currentDeltaPwm );
                break;
            case MKeyboard::DN_AUTO_CLICK :
                currentPwmMin = Tools->decNum( currentPwmMin, MDcConsts::iPwm_l, currentDeltaPwm );
                break;
            case MKeyboard::B_CLICK :
                Tools->saveInt( "dc", "i_pwm_l", currentPwmMin ); 
                return new MSetCurrentPwmMax(Tools);
            default :;
        }
        #ifdef OLED_1_3
            Oled->showLine3Num( currentPwmMin );
        #endif
        return this;
    };

    // Коррекция pwm для максимального тока заряда.
    MSetCurrentPwmMax::MSetCurrentPwmMax(MTools * Tools) : MState(Tools)
    {
        // Индикация
        Oled->showLine4Text("   Ipwm max  ");
        Oled->showLine3Num( currentPwmMax );
        Tools->showUpDn(); // " UP/DN, В-выбор "
    }
    MState * MSetCurrentPwmMax::fsm()
    {
        switch ( Keyboard->getKey() )
        {
            case MKeyboard::C_LONG_CLICK :
                return new MStop(Tools);
            case MKeyboard::C_CLICK :
                return new MCurrentPwmExe(Tools);
            case MKeyboard::P_CLICK :                       // Продолжение выбора объекта настройки
                return new MSetDischargePwmDelta(Tools);
    //break;
            case MKeyboard::B_CLICK :                       // Сохранить и перейти к следующему параметру
                Tools->saveInt( "dc", "i_pwm_h", currentPwmMax ); 
                return new MCurrentPwmExe(Tools);           // Выполнять
            case MKeyboard::UP_CLICK :
                currentPwmMax = Tools->incNum( currentPwmMax, MDcConsts::iPwm_h, 1 );
                break;
            case MKeyboard::DN_CLICK :
                currentPwmMax = Tools->decNum( currentPwmMax, MDcConsts::iPwm_l, 1 );
                break;
            case MKeyboard::UP_AUTO_CLICK :
                currentPwmMax = Tools->incNum( currentPwmMax, MDcConsts::iPwm_h, currentDeltaPwm );
                break;
            case MKeyboard::DN_AUTO_CLICK :
                currentPwmMax = Tools->decNum( currentPwmMax, MDcConsts::iPwm_l, currentDeltaPwm );
                break;
            default:;
        }
        #ifdef OLED_1_3
            Oled->showLine3Num( currentPwmMax );
        #endif

        return this;
    };

    // Регулировка DC источника по току заряда.
    MCurrentPwmExe::MCurrentPwmExe(MTools * Tools) : MState(Tools)
    {
        // Индикация
        Oled->showLine4RealCurrent();       //  4Text("   Ipwm exe  ");
        Oled->showLine3Num( currentPwm );
        Tools->showUpDn(); // " UP/DN,В-повтор "
        Board->powOn();     Board->swOn();          // Включение преобразователя и коммутатора.
        Board->setVoltagePwm( 512 );    //1024 );                // Иначе не даст тока

        currentPwm = currentPwmMin;
        Board->setCurrentPwm( currentPwm );
    }
    MState * MCurrentPwmExe::fsm()
    {
        switch ( Keyboard->getKey() )
        {
            case MKeyboard::C_LONG_CLICK :                  // Отказ от продолжения ввода параметров - стоп
                return new MStop(Tools);
            case MKeyboard::P_CLICK :                       // Продолжение выбора объекта настройки
                return new MSetDischargePwmDelta(Tools);
    //break;
            case MKeyboard::B_CLICK :                       // Повторить регулировку по току
                return new MSetCurrentPwmDelta(Tools);
            case MKeyboard::UP_CLICK :
            case MKeyboard::C_CLICK :
                currentPwm = Tools->incNum( currentPwm, currentPwmMax, currentDeltaPwm );
                Board->setCurrentPwm( currentPwm );
                break;
            case MKeyboard::DN_CLICK :
                currentPwm = Tools->decNum( currentPwm, currentPwmMin, currentDeltaPwm );
                Board->setCurrentPwm( currentPwm );
                break;
            default:;
        }
        #ifdef OLED_1_3
            Oled->showLine4RealCurrent();
            Oled->showLine3Num( currentPwm );
        #endif

        return this;
    };

    // Коррекция шага приращения pwm для регулировки тока разряда.
    MSetDischargePwmDelta::MSetDischargePwmDelta(MTools * Tools) : MState(Tools)
    {
        dischargePwmMin     = Tools->readNvsInt("dc", "r_pwm_l",    MDcConsts::rPwm_l);
        dischargePwmMax     = Tools->readNvsInt("dc", "r_pwm_h",    MDcConsts::rPwm_h);
        dischargeDeltaPwm   = Tools->readNvsInt("dc", "rd_pwm",     MDcConsts::rPwm_h / 16);

        // Индикация
        Oled->showLine4Text("  Rpwm delta ");
        Oled->showLine3Num( dischargeDeltaPwm );
        Tools->showUpDn(); // " UP/DN, В-выбор "

        //Board->powOff();     Board->swOff();          // Выключение преобразователя и коммутатора.
        Tools->shutdownDC();

    }
    MState * MSetDischargePwmDelta::fsm()
    {
        switch ( Keyboard->getKey() )
        {
            case MKeyboard::C_LONG_CLICK :                  // Отказ от продолжения ввода параметров - стоп
                return new MStop(Tools);
            case MKeyboard::C_CLICK :
                return new MDischargePwmExe(Tools);
            case MKeyboard::P_CLICK :                       // Продолжение выбора объекта настройки
                return new MSetDischargePwmMin(Tools);
            case MKeyboard::B_CLICK :                       // Сохранить и перейти к следующему параметру
                Tools->saveInt( "dc", "id_pwm", currentDeltaPwm ); 
                return new MSetDischargePwmMin(Tools);
            case MKeyboard::UP_CLICK :
                dischargeDeltaPwm = Tools->incNum( dischargeDeltaPwm, MDcConsts::rPwm_h, 1 );
                break;
            case MKeyboard::DN_CLICK :
                dischargeDeltaPwm = Tools->decNum( dischargeDeltaPwm, MDcConsts::rPwm_l, 1 );
                break;
            case MKeyboard::UP_AUTO_CLICK:
                dischargeDeltaPwm = Tools->incNum( dischargeDeltaPwm, MDcConsts::rPwm_h, 8 );
                break;
            case MKeyboard::DN_AUTO_CLICK :
                dischargeDeltaPwm = Tools->decNum( dischargeDeltaPwm, MDcConsts::rPwm_l, 8 );
                break;
            default:;
        }
        #ifdef OLED_1_3
            Oled->showLine3Num( dischargeDeltaPwm );
        #endif

        return this;
    };

    // Коррекция pwm для минимального тока разряда.
    MSetDischargePwmMin::MSetDischargePwmMin(MTools * Tools) : MState(Tools)
    {
        // Индикация
        Oled->showLine4Text("   Rpwm min  ");
        Oled->showLine3Num( dischargePwmMin );
        Tools->showUpDn(); // " UP/DN, В-выбор "
    }
    MState * MSetDischargePwmMin::fsm()
    {
        switch ( Keyboard->getKey() )
        {
            case MKeyboard::C_LONG_CLICK :                  // Отказ от продолжения ввода параметров - стоп
                return new MStop(Tools);
            case MKeyboard::C_CLICK :
                return new MDischargePwmExe(Tools);
            case MKeyboard::P_CLICK :                       // Продолжение выбора объекта настройки
                return new MSetDischargePwmMax(Tools);
            case MKeyboard::B_CLICK :                       // Сохранить и перейти к следующему параметру
                Tools->saveInt( "dc", "r_pwm_l", dischargePwmMin ); 
                return new MSetDischargePwmMax(Tools);
            case MKeyboard::UP_CLICK :
                dischargePwmMin = Tools->incNum( dischargePwmMin, MDcConsts::rPwm_h, 1 );
                break;
            case MKeyboard::DN_CLICK :
                dischargePwmMin = Tools->decNum( dischargePwmMin, MDcConsts::rPwm_l, 1 );
                break;
            case MKeyboard::UP_AUTO_CLICK:
                dischargePwmMin = Tools->incNum( dischargePwmMin, MDcConsts::rPwm_h, dischargeDeltaPwm );
                break;
            case MKeyboard::DN_AUTO_CLICK :
                dischargePwmMin = Tools->decNum( dischargePwmMin, MDcConsts::rPwm_l, dischargeDeltaPwm );
                break;
            default:;
        }
        #ifdef OLED_1_3
            Oled->showLine3Num( dischargePwmMin );
        #endif

        return this;
    };

// Коррекция pwm для максимального тока разряда.
    MSetDischargePwmMax::MSetDischargePwmMax(MTools * Tools) : MState(Tools)
    {
        // Индикация
        Oled->showLine4Text("   Rpwm max  ");
        Oled->showLine3Num( dischargePwmMax );
        Tools->showUpDn(); // " UP/DN, В-выбор "
    }
    MState * MSetDischargePwmMax::fsm()
    {
        switch ( Keyboard->getKey() )
        {
            case MKeyboard::C_LONG_CLICK :                  // Отказ от продолжения ввода параметров - стоп
                return new MStop(Tools);
            case MKeyboard::C_CLICK :
                return new MDischargePwmExe(Tools);
            case MKeyboard::P_CLICK :                       // Продолжение выбора объекта настройки
                return new MSetDischargePwmDelta(Tools);
    //break;
            case MKeyboard::B_CLICK :                       // Сохранить и перейти к следующему параметру
                Tools->saveInt( "dc", "i_pwm_h", dischargePwmMax ); 
                return new MDischargePwmExe(Tools);           // Выполнять
            case MKeyboard::UP_CLICK :
                dischargePwmMax = Tools->incNum( dischargePwmMax, MDcConsts::rPwm_h, 1 );
                break;
            case MKeyboard::DN_CLICK :
                dischargePwmMax = Tools->decNum( dischargePwmMax, MDcConsts::rPwm_l, 1 );
                break;
            case MKeyboard::UP_AUTO_CLICK:
                dischargePwmMax = Tools->incNum( dischargePwmMax, MDcConsts::rPwm_h, dischargeDeltaPwm );
                break;
            case MKeyboard::DN_AUTO_CLICK :
                dischargePwmMax = Tools->decNum( dischargePwmMax, MDcConsts::rPwm_l, dischargeDeltaPwm );
                break;
            default:;
        }
        #ifdef OLED_1_3
            Oled->showLine3Num( currentPwmMax );
        #endif

        return this;
    };

    // Регулировка DC источника по току разряда.
    MDischargePwmExe::MDischargePwmExe(MTools * Tools) : MState(Tools)
    {
        // Индикация
        Oled->showLine4RealCurrent();       //  4Text("   Ipwm exe  ");
        Oled->showLine3Num( dischargePwm );
        Tools->showUpDn(); // " UP/DN,В-повтор "
        Board->powOn();     Board->swOn();          // Включение преобразователя и коммутатора.
        Board->setVoltagePwm( 20 );                 // Иначе не даст тока

        dischargePwm = dischargePwmMin;
        Board->setDischargePwm( dischargePwm );
    }
    MState * MDischargePwmExe::fsm()
    {
        switch ( Keyboard->getKey() )
        {
            case MKeyboard::C_LONG_CLICK :                  // Отказ от продолжения ввода параметров - стоп
                return new MStop(Tools);
            case MKeyboard::P_CLICK :                       // Продолжение выбора объекта настройки
                return new MStop(Tools);
    //break;
            case MKeyboard::B_CLICK :                       // Повторить регулировку по току
                return new MSetDischargePwmDelta(Tools);
            case MKeyboard::UP_CLICK :
            case MKeyboard::C_CLICK :
                dischargePwm = Tools->incNum( dischargePwm, dischargePwmMax, dischargeDeltaPwm );
                Board->setCurrentPwm( dischargePwm );
                break;
            case MKeyboard::DN_CLICK :
                dischargePwm = Tools->decNum( dischargePwm, dischargePwmMin, dischargeDeltaPwm );
                Board->setCurrentPwm( dischargePwm );
                break;
            default:;
        }
        #ifdef OLED_1_3
            Oled->showLine4RealCurrent();
            Oled->showLine3Num( dischargePwm );
        #endif

        return this;
    };








    // Процесс выхода из режима регулировок DC
    MStop::MStop(MTools * Tools) : MState(Tools)
    {
        Tools->shutdownDC();                 
        Oled->showLine4RealVoltage();
        Oled->showLine3RealCurrent();

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
