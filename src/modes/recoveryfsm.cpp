/*
 * recoveryfsm.cpp - стадия экспериментов
 * В прототипе - опция расширенного заряда, в этой версии RECOVER, Recover
 * На дисплее при выборе режима отображается как " Восстановление "
 * Проект: Q921-Andr 02.01.2019         !!! Местами настройки только для 12-вольтовой батареи !!!
 * Автор: Олег Морозов 
 * Консультант: Андрей Морозов
*/

#include "modes/recoveryfsm.h"
#include "mtools.h"
#include "board/mboard.h"
#include "measure/mkeyboard.h"
#include "display/moled.h"
#include "mstate.h"
#include <Arduino.h>

namespace RecoveryFsm
{
    MState * MInvitation::fsm()
    {


//Tools->clearAllKeys ("recovery"); // Удаление всех старых ключей

        #ifdef DEBUG_CHARGE
            Serial.println("Recovery: Invitation");
        #endif
        Board->ledsOn();
        // При выборе режима основные параметры восстанавливаются такими, какими они были в крайнем
        // сеансе работы в режиме заряда АКБ из энергонезависимой памяти (qulon, recovery namespaces).
        // В случае первого запуска с чистой памятью - значения дефолтные.
        // Выбор акб при включении прибора - см конструктор MDdispatcher.
//        Tools->voltageNom    = Tools->readNvsFloat("recovery", "akbU",   Tools->akb[3][0]);    // Начальный выбор 12 вольт
//        Tools->capacity      = Tools->readNvsFloat("recovery", "akbAh",  Tools->akb[3][1]);    //                 55 Ач
        Tools->setVoltageMax( Tools->readNvsFloat("recovery", "voltMax", 14.7f) );
        Tools->setCurrentMax( Tools->readNvsFloat("recovery", "currMax",  5.4f) );
        Tools->setDurationOn( Tools->readNvsFloat("recovery", "duratOn", 5.0f ) );
        Tools->setDurationOff( Tools->readNvsFloat("recovery", "duratOff", 2.5f ) );
        
        #ifdef DEBUG_CHARGE
            Serial.print("Akb Voltage: "); Serial.println(Tools->getVoltageNom(), 0);                // формат XX
            Serial.print("Akb Ah     : "); Serial.println(Tools->getCapacity(), 0);
            Serial.print("Voltage End: "); Serial.println(Tools->getVoltageMax(), 1);                // формат XX.X
            Serial.print("Current Max: "); Serial.println(Tools->getCurrentMax(), 1);
            Serial.print("DurationOn : "); Serial.println(Tools->getDurationOn(), 1);       // формат XX.X
            Serial.print("DurationOff: "); Serial.println(Tools->getDurationOff(), 1);
        #endif

        Tools->activateChargeStart();    

        #ifdef DEBUG_CHARGE
            Serial.println("Recovery: Start");
        #endif
        return new MStart(Tools);                       // Переход в новое состояние MStart
    };

    MState * MStart::fsm()
    {
        if(Keyboard->getKey(MKeyboard::C_CLICK)) 
        {
            // Старт без уточнения параметров (быстрый старт), 
            // максимальный ток и напряжение окончания - паспортные, исходя из параметров АКБ
            // Восстановленные на предыдущем шаге данные заменяются на паспортные 

// Перенести в активацию основного заряда              
    Tools->setVoltageMax( Tools->getVoltageNom() * 1.234f );                        // Например, voltageMax = 14.8;
    Tools->setCurrentMax( Tools->getCapacity() / 10.0f );

            Tools->clrTimeCounter();
            Tools->clrAhCharge();
            
            Tools->activatePrecharge( Tools->getVoltageNom() + 0.6f, Tools->getCapacity() / 20.0f, true );    // Активация импульсного предзаряда

            #ifdef DEBUG_CHARGE
                Serial.println("Recovery: Precharge activated");
            #endif
            
            return new MPreliminaryCharge(Tools);            //return new MPhase1(Tools);
        }

        if(Keyboard->getKey(MKeyboard::P_CLICK)) 
        {
            Tools->activateSelBat();
            return new MSelectBattery(Tools);   // Переход к выбору
        }
        return this;
    };

