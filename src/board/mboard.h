#ifndef _MBOARD_H_
#define _MBOARD_H_

/*
 * Вариант Q921v502
 */

//#include "esp32-hal.h"
#include "stdint.h"

class MOled;
class MOverseer;

class MBoard
{
  public:
    MOled     * Oled     = nullptr;   // external
    MOverseer * Overseer = nullptr;   // local

        // PWM channels
    static constexpr uint8_t  ch_current   = 0;  //
    static constexpr uint8_t  ch_voltage   = 1;  // 
    static constexpr uint8_t  ch_discharge = 2;  // 
    static constexpr uint8_t  ch_fan       = 3;  // 

  public:
    MBoard(MOled * oled);

    void initPorts();
    #ifdef V22
      void ledROn();	  // Turn the RED LED on
      void ledROff();	  // Turn the RED LED off

      void ledGOn();    // Turn the GREEN LED on
      void ledGOff();	  // Turn the GREEN LED off

      void ledBOn();	  // Turn the BLUE LED on
      void ledBOff();	  // Turn the BLUE LED off

      void ledsOn();	
      void ledsOff();
      void ledsRed();
      void ledsGreen();
      void ledsYellow();	
    #endif
    //void buzzerOn();
    //void buzzerOff();

    void powOn();	    // Turn the POW on
    void powOff();	  // Turn the POW off

    void swOn();	    // Turn the ON on
    void swOff();	    // Turn the ON off

    bool getNeg();
    bool getFastOff();
    bool getPowGood();
    #ifdef V22
      void blinkWhite();
      void blinkRed();
      void blinkGreen();
      void blinkBlue();
      void blinkYellow();
    #endif
//=================201904==================

    //void initAdcV11db();
    int adcV_11dbRead();

    void initAdcV11db0();
    void initAdcV6db0();
    void initAdcV2db5();
    void initAdcV0db0();

    void initAdcI11db0();
    #ifdef V43
        void initAdcI3_11db0();
    #endif
    void initAdcT11db0();
    void initAdcK11db0();

    int getAdcV();
    int getAdcI();
    #ifdef V43
        int getAdcI3();
    #endif
    int getAdcT();
    int getAdcK();

//==========================================

    void setVoltage(float);
    void setCurrent(float);

    float getVoltage();
    float getCurrent();

//    void  setCelsius(float);
//    float getCelsius();

    void  setOffsetU(float);
    float getOffsetU();

    void  setOffsetI(float);
    float getOffsetI();

    void  setWiresResistance(float);
    float getWiresResistance();

    float readVoltage( int ivalue );
    float readVoltage0db( int ivalue );
  
    //void runKeys();
    //void calcKeys(int ivalue);

    void calcVoltage( float volt );
    void calcCurrent( int ivalue );
    #ifdef V43
        void calcCurrentI3( int ivalue );
    #endif
//    void runCool();

    void setCurrentAmp( float ampers );
    void setVoltageVolt( float volts );
    void setDischargeAmp( float ampers );

//    int  getFanPwm();

    void setRangeV(int range);
    int  getRangeV();

    // Калибровки измерителей
    float       voltageMultiplier =   1.0000;                   // Калибровка (погрешность делителя)
    float       voltageOffset     =   0.0000;                   // Калибровка           "
    float       currentMultiplier =   1.0000;                   // Калибровка (погрешность экземпляра 712)
    float       currentOffset     =   0.1500;                   // Калибровка           "

  private:
    float voltage         =  0.0f;    // Напряжение на клеммах аккумулятора, В ( с поправкой на падение напряжения на проводах)
    float current         =  0.0f;    // Текущий измеренный ток, А
    #ifdef V43
      float current3      =  0.0f;    // Текущий измеренный шунтом ток, А
    #endif
    float wiresResistance =  0.14f;   // The resistance of the wires (KofPad) 0,14 Ом - провода длинные, ок 3м, при токе ок 1,2А, подбором
    float offsetU         =  1.10f;   // Подбор
    float offsetI         = -0.08f;

    //const uint8_t adc_samples = 1;    //Set number of samples in the range, default is 1, it has an effect of increasing sensitivity
    const float ratioU = (( 39.0f + 5.6f ) / 5.6f ) * 3.3f / 4095.0f ;  // (KU) The voltage conversion ratio 
    const float ratioI = ( 16.8f / 5.0f ) * (( 30.0f + 30.0f ) / 30.0f ) * ( 3.3f / 4095.0f ); // The current conversion ratio
    // 11.04.2019
    const float voltageDivider    = ( 39.0f + 5.6f ) / 5.6f;  // Делитель 39кОм 1% / 5.6кОм 1%
    const float acs712Multiplier  = -12.50f;                  //  80mV/А  ( 1/0.080)
    const float acs712Offset      =  20.00f;

    
    //void initKeys();
//    void initCool();
//    void setFan(uint16_t val);

    uint16_t pwm( float ampers );
    int rangeV = 0;

};

#endif //_MBOARD_H_
