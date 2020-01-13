
/*
    exchargerfsm.cpp
    Конечный автомат заряда в режиме    "Импульсный заряд"
   При глубоко разряженной АКБ процесс начинается с заряда постоянным  током величиной,
 заданной пользователем (), пока не будет достигнуто установленное напряжение () окончания предзаряда.
   Основной заряд производится асимметричным током в виде импульсов  заряда и разряда
 с длительностями, установленными пользователем. Происходит плавное увеличение амплитуды импульса
 зарядного тока до максимального значения, установленного пользователем при постоянном
 контроле напряжения. При этом амплитуда импульса разрядного тока также  изменяется и
 составляет 10% от амплитуды импульса зарядного тока. При достижении заданного напряжения
 начинается постепенное снижение амплитуды импульсов зарядного тока (а также разрядного). В этом
 состоянии напряжение поднимается до максимального значения. ***
*/

#include "modes/exchargerfsm.h"
#include "measure/mkeyboard.h"
#include <Arduino.h>

namespace ExChargerFsm
{
    // Старт импульсного заряда
    MState * MStart::fsm()
    {
        if( Keyboard->getKey(MKeyboard::C_CLICK))
        {
            // Старт без уточнения параметров (быстрый старт),
            // максимальный ток и напряжение окончания - паспортные, исходя из параметров АКБ
            // Восстановленные на предыдущем шаге данные заменяются на паспортные
            Tools->setVoltageMax( Tools->getVoltageNom() * 1.234f );                  // Например, voltageMax = 14.8;
            Tools->setCurrentMax( Tools->getCapacity() / 10.0f );

            // //Активация задержки включения, аргументы для быстрого старта:
            // // напряжение окончания предзаряда и максимальный ток
            //     Tools->activatePostpone( Tools->getVoltageNom() / 2.0f, Tools->getCapacity() / 20.0f );

            return new MPostpone(Tools);
        }

        if( Keyboard->getKey(MKeyboard::P_CLICK)) return new MSetFactory(Tools);
        return this;                                    // Состояние не меняется
    };

    // Выбор заводских параметров или пользовательских ("профиль")
    MState * MSetFactory::fsm()
    {
        if ( Keyboard->getKey(MKeyboard::C_CLICK)) { return new MSetCurrentPre(Tools); }  // Отказ в восстановлении заводских параметров.

        // Восстановление заводских параметров режима импульсного заряда.
        if( Keyboard->getKey(MKeyboard::B_CLICK)) {
            Tools->clearAllKeys("e-charge");                                        // Очистка в энергонезависимой памяти.

            // Восстановление параметров заряда (после очистки – дефолтными значениями).
            Tools->setVoltageMax( Tools->readNvsFloat("cccv", "voltMax", 14.5f) );
            Tools->setVoltageMin( Tools->readNvsFloat("cccv", "voltMin", 13.2f) );
            Tools->setCurrentMax( Tools->readNvsFloat("cccv", "currMax",  5.0f) );
            Tools->setCurrentMin( Tools->readNvsFloat("cccv", "currMin",  0.5f) );
            return new MSetCurrentPre(Tools);
        }
        return this;
    };

    // Выбор максимального тока предзаряда
    MState * MSetCurrentPre::fsm()
    {
        // Выход без сохранения корректируемого параметра.
        if( Keyboard->getKey(MKeyboard::C_LONG_CLICK)) { Tools->shutdownCharge(); return new MStop(Tools); }

        // Коррекция параметра
        if( Keyboard->getKey(MKeyboard::UP_CLICK))      { Tools->incCurrentPre( 0.1f, false ); return this; }     // Добавить 100 мА
        if( Keyboard->getKey(MKeyboard::DN_CLICK))      { Tools->decCurrentPre( 0.1f, false ); return this; }
        if( Keyboard->getKey(MKeyboard::UP_LONG_CLICK)) { Tools->incCurrentPre( 0.5f, false ); return this; }     // Добавить 500 мА
        if( Keyboard->getKey(MKeyboard::DN_LONG_CLICK)) { Tools->decCurrentPre( 0.5f, false ); return this; }

        // Сохранение параметра и переход к следующему
        if( Keyboard->getKey(MKeyboard::B_CLICK))
        {
            Tools->writeNvsFloat( "e-charge", "currPre", Tools->getCurrentPre() );
            return new MSetVoltagePre(Tools);
        }
        return this;
    };

