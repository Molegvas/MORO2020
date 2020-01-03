/*
    cccvfsm.cpp
    Конечный автомат заряда в режиме CC/CV.
    Такой алгоритм обеспечивает достаточно быстрый и «бережный» режим заряда аккумулятора. Для исключения
    долговременного пребывания аккумулятора в конце процесса заряда устройство переходит в режим поддержания 
    (компенсации тока саморазряда) напряжения на аккумуляторе. Такой алгоритм называется трехступенчатым. 
    График такого алгоритма заряда представлен на рисунке  http://www.balsat.ru/statia2.php
    27.05.2019 
*/

#include "modes/cccvfsm.h"
#include "mtools.h"
#include "board/mboard.h"
#include "measure/mkeyboard.h"
#include "display/moled.h"
#include <Arduino.h>

namespace CcCvFsm
{
    // Состояние "Старт", инициализация выбранного режима работы (Заряд CCCV).
    MStart::MStart(MTools * Tools) : MState(Tools)
    {
        // Параметры заряда из энергонезависимой памяти, Занесенные в нее при предыдущих включениях, как и
        // выбранные ранее номинальные параметры батареи (напряжение, емкость).
        // При первом включении, как правило заводском, номиналы батареи задаются в mdispather.h. 
        Tools->setVoltageMax( Tools->readNvsFloat("cccv", "voltMax", MChConsts::voltageMaxFactor * Tools->getVoltageNom()) );
        Tools->setVoltageMin( Tools->readNvsFloat("cccv", "voltMin", MChConsts::voltageMinFactor * Tools->getVoltageNom()) );
        Tools->setCurrentMax( Tools->readNvsFloat("cccv", "currMax", MChConsts::currentMaxFactor * Tools->getCapacity()) );
        Tools->setCurrentMin( Tools->readNvsFloat("cccv", "currMin", MChConsts::currentMinFactor * Tools->getCapacity()) );

        // Индикация
        #ifdef OLED_1_3
            Oled->showLine4Text("   Зарядное  ");
            Oled->showLine3Akb( Tools->getVoltageNom(), Tools->getCapacity() );              // example: "  12В  55Ач  "
            Oled->showLine2Text(" P-корр.С-старт ");        // Подсказка: активны две кнопки: P-сменить настройки, и C-старт
            Oled->showLine1Time(0);                         // уточнить
            Oled->showLine1Ah(0.0);                         // уточнить
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
            case MKeyboard::C_CLICK :
                // Старт без уточнения параметров (здесь – для батарей типа AGM), 
                // максимальный ток и напряжение окончания - паспортные, исходя из параметров АКБ 
                // Выбор АКБ производится в "Настройках".
                        // MChConsts::xxxFactor * Tools->getXxxx(),

                Tools->setVoltageMax( MChConsts::voltageMaxFactor * Tools->getVoltageNom() );    // Например, voltageMax = 14.8;
                Tools->setVoltageMin( MChConsts::voltageMinFactor * Tools->getVoltageNom() );
                Tools->setCurrentMax( MChConsts::currentMaxFactor * Tools->getCapacity() );
                Tools->setCurrentMin( MChConsts::currentMinFactor * Tools->getCapacity() );
                return new MPostpone(Tools);
            case MKeyboard::P_CLICK :
                return new MSetFactory(Tools);      // Выбрано уточнение настроек заряда.
            default:;
        }

