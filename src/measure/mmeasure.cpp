/*
    Конечный автомат обработки данных АЦП:
    напряжения с выбором диапазона,
    тока (пока выбора входа нет),
    температуры,
    выбора кнопки.
*/
#include "measure/mmeasure.h"
#include "mtools.h"
#include "board/mboard.h"
#include "measure/mkeyboard.h"
#include "display/moled.h"
#include "mstate.h"
#include <Arduino.h>
#include <stdint.h>

MMeasure::MMeasure(MTools * tools) : Tools(tools), Board(tools->Board)
{
    State = new MMeasureStates::MAdcVI(Tools);
}

void MMeasure::run()
{
    MState * newState = State->fsm();      
    if (newState != State)                      //state changed!
    {
        delete State;
        State = newState;
    } 
}

namespace MMeasureStates
{
    MState * MAdcVI::fsm()
    {
        if( cntVI == 16 )       //8 )
        {
            cntVI = 0;

            averageI = collectI / 16;       //8;
//                    Serial.println( averageI );

            Board->calcCurrent( averageI );
            collectI = 0;    
            averageV = collectV / 16;       //8;
            collectV = 0;

            // Выбор диапазона
            if( Board->getRangeV() == 0 )
            {
                // ADC_11db, accounting voltageDivider
                volt = 0.0064863f * averageV + 1.1180378f;

        //        Serial.println( volt );
                Board->calcVoltage( volt );
                if( volt <= 5.0f ) {
        //            Board->ledsOff(); Board->ledROn();
                    Board->setRangeV( 1 );
                    Board->initAdcV0db0();
                }
            }
            else
            {
                // ADC_0db, accounting voltageDivider
                volt = 0.0019166f * averageV + 0.5868099f;

        //     Serial.println( volt );
                Board->calcVoltage( volt );
                if( volt >= 6.0f ) {
        //            Board->ledsOff(); Board->ledGOn();
                    Board->setRangeV( 0 );
                    Board->initAdcV11db0();
                }
            }
            return new MAdcT(Tools);
        }
        cntVI++;
        collectV += Board->getAdcV();
        collectI += Board->getAdcI();
    //Serial.print("collectV= "); Serial.println( collectV );
        return this;
    };

    MState * MAdcT::fsm()
    {
        Tools->setCelsius( Board->getAdcT() );
        return new MAdcK(Tools);
    };

    MState * MAdcK::fsm()
    {
        Keyboard->calcKeys( Board->getAdcK() / 4 );
        return new MAdcVI(Tools);
    };

};