    // Выбор напряжения окончания предзаряда
    MState * MSetVoltagePre::fsm()
    {
        // Выход без сохранения корректируемого параметра
        if( Keyboard->getKey(MKeyboard::C_LONG_CLICK)) { Tools->shutdownCharge(); return new MStop(Tools); }

        if( Keyboard->getKey(MKeyboard::UP_CLICK))      { Tools->incVoltagePre( 0.1f, false ); return this; }
        if( Keyboard->getKey(MKeyboard::DN_CLICK))      { Tools->decVoltagePre( 0.1f, false ); return this; }
        if( Keyboard->getKey(MKeyboard::UP_LONG_CLICK)) { Tools->incVoltagePre( 1.0f, false ); return this; }
        if( Keyboard->getKey(MKeyboard::DN_LONG_CLICK)) { Tools->decVoltagePre( 1.0f, false ); return this; }

        if( Keyboard->getKey(MKeyboard::B_CLICK))
        {
            Tools->writeNvsFloat( "e-charge", "voltPre", Tools->getVoltagePre() );
            return new MSetCurrentMax(Tools);
        }
        return this;
    };

    // Выбор максимального тока импульсного заряда
    MState * MSetCurrentMax::fsm()
    {
        // // Выход без сохранения корректируемого параметра
        // if( Keyboard->getKey(MKeyboard::C_LONG_CLICK)) { Tools->shutdownCharge(); return new MStop(Tools); }

        // if( Keyboard->getKey(MKeyboard::UP_CLICK))      { Tools->incCurrentMax( 0.1f, false ); return this; }     // Добавить 100 мА
        // if( Keyboard->getKey(MKeyboard::DN_CLICK))      { Tools->decCurrentMax( 0.1f, false ); return this; }
        // if( Keyboard->getKey(MKeyboard::UP_LONG_CLICK)) { Tools->incCurrentMax( 0.5f, false ); return this; }     // Добавить 500 мА
        // if( Keyboard->getKey(MKeyboard::DN_LONG_CLICK)) { Tools->decCurrentMax( 0.5f, false ); return this; }

        // if( Keyboard->getKey(MKeyboard::B_CLICK))
        // {
        //     Tools->writeNvsFloat( "e-charge", "currMax", Tools->getCurrentMax() );
        //     return new MSetVoltageMax(Tools);
        // }

        switch ( Keyboard->getKey() )
        {
            case MKeyboard::C_LONG_CLICK :                  // Отказ от продолжения ввода параметров - стоп
                return new MStop(Tools);
            case MKeyboard::C_CLICK :                       // Отказ от дальнейшего ввода параметров - исполнение
                return new MPostpone(Tools);
            case MKeyboard::B_CLICK :                       // Сохранить и перейти к следующему параметру
                Tools->saveFloat( "e-charge", "currMax", Tools->getCurrentMax() ); 
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

    // Выбор напряжения окончания заряда
    MState * MSetVoltageMax::fsm()
    {
        // Выход без сохранения корректируемого параметра
        if( Keyboard->getKey(MKeyboard::C_LONG_CLICK)) { Tools->shutdownCharge(); return new MStop(Tools); }

        if( Keyboard->getKey(MKeyboard::UP_CLICK))      { Tools->incVoltageMax( 0.1f, false ); return this; }
        if( Keyboard->getKey(MKeyboard::DN_CLICK))      { Tools->decVoltageMax( 0.1f, false ); return this; }
        if( Keyboard->getKey(MKeyboard::UP_LONG_CLICK)) { Tools->incVoltageMax( 1.0f, false ); return this; }
        if( Keyboard->getKey(MKeyboard::DN_LONG_CLICK)) { Tools->decVoltageMax( 1.0f, false ); return this; }

        if( Keyboard->getKey(MKeyboard::B_CLICK))
        {   // Выбор заносится в энергонезависимую память
            Tools->writeNvsFloat( "e-charge", "voltMax", Tools->getVoltageMax() );
            return new MSetDurationOn(Tools);
        }
        return this;
    };

    // Выбор продолжительности импульса заряда
    MState * MSetDurationOn::fsm()
    {
        if( Keyboard->getKey(MKeyboard::UP_CLICK)) { Tools->incDurationOn( false ); return this; }
        if( Keyboard->getKey(MKeyboard::DN_CLICK)) { Tools->decDurationOn( false ); return this; }

        if( Keyboard->getKey(MKeyboard::B_CLICK))
        {
            Tools->writeNvsFloat( "e-charge", "duratOn", Tools->getDurationOn() );
            return new MSetDurationOff(Tools);
        }
        return this;
    };

    // Выбор продолжительности разряда
    MState * MSetDurationOff::fsm()
    {
        if( Keyboard->getKey(MKeyboard::UP_CLICK)) { Tools->incDurationOff( false ); return this; }
        if( Keyboard->getKey(MKeyboard::DN_CLICK)) { Tools->decDurationOff( false ); return this; }

        if( Keyboard->getKey(MKeyboard::B_CLICK))
        {
            Tools->writeNvsFloat( "e-charge", "duratOff", Tools->getDurationOff() );
            return new MPostpone(Tools);                // Последний параметр скорректирован, старт автоматически
        }
        return this;
    };

// Задержка включения (отложенный старт)
    MState * MPostpone::fsm()
    {
        // Возможно только досрочное прекращение заряда длинным "С" или
        if (Keyboard->getKey(MKeyboard::C_LONG_CLICK)) { return new MStop(Tools); }    

        // ... старт по времени или оператором коротким "С"
        if( Tools->postponeCalculation() || Keyboard->getKey(MKeyboard::C_CLICK ) ) { return new MPreliminaryCharge(Tools); }
        #ifdef V22
            Board->blinkWhite();        // Мигать белым при ожидании
        #endif
        return this;
    };

    // Предзаряд постоянным током.
    // При глубоко разряженной АКБ процесс заряда начинается с восстановления величиной тока, заданной пользователем,
    // пока не будет достигнуто установленное напряжение окончания предзаряда. Светодиод горит зеленым светом.
    MState * MPreliminaryCharge::fsm()
    {
        Tools->chargeCalculations();                        // Подсчет отданных ампер-часов.

        // После пуска короткое нажатие кнопки "C" производит отключение тока.
        if( Keyboard->getKey(MKeyboard::C_CLICK)) { return new MStop(Tools); }

        // Проверка окончания предзаряда, переход к импульсному заряду с набором тока
        if( Board->getVoltage() >= Tools->getVoltagePre() ) 
        {
            // Для первого импульса ток устанавливается на уровне достигнутого в предзаряде
            Tools->setCurrentAvr( Board->getCurrent() );
            return new MPulseUp(Tools); 
        }

        // Коррекция источника тока по измерению (установщик откалиброван с некоторым завышением)
        if( Board->getCurrent() > Tools->getCurrentPre() ) { Tools->adjustIntegral( -0.250f ); } // -0.025A
        Tools->runPid( Board->getVoltage() );     // Подъём и поддержание тока.
        return this;
    };

// Импульс заряда в фазе набора тока
    MState * MPulseUp::fsm()
    {
        Tools->chargeCalculations();                        // Подсчет отданных ампер-часов.

        if( Keyboard->getKey(MKeyboard::C_CLICK)) { return new MStop(Tools); }


        if( Tools->getCycle() <= 0 )
        {
    //  Tools->activateImpulsDischarge( Tools->getCurrentAvr() / 10.0f );
    //            #ifdef DEBUG_CHARGE
    //                Serial.println("#-");
    //            #endif
     Serial.print("#+0 currentAvr ");    Serial.println(Tools->getCurrentAvr() );
        Tools->setCurrentAvr( Tools->stopPulse() );
     Serial.print("#+1 currentAvr =? ");    Serial.println(Tools->getCurrentAvr() );

                return new MDischUp(Tools);
        }

    //    Tools->runImpulsCharge();               // Импульс заряда с подъемом тока


    
    //    cycle--;
    //    Serial.print("#+ ");    Serial.println(cycle);
    #ifdef PID_CHARGE   
        Tools->runPulse();
    #else
        Board->setCurrentAmp( Tools->getCurrentAvr() );
        Tools->addCollectAvr( Board->getCurrent() );
        Tools->decCycle();
    #endif
        return this;
    };

// Импульс разряда в фазе подъёма тока
    MState * MDischUp::fsm()
    {
        Tools->chargeCalculations();

        if( Keyboard->getKey(MKeyboard::C_CLICK)) { return new MStop(Tools); }

        if( Tools->getCycle() <= 0 )
        {
            // Импульс разряда закончен по времени
            Tools->stopPulse();         // среднее не используется
            return new MPulseUp(Tools);
        }
        // Поддерживается ток, заданный при инициализации
        Tools->decCycle();
        return this;
    };













    // ****************************

    // Импульс разряда в фазе удержания максимального напряжения
    MState * MImpulsDischarge2::fsm()
    {
        //Tools->chargeCalculations();  // Пока подсчета нет
        //if( Keyboard->getKey(MKeyboard::C_CLICK)) { Tools->shutdownCharge();    return new MStop(Tools); }
        if( Keyboard->getKey(MKeyboard::C_CLICK)) { return new MStop(Tools); }

            // #ifdef DEBUG_CHARGE
            //     Serial.print("@- : "); Serial.print( Tools->cycle );  Serial.print(" old ");  Serial.println( Tools->currentOld );
            // #endif

        if (Tools->getCycle() == 0)
        {
            Board->setDischargeAmp( 0.0 );
            Board->setCurrentAmp( 0.0 );

            // Импульс разряда закончен по времени, активируется импульс заряда - всё уточнить
            Tools->activateImpulsCharge2( Tools->getVoltageMax(), Tools->getCurrentAvr() );   // * 0.9 ); 

            return new MImpulsCharge2(Tools);
        }

        //Tools->runImpulsDischarge2();
      //  Tools->decCycle();
      //Tools->runImpDisch();

        return this;
    };

    // Импульс заряда в фазе удержания максимального напряжения
    MState * MImpulsCharge2::fsm()
    {
        Tools->chargeCalculations();
        //if( Keyboard->getKey(MKeyboard::C_CLICK)) { Tools->shutdownCharge();    return new MStop(Tools); }
        if( Keyboard->getKey(MKeyboard::C_CLICK)) { return new MStop(Tools); }

            // Проверяются условия окончания заряда
            //float curr = Tools->getCurrentAvr();       //Board->getCurrent();

            Serial.print(" Ток : "); Serial.println( Tools->getCurrentAvr() );
            if( Tools->getCurrentAvr() <= Tools->getCurrentMax() * 0.1f )            // другие проверки: || (         ) )
            {
                            Serial.print(" Ток принятия решения: "); Serial.println( Tools->getCurrentAvr() );

            //Tools->shutdownCharge();
            return new MStop(Tools);
            }



        if( Tools->getCycle() <= 0 ) // // // 
        {

            // Импульс заряда закончен по времени, активируется импульс разряда
            Tools->activateImpulsDischarge2( Board->getCurrent() );     // Для последующего импульса заряда
            return new MImpulsDischarge2(Tools);
        }

        Tools->runImpulsCharge2();
        Tools->decCycle();
        return this;
    };

    // Процесс выхода из режима заряда - до нажатия кнопки "С" удерживается индикация
    // о продолжительности и отданном заряде.
    MState * MStop::fsm()
    {
        if(Keyboard->getKey(MKeyboard::C_CLICK)) 
        { 
            Tools->activateExit(" Импульсный зар."); 
            return 0; 
        }   // Возврат к выбору режима
        return this;
    };
};