        return this;
    };

    // Выбор заводских параметров или пользовательских ("профиль")
    MSetFactory::MSetFactory(MTools * Tools) : MState(Tools)
    {
        // Индикация
        #ifdef OLED_1_3
            Oled->showLine4Text("   Factory   ");
            Oled->showLine3Text("     Y/NO    ");
            Oled->showLine2Text("  B-yes,  C-no  ");
        #endif

        #ifdef TFT_1_44
            // no leds
        #endif
        #ifdef TFT_1_8
            // no leds
        #endif
    }
    MState * MSetFactory::fsm()
    {
        switch ( Keyboard->getKey() )
        {
            case MKeyboard::C_CLICK :
                return new MSetCurrentMax(Tools);           // Отказ в восстановлении заводских параметров.
            case MKeyboard::B_CLICK :
                Tools->clearAllKeys("cccv");                // Очистка в энергонезависимой памяти.

                // Восстановление параметров заряда (после очистки – дефолтными значениями).
                Tools->setVoltageMax( Tools->readNvsFloat("cccv", "voltMax", MChConsts::voltageMaxFactor * Tools->getVoltageNom()) );
                Tools->setVoltageMin( Tools->readNvsFloat("cccv", "voltMin", MChConsts::voltageMinFactor * Tools->getVoltageNom()) );
                Tools->setCurrentMax( Tools->readNvsFloat("cccv", "currMax", MChConsts::currentMaxFactor * Tools->getCapacity()) );
                Tools->setCurrentMin( Tools->readNvsFloat("cccv", "currMin", MChConsts::currentMinFactor * Tools->getCapacity()) );
                return new MSetCurrentMax(Tools);
            default:;
        }

        return this;
    };

    // Коррекция максимального тока заряда.
    MSetCurrentMax::MSetCurrentMax(MTools * Tools) : MState(Tools)
    {
        // Индикация
        #ifdef OLED_1_3
            Oled->showLine4Text(" Imax заряда ");
            Oled->showLine3MaxI( Tools->getCurrentMax() );
            Tools->showUpDn(); // " UP/DN, В-выбор "
        #endif
    }
    MState * MSetCurrentMax::fsm()
    {
        switch ( Keyboard->getKey() )
        {
            case MKeyboard::C_LONG_CLICK :                  // Отказ от продолжения ввода параметров - стоп
                return new MStop(Tools);
            case MKeyboard::C_CLICK :                       // Отказ от дальнейшего ввода параметров - исполнение
                return new MPostpone(Tools);
            case MKeyboard::B_CLICK :                       // Сохранить и перейти к следующему параметру
                Tools->saveFloat( "cccv", "currMax", Tools->getCurrentMax() ); 
                return new MSetVoltageMax(Tools);

            case MKeyboard::UP_CLICK :
                Tools->incCurrentMax( 0.1f, false );        // По кольцу? - Нет
                break;
            case MKeyboard::DN_CLICK:
                Tools->decCurrentMax( 0.1f, false );
                break;
            case MKeyboard::UP_AUTO_CLICK:
                Tools->incCurrentMax( 0.1f, false );
                break;
            case MKeyboard::DN_AUTO_CLICK:
                Tools->decCurrentMax( 0.1f, false );
                break;
            default:;
        }
        #ifdef OLED_1_3
            Oled->showLine3MaxI( Tools->getCurrentMax() );
        #endif
        return this;
    };

    // Коррекция максимального напряжения.
    MSetVoltageMax::MSetVoltageMax(MTools * Tools) : MState(Tools)
    {
        // Индикация
        Oled->showLine4Text(" Vmax заряда ");
        Oled->showLine3MaxU( Tools->getVoltageMax() );
        Tools->showUpDn(); // " UP/DN, В-выбор "
    }
    MState * MSetVoltageMax::fsm()
    {
        switch ( Keyboard->getKey() )
        {
            case MKeyboard::C_LONG_CLICK :                  // Отказ от продолжения ввода параметров - стоп
                return new MStop(Tools);
            case MKeyboard::C_CLICK :                       // Отказ от дальнейшего ввода параметров - исполнение
                return new MPostpone(Tools);
            case MKeyboard::B_CLICK :                       // Сохранить и перейти к следующему параметру
                Tools->saveFloat( "cccv", "voltMax", Tools->getVoltageMax() ); 
                return new MSetCurrentMin(Tools);

            case MKeyboard::UP_CLICK :
                Tools->incVoltageMax( 0.1f, false );        // По кольцу? - Нет
                break;
            case MKeyboard::DN_CLICK:
                Tools->decVoltageMax( 0.1f, false );
                break;
            case MKeyboard::UP_AUTO_CLICK:
                Tools->incVoltageMax( 0.1f, false );
                break;
            case MKeyboard::DN_AUTO_CLICK:
                Tools->decVoltageMax( 0.1f, false );
                break;
            default:;
        }
        #ifdef OLED_1_3
            Oled->showLine3MaxU( Tools->getVoltageMax() );
        #endif

        return this;
    };

    // Коррекция минимального тока заряда
    MSetCurrentMin::MSetCurrentMin(MTools * Tools) : MState(Tools)
    {
        // Индикация
        Oled->showLine4Text(" Imin заряда ");
        Oled->showLine3MaxI( Tools->getCurrentMin() );
        Tools->showUpDn(); // " UP/DN, В-выбор "
    }   
    MState * MSetCurrentMin::fsm()
    {
        switch ( Keyboard->getKey() )
        {
            case MKeyboard::C_LONG_CLICK :                  // Отказ от продолжения ввода параметров - стоп
                return new MStop(Tools);
            case MKeyboard::C_CLICK :                       // Отказ от дальнейшего ввода параметров - исполнение
                return new MPostpone(Tools);
            case MKeyboard::B_CLICK :                       // Сохранить и перейти к следующему параметру
                Tools->saveFloat( "cccv", "currMin", Tools->getCurrentMin() ); 
                return new MSetVoltageMin(Tools);

            case MKeyboard::UP_CLICK :
                Tools->incCurrentMin( 0.1f, false );        // По кольцу? - Нет
                break;
            case MKeyboard::DN_CLICK:
                Tools->decCurrentMin( 0.1f, false );
                break;
            case MKeyboard::UP_AUTO_CLICK:
                Tools->incCurrentMin( 0.1f, false );
                break;
            case MKeyboard::DN_AUTO_CLICK:
                Tools->decCurrentMin( 0.1f, false );
                break;
            default:;
        }
        #ifdef OLED_1_3
            Oled->showLine3MaxI( Tools->getCurrentMin() );
        #endif

        return this;
    };

    // Коррекция минимального напряжения окончания заряда.
    MSetVoltageMin::MSetVoltageMin(MTools * Tools) : MState(Tools)
    {
        // Индикация
        Oled->showLine4Text(" Vmin заряда ");
        Oled->showLine3MaxU( Tools->getVoltageMin() );
        Tools->showUpDn(); // " UP/DN, В-выбор "
    }   
    MState * MSetVoltageMin::fsm()
    {
        switch ( Keyboard->getKey() )
        {
            case MKeyboard::C_LONG_CLICK :                  // Отказ от продолжения ввода параметров - стоп
                return new MStop(Tools);
            case MKeyboard::C_CLICK :                       // Отказ от дальнейшего ввода параметров - исполнение
                return new MPostpone(Tools);
            case MKeyboard::B_CLICK :                       // Сохранить и перейти к следующему параметру
                Tools->saveFloat( "cccv", "voltMin", Tools->getVoltageMin() ); 
                return new MPostpone(Tools);

            case MKeyboard::UP_CLICK :
                Tools->incVoltageMin( 0.1f, false );        // По кольцу? - Нет
                break;
            case MKeyboard::DN_CLICK:
                Tools->decVoltageMin( 0.1f, false );
                break;
            case MKeyboard::UP_AUTO_CLICK:
                Tools->incVoltageMin( 0.1f, false );
                break;
            case MKeyboard::DN_AUTO_CLICK:
                Tools->decVoltageMin( 0.1f, false );
                break;
            default:;
        }
        #ifdef OLED_1_3
            Oled->showLine3MaxU( Tools->getVoltageMin() );
        #endif

        return this;
    };