    MState * MSelectBattery::fsm()
    {
        if(Keyboard->getKey(MKeyboard::UP_CLICK)) { Tools->incBattery(); return this; }
        if(Keyboard->getKey(MKeyboard::DN_CLICK)) { Tools->decBattery(); return this; } 

        if(Keyboard->getKey(MKeyboard::B_CLICK))              // Завершить выбор батареи
        {
            Tools->writeNvsInt( "qulon", "akbInd", Tools->getAkbInd() );
            Tools->activateSetCurrMax( true );
            return new MSetCurrentMax(Tools);
        }
        return this;
    };

    MState * MSetCurrentMax::fsm()
    {
        // if(Keyboard->getKey(MKeyboard::UP_CLICK)) { Tools->incCurrentMax( 0.1f, false ); return this; }     // Добавить 100 мА
        // if(Keyboard->getKey(MKeyboard::DN_CLICK)) { Tools->decCurrentMax( 0.1f, false ); return this; } 

        // if(Keyboard->getKey(MKeyboard::B_CLICK))      // Завершить коррекцию максимального тока заряда
        // {
        //     Tools->writeNvsFloat( "recovery", "currMax", Tools->getCurrentMax() );
        //     Tools->activateSetVoltMax( true );
        //     return new MSetvoltageMax(Tools);
        // }

        switch ( Keyboard->getKey() )
        {
            // case MKeyboard::C_LONG_CLICK :                  // Отказ от продолжения ввода параметров - стоп
            //     return new MStop(Tools);
            // case MKeyboard::C_CLICK :                       // Отказ от дальнейшего ввода параметров - исполнение
            //     return new MPostpone(Tools);
            case MKeyboard::B_CLICK :                       // Сохранить и перейти к следующему параметру
                Tools->saveFloat( "recovery", "currMax", Tools->getCurrentMax() );
    Tools->activateSetVoltMax( true );                  // старый вариант!
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

    MState * MSetVoltageMax::fsm()
    {
        if(Keyboard->getKey(MKeyboard::UP_CLICK)) { Tools->incVoltageMax( 0.1f, false ); return this; }
        if(Keyboard->getKey(MKeyboard::DN_CLICK)) { Tools->decVoltageMax( 0.1f, false ); return this; } 

        if(Keyboard->getKey(MKeyboard::B_CLICK))      // Завершить коррекцию величины напряжения окончания заряда
        {
//            Tools->saveVoltageMax("recovery");
            Tools->writeNvsFloat( "recovery", "voltMax", Tools->getVoltageMax() );

            #ifdef DEBUG_CHARGE
                Tools->setVoltageMax( Tools->readNvsFloat("recovery", "voltMax", 14.7f ) );    // Тестовая проверка
                Serial.print("New Voltage End = "); Serial.println(Tools->getVoltageMax());
                Serial.println("Recovery: MSetDurationOn activated");
            #endif

            Tools->activateSetDurationOn();
            return new MSetDurationOn(Tools);
        }
        return this;
    };

    MState * MSetDurationOn::fsm()
    {
        if(Keyboard->getKey(MKeyboard::UP_CLICK)) { Tools->incDurationOn(false ); return this; }
        if(Keyboard->getKey(MKeyboard::DN_CLICK)) { Tools->decDurationOn( false ); return this; } 

        if(Keyboard->getKey(MKeyboard::B_CLICK))      // Завершить коррекцию времени включения тока
        {
//            Tools->saveDurationOn("recovery");
            Tools->writeNvsInt( "recovery", "duratOn", Tools->getDurationOn() );

            #ifdef DEBUG_CHARGE
                Tools->setDurationOn( Tools->readNvsFloat("recovery", "duratOn", 20.0f ) );    // Тестовая проверка
                Serial.print("New Duration On = "); Serial.println(Tools->getDurationOn(), 1);
                Serial.println("Recovery: SetCharge activated");
            #endif

            Tools->activateSetDurationOff();
            return new MSetDurationOff(Tools);
        }
        return this;
    };

    MState * MSetDurationOff::fsm()
    {
        if(Keyboard->getKey(MKeyboard::UP_CLICK)) { Tools->incDurationOff( false ); return this; }
        if(Keyboard->getKey(MKeyboard::DN_CLICK)) { Tools->decDurationOff( false ); return this; } 

        if(Keyboard->getKey(MKeyboard::B_CLICK))      // Завершить коррекцию времени включения тока
        {
//            Tools->saveDurationOff("recovery");
            Tools->writeNvsInt( "recovery", "duratOff", Tools->getDurationOff() );
            Tools->clrTimeCounter();
            Tools->clrAhCharge();

            //Tools->activatePrecharge( Tools->getVoltageNom() / 2.0, Tools->getCapacity() / 20.0, true );    // Активация импульсного предзаряда
        Tools->activatePrecharge( Tools->getVoltageNom() + 0.6f, Tools->getCapacity() / 20.0f, true );    // Активация импульсного предзаряда

            #ifdef DEBUG_CHARGE
                Serial.println("Recovery: Precharge activated");
            #endif
            return new MPreliminaryCharge(Tools);
        }
        return this;
    };

// Предзаряд импульсным током.
// При глубоко разряженной АКБ процесс заряда начинается с
// восстановления импульсным током величиной, заданной пользователем,
// пока не будет достигнуто установленное напряжение окончания предзаряда
// Светодиод горит поочередно зеленым и жёлтым светом.
    MState * MPreliminaryCharge::fsm()
    {
        Tools->chargeCalculations();
        if (Keyboard->getKey(MKeyboard::C_CLICK)) { Tools->shutdownCharge();    return new MExit(Tools); }    

        if( Tools->getCycle() <= 0 )
        {
            Tools->activateSuspendCharge();   // to suspend the charge
            // Board->swOff();
            // Board->ledROn();     // Жёлтый светодиод - процесс заряда приостановлен
            // Tools->cycle = Tools->chargeOff * 2;
            return new MIdleCharge(Tools);
        }

        //if( Board->getVoltage() >= Tools->getVoltageMax() / 2.0f )    // 
        if( Board->getVoltage() >= 12.5 )                           // TEST 
        {
                 Oled->showLine2Text(" Заряжается...  ");        // " Время:  Заряд: "

            return new MPhase1(Tools);
        }

        Tools->runChargePh1();
        Tools->decCycle();

        return this;

    };

    MState * MIdleCharge::fsm()
    {
        Tools->chargeCalculations();
        if (Keyboard->getKey(MKeyboard::C_CLICK)) { Tools->shutdownCharge();    return new MExit(Tools); }    

        if( Tools->getCycle() <= 0 )
        {
            Tools->activateResumeCharge(); //to resume the charge
            // Board->swOn();
            // Board->ledsOff(); Board->ledGOn();    // Зеленый светодиод - процесс заряда запущен
            // Tools->cycle = Tools->chargeOn * 2;
            return new MPreliminaryCharge(Tools);
        }

        Tools->decCycle();
        return this;
    };




    // Начальный этап заряда - ток поднимается не выше заданного уровня,
    // при достижении напряжения выше заданного уровня - переход ко второй фазе.
    // Здесь и далее подсчитывается время и отданный заряд, а также
    // сохраняется возможность прекращения заряда оператором.
    MState * MPhase1::fsm()
    {
        Tools->chargeCalculations();
        if (Keyboard->getKey(MKeyboard::C_CLICK)) { Tools->shutdownCharge();    return new MExit(Tools); }    

        // Здесь будет проверка напряжения - setpoint по напряжению
        float setpoint = Tools->getVoltageMax() * 0.96f;      // Для второй фазы
        if( Board->getVoltage() >= setpoint )           // 96% = 14.2В
        {
            Tools->activateChargePh2( setpoint );             // output соответствует фактическому значению тока 
            #ifdef DEBUG_CHARGE
                Serial.println("Recovery: Phase2 activated");
            #endif
            return new MPhase2(Tools);                  // если достигли, иначе return this;
        }

        Tools->runChargePh1();
        return this;
    };

    // Вторая фаза заряда для заряженного примерно наполовину акб
    // В этом случае первая фаза состоит из инициализации процесса
    // Используется как промежуточное состояние для подъема тока
    MState * MPhase2recharge::fsm()
    {
        Tools->chargeCalculations();
        if (Keyboard->getKey(MKeyboard::C_CLICK)) { Tools->shutdownCharge();    return new MExit(Tools); }  

        float setpoint = Tools->getVoltageMax() * 0.96f;      // Для второй фазы
        if( Board->getVoltage() >= setpoint )
        {
            Tools->activateChargePh2( setpoint ); 
            return new MPhase2(Tools);
        }  

        Tools->runChargePh2();
        return this;
    };

    // Вторая фаза заряда - Достигнуто напряжение 96% от конечного
    // Увеличение тока, если не произошло достижения предельного уровня, остановлено на достигнутом.
    // При достижении конечного напряжения заряда - переход к третьей фазе 
    MState * MPhase2::fsm()
    {
        Tools->chargeCalculations();
        if (Keyboard->getKey(MKeyboard::C_CLICK)) { Tools->shutdownCharge();    return new MExit(Tools); }    

        if( Board->getCurrent() <= Tools->getCapacity() * 0.01f) //currentMax * 0.1 )
        {
            Tools->activateChargePh3();
            #ifdef DEBUG_CHARGE
                Serial.println("Recovery: Phase3 activated");
            #endif
            return new MPhase3(Tools);
        }

        Tools->runChargePh2();
        return this;
    };


    MState * MPhase3::fsm()
    {
        Tools->chargeCalculations();
        if (Keyboard->getKey(MKeyboard::C_CLICK)) { Tools->shutdownCharge();    return new MExit(Tools); }    

        if( ( Board->getVoltage() >= ( Tools->getVoltageMax() - 0.05f )) && ( Board->getCurrent() <= Tools->getCurrentMax() * 0.1f ) )
        {
            #ifdef DEBUG_CHARGE
                Serial.println( "Заряд закончен: по напряжению и току" );
            #endif
            Tools->shutdownCharge(); 
            return new MExit(Tools);
        } 

        // Другие проверки окончания заряда
        if( Board->getVoltage() > ( Tools->getVoltageMax() + 0.15f )) 
        {
            #ifdef DEBUG_CHARGE
                Serial.println( "Заряд закончен по напряжению" );
            #endif
            Tools->shutdownCharge(); 
            return new MExit(Tools); ;
        }
 
        if( Tools->getChargeTimeCounter() > ( Tools->charge_time_out_limit * 7200 ) )          // Фиксировано для 500мс
        {
            #ifdef DEBUG_CHARGE
                Serial.println( "Заряд закончен по времени" );      // Уточнить какое время контролировать
            #endif
            Tools->shutdownCharge(); 
            return new MExit(Tools); ;
        }

        // Ничего не делать, ПИД-регулятор остановлен
        return this;
    };

    // Процесс выхода из режима заряда - до нажатия кнопки "С" удерживается индикация
    // о продолжительности и отданном заряде.
    MState * MExit::fsm()
    {
        if(Keyboard->getKey(MKeyboard::C_CLICK)) 
        {

            #ifdef DEBUG_CHARGE
                Serial.println("Recovery: Exit"); 
                Serial.print("\t charge = ");   Serial.println( Tools->getAhCharge() );
            #endif  

            Tools->activateExit(" Восстановление ");
            return 0;   // Возврат к выбору режима
        }
        return this;
    };


};