// **************************************************************************************

    // Задержка включения (отложенный старт), светидиод белый, мигает.
    MPostpone::MPostpone(MTools * Tools) : MState(Tools)
    {
        // Параметр задержки начала заряда из энергонезависимой памяти, при первом включении - заводское
        Tools->postpone = Tools->readNvsInt( "qulon", "postp", 0 );
                
        // Индикация
        Oled->showLine4RealVoltage();
        Oled->showLine3RealCurrent();
        Oled->showLine2Text(" До старта...   ");

        // Инициализация счетчика времени до старта
        Tools->setTimeCounter( Tools->postpone * 36000 );                // Отложенный старт ( * 0.1s )
    }     
    MState * MPostpone::fsm()
    {
        // // Возможно досрочное прекращение заряда или
        // if (Keyboard->getKey(MKeyboard::C_LONG_CLICK)) { return new MStop(Tools); }    

        // // ... старт по времени или оператором
        // if( Tools->postponeCalculation() || Keyboard->getKey(MKeyboard::C_CLICK ) ) { return new MUpCurrent(Tools); }

        if( Tools->postponeCalculation() ) return new MUpCurrent(Tools);    // Старт по времени

        switch ( Keyboard->getKey() )
        {
            case MKeyboard::C_LONG_CLICK :        // Досрочное прекращение заряда оператором
                return new MStop(Tools);
            case MKeyboard::C_CLICK :
                return new MStop(Tools);
            default:;
        }
        #ifndef OLED_1_3
            Board->blinkWhite();           // Мигать белым – время пошло.
        #endif
        return this;
    };

    // Начальный этап заряда - ток поднимается не выше заданного уровня,
    // при достижении заданного максимального напряжения переход к его удержанию.
    // Здесь и далее подсчитывается время и отданный заряд, а также
    // сохраняется возможность прекращения заряда оператором.
    MUpCurrent::MUpCurrent(MTools * Tools) : MState(Tools)
    {
        // Обнуляются счетчики времени и отданного заряда
        Tools->clrTimeCounter();
        Tools->clrAhCharge();

        // Включение преобразователя и коммутатора (уточнить порядок?)
        Board->setDischargeAmp( 0.0f );
        Board->setCurrentAmp( 0.0f );               // Ток в начале будет ограничен
        Board->powOn();     Board->swOn();          // Включение преобразователя и коммутатора.
        Board->ledsOff();   Board->ledGOn();        // Зеленый светодиод - процесс заряда запущен
        // Индикация построчно (4-я строка - верхняя)
        Oled->showLine4RealVoltage();
        Oled->showLine3RealCurrent();
        Oled->showLine2Text("  I up & const  ");
        Oled->showLine1Time( Tools->getChargeTimeCounter() );
        Oled->showLine1Ah( Tools->getAhCharge() );
                
        // Настройка ПИД-регулятора
        Tools->initPid( MPidConstants::outputMin,
                        MPidConstants::outputMaxFactor * Tools->getCurrentMax(),
                        MPidConstants::k_p,
                        MPidConstants::k_i,
                        MPidConstants::k_d,
                        MPidConstants::bangMin,
                        MPidConstants::bangMax,
                        MPidConstants::timeStep );
        Tools->setSetPoint( Tools->getVoltageMax() );                                      //лишнее?

        // ???
        Board->setVoltageVolt( Tools->getVoltageMax() * 1.05f );     // Voltage limit
    }     
    MUpCurrent::MState * MUpCurrent::fsm()
    {
        Tools->chargeCalculations();                                                    // Подсчет отданных ампер-часов.

        // После пуска короткое нажатие кнопки "C" производит отключение тока.
        if(Keyboard->getKey(MKeyboard::C_CLICK)) { return new MStop(Tools); }    

        // Проверка напряжения и переход на поддержание напряжения.
        if( Board->getVoltage() >= Tools->getVoltageMax() ) { return new MKeepVmax(Tools); }

        // Коррекция источника тока по измерению (установщик откалиброван с некоторым завышением)
        if( Board->getCurrent() > Tools->getCurrentMax() ) { Tools->adjustIntegral( -0.250f ); } // -0.025A

        Tools->runPid( Board->getVoltage() );                                           // Подъём и поддержание тока.
        return this;
    };

    // Вторая фаза заряда - достигнуто заданное максимальное напряжение.
    // При падении тока ниже заданного уровня - переход к третьей фазе.
    MKeepVmax::MKeepVmax(MTools * Tools) : MState(Tools)
    {
        // Порог регулирования по напряжению
        Tools->setSetPoint( Tools->getVoltageMax() );
        // Индикация
        Oled->showLine2Text("  const Vmax... ");
        Board->ledROn();                // Желтый светодиод (R & G) - процесс поддержания максимального напряжения
    }       
    MState * MKeepVmax::fsm()
    {
        Tools->chargeCalculations();                                                    // Подсчет отданных ампер-часов.

        if (Keyboard->getKey(MKeyboard::C_CLICK)) { return new MStop(Tools); }                // Окончание процесса оператором.

        if( Board->getCurrent() <= Tools->getCurrentMin() ) { return new MKeepVmin(Tools); } // Ожидание спада тока ниже C/20 ампер.

        // Коррекция
        if( Board->getCurrent() > Tools->getCurrentMax() ) { Tools->adjustIntegral( -0.250f ); } // -0.025A

        Tools->runPid( Board->getVoltage() );                                           // Поддержание максимального напряжения.
        return this;
    };

    // Третья фаза заряда - достигнуто снижение тока заряда ниже заданного предела.
    // Проверки различных причин завершения заряда.
    MKeepVmin::MKeepVmin(MTools * Tools) : MState(Tools)
    {
        // Порог регулирования по напряжению
        Tools->setSetPoint( Tools->getVoltageMin() );
                
        // Индикация
        Oled->showLine2Text("  const Vmin... ");
    }     
    MState * MKeepVmin::fsm()
    {
        Tools->chargeCalculations();                                                    // Подсчет отданных ампер-часов.

        if (Keyboard->getKey(MKeyboard::C_CLICK)) { return new MStop(Tools); }                // Окончание процесса оператором.

        // Здесь возможны проверки других условий окончания заряда
        // if( ( ... >= ... ) && ( ... <= ... ) )  { return new MStop(Tools); }

        // Максимальное время заряда, задается в "Настройках"
        if( Tools->getChargeTimeCounter() >= ( Tools->charge_time_out_limit * 36000 ) ) { return new MStop(Tools); }

        // Необходимая коррекция против выброса тока
        if( Board->getCurrent() > Tools->getCurrentMax() ) { Tools->adjustIntegral( -0.250f ); }        // -0.025A

        Board->blinkYellow();           // Желтый светодиод мигает - процесс поддержания минимального напряжения
        Tools->runPid( Board->getVoltage() );           // Регулировка по напряжению
        return this;
    };

    // Процесс выхода из режима заряда - до нажатия кнопки "С" удерживается индикация о продолжительности и отданном заряде.
    MStop::MStop(MTools * Tools) : MState(Tools)
    {
        Tools->shutdownCharge();            // Включение красного светодиода здесь
    }    
    MState * MStop::fsm()
    {
        if(Keyboard->getKey(MKeyboard::C_CLICK)) { Tools->activateExit("  CC/CV  заряд  "); return nullptr; }   // Возврат к выбору режима

        return this;
    };
};
