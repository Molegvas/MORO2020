/*
  Надо будет попробовать http://forum.amperka.ru/threads/Хаос-в-значениях-АЦП-esp32.19985/

*/


#include "board/mboard.h"
#include "board/mpins.h"
#include "display/moled.h"
#include "board/moverseer.h"
#include <AutoPID.h>  // 201905 double ---> float
#include <Arduino.h>

//using namespace std;

MBoard::MBoard(MOled * oled) : Oled(oled)
{
	  initPorts();
	  //initKeys();
    //initCool();
    Overseer = new MOverseer(this);
}
#ifdef V22
  void MBoard::ledROn()	  { digitalWrite(MPins::led_r_pin, LOW); }   // Turn the RED LED on
  void MBoard::ledROff()	{ digitalWrite(MPins::led_r_pin, HIGH); }  // Turn the RED LED off
  void MBoard::ledGOn()	  { digitalWrite(MPins::led_g_pin, LOW); }   // Turn the GREEN LED on
  void MBoard::ledGOff()	{ digitalWrite(MPins::led_g_pin, HIGH); }  // Turn the GREEN LED off
  void MBoard::ledBOn()	  { digitalWrite(MPins::led_b_pin, LOW); }   // Turn the BLUE LED on
  void MBoard::ledBOff()	{ digitalWrite(MPins::led_b_pin, HIGH); }  // Turn the BLUE LED off

  void MBoard::ledsOn()	    { ledROn(); ledGOn(); ledBOn(); }
  void MBoard::ledsOff()	  { ledROff(); ledGOff(); ledBOff(); }
  void MBoard::ledsRed()    { ledROn(); ledGOff(); ledBOff(); }   // Красный светодиод
  void MBoard::ledsGreen()  { ledROff(); ledGOn(); ledBOff(); }   // Зеленый светодиод
  void MBoard::ledsYellow() { ledROn(); ledGOn(); ledBOff(); }    // Желтый светодиод
#endif

//void MBoard::buzzerOn()   { digitalWrite(buz_pin, HIGH); }
//void MBoard::buzzerOff()  { digitalWrite(buz_pin, LOW); }

void MBoard::powOn()	{ digitalWrite(MPins::pow_pin, HIGH); }  // Turn the POW on
void MBoard::powOff()	{ digitalWrite(MPins::pow_pin, LOW); }  	// Turn the POW off
void MBoard::swOn()		{ digitalWrite(MPins::on_pin, HIGH); }   // Turn the ON on
void MBoard::swOff()	{ digitalWrite(MPins::on_pin, LOW); }  	// Turn the ON off

bool MBoard::getNeg() 		{ return(digitalRead(MPins::neg_pin)); }
bool MBoard::getFastOff() { return(digitalRead(MPins::fast_off_pin)); }
bool MBoard::getPowGood() { return(digitalRead(MPins::pow_good_pin)); }

//=================201904==================

int MBoard::adcV_11dbRead()
{
    analogSetPinAttenuation( MPins::pinV, ADC_11db );	// Порт измерителя напряжения
    return analogRead( MPins::pinV );
}
    void MBoard::initAdcV11db0() { adcAttachPin( MPins::pinV );  analogSetPinAttenuation( MPins::pinV, ADC_11db); }
    void MBoard::initAdcV6db0()  { adcAttachPin( MPins::pinV );  analogSetPinAttenuation( MPins::pinV, ADC_6db); }
    void MBoard::initAdcV2db5()  { adcAttachPin( MPins::pinV );  analogSetPinAttenuation( MPins::pinV, ADC_2_5db); }
    void MBoard::initAdcV0db0()  { adcAttachPin( MPins::pinV );  analogSetPinAttenuation( MPins::pinV, ADC_0db); }

    void MBoard::initAdcI11db0() { adcAttachPin( MPins::pinI );  analogSetPinAttenuation( MPins::pinI, ADC_11db); }
    #ifdef V43
        void MBoard::initAdcI3_11db0() { adcAttachPin( MPins::pinI3 );  analogSetPinAttenuation( MPins::pinI3, ADC_11db); }
    #endif

    void MBoard::initAdcT11db0() { adcAttachPin( MPins::pinT );  analogSetPinAttenuation( MPins::pinT,  ADC_11db); }
    void MBoard::initAdcK11db0() { adcAttachPin( MPins::pinK );  analogSetPinAttenuation( MPins::pinK,  ADC_11db); }

    int MBoard::getAdcV() { return analogRead( MPins::pinV ); }
    int MBoard::getAdcI() { return analogRead( MPins::pinI ); }
    #ifdef V43
        int MBoard::getAdcI3() { return analogRead( MPins::pinI3 ); }
    #endif
    int MBoard::getAdcT() { return analogRead( MPins::pinT ); }
    int MBoard::getAdcK() { return analogRead( MPins::pinK ); }


//==========================================

void MBoard::initPorts() {
  // ADC init
  //Sets the sample bits and read resolution, default is 12-bit (0 - 4095), range is 9 - 12 bits
  analogSetWidth(12);
  
  // Attach a pin to ADC (also clears any other analoque mode that could be on), returns TRUE/FALSE result
  adcAttachPin( MPins::pinV );
  adcAttachPin( MPins::pinI );
  #ifdef V43
      adcAttachPin( MPins::pinI3 );
  #endif
  adcAttachPin( MPins::pinT );
  adcAttachPin( MPins::pinK );

  // Set the input attenuation, default is ADC_11db, values are ADC_0db, ADC_2_5db, ADC_6db, ADC_11db
  analogSetPinAttenuation( MPins::pinV, ADC_11db);   //ADC_11db);
  rangeV = 0;
  analogSetPinAttenuation( MPins::pinI, ADC_11db);
  #ifdef V43
    analogSetPinAttenuation( MPins::pinI3, ADC_11db);
  #endif
  analogSetPinAttenuation( MPins::pinT, ADC_11db);
  analogSetPinAttenuation( MPins::pinK, ADC_11db);

  #ifdef V22
    // Светодиоды
    pinMode( MPins::led_r_pin, OUTPUT_OPEN_DRAIN );     	// Initialize the LED pins as an outputs
    pinMode( MPins::led_g_pin, OUTPUT_OPEN_DRAIN );      // ...
    pinMode( MPins::led_b_pin, OUTPUT_OPEN_DRAIN );      // ...
    ledsOff();
  #endif

  // Зуммер
  //pinMode( buz_pin, OUTPUT );
  //buzzerOff(); 

	//	Порт включения питания
  pinMode ( MPins::pow_pin, OUTPUT );        // port IO05 pu!!!
	powOff();

	// Порт управления выходным (силовым ключем)
	pinMode( MPins::on_pin, OUTPUT );
	swOff();

	// Порт обратной полярности
	pinMode( MPins::neg_pin, INPUT_PULLUP );

	// Порт превышения тока
	pinMode( MPins::fast_off_pin, INPUT_PULLUP );

	// Порт power good
	pinMode( MPins::pow_good_pin, INPUT_PULLUP );

	// Настройка каналов связанных с работой ШИМ
	ledcSetup( ch_current, 20000, 10 );
	ledcAttachPin( MPins::pwm_i_pin, ch_current );

	ledcSetup( ch_voltage, 20000, 10 );
	ledcAttachPin( MPins::pwm_u_pin, ch_voltage );

	ledcSetup( ch_discharge, 20000, 10 );
	ledcAttachPin( MPins::pwm_dis_pin, ch_discharge );

	ledcSetup( ch_fan, 20000, 10);              // 20000, 10
	ledcAttachPin( MPins::pwm_fan_pin, ch_fan );

	// Начальная установка ШИМ каналов в 0.
	ledcWrite( ch_current, 0 );
	ledcWrite( ch_voltage, 0 );
	ledcWrite( ch_discharge, 0 );
	ledcWrite( ch_fan, 0 );
}

#ifdef V22
  void MBoard::blinkWhite() {
      if( Oled->blink() ) { ledsOn(); }
      else                { ledsOff();}
  }

  void MBoard::blinkRed() {
      if( Oled->blink() ) { ledROn(); } 
      else                { ledsOff();}
  }

  void MBoard::blinkGreen()   { if(Oled->blink()) ledGOn(); else ledsOff(); }
  void MBoard::blinkBlue()    { if(Oled->blink()) ledBOn(); else ledsOff(); }
  void MBoard::blinkYellow()  { if(Oled->blink()) {ledROn(); ledGOn();} else ledsOff(); }
#endif

// Формула предложена 
// https://github.com/G6EJD/ESP32-ADC-Accuracy-Improvement-function/blob/master/ESP32_ADC_Read_Voltage_Accurate.ino
float MBoard::readVoltage( int reading )
{
  // Reference voltage is 3v3 = 4095 in range 0 to 4095
  if ( reading < 1 || reading >= 4095 ) return 0;
  const float reading2 = reading  * reading;
  const float reading3 = reading2 * reading;
  return -0.000000000000016f * reading3 * reading
        + 0.000000000118171f * reading3
        - 0.000000301211691f * reading2
        + 0.001109019271794f * reading
        + 0.034143524634089f;
}

float MBoard::readVoltage0db( int ivalue ) {
  return -0.00000001f * ivalue * ivalue   //(float)pow( ivalue, 2 )
        + 0.00191662f * ivalue        //(float)ivalue
        + 0.58680987f;
}

void MBoard::calcVoltage( float volt ) {
    float fvalue = ( volt * voltageMultiplier + voltageOffset ) - abs( current ) * wiresResistance;
    if( fvalue <= 0.8f ) { fvalue = 0.0f; }
    voltage = fvalue;
}

void MBoard::calcCurrent( int ivalue ) {
#ifndef V22    
    current = ( readVoltage( ivalue ) * acs712Multiplier + acs712Offset ) * currentMultiplier + currentOffset;
#endif
#ifdef V43
    current = -( readVoltage( ivalue ) * acs712Multiplier + acs712Offset ) * currentMultiplier + currentOffset;
 //  Serial.println( current );
#endif
}
#ifdef V43
    void MBoard::calcCurrentI3( int ivalue )    // вход с шунта
    {
      // квадратичная аппроксимация  y=−0.0000000993x 2 +0.0055366734x+−0.0541871439 
      // current3 = readVoltage( ivalue ) * readVoltage( ivalue ) * -0.0000000993 + readVoltage( ivalue ) * 0.0055366734 - 0.0541871439;
    //Serial.print("s_shunta= "); Serial.println( ivalue );

        current3 = ivalue;  // TEST (без вычисления)
        //Serial.println( current3 );
    }
#endif

void MBoard::setVoltage(float _voltage) { voltage = _voltage; }
void MBoard::setCurrent(float _current) { current = _current; }

float MBoard::getVoltage() { return voltage; }
float MBoard::getCurrent() { return current; }

void  MBoard::setOffsetU(float u) { offsetU = u; }
float MBoard::getOffsetU() { return offsetU; }

void  MBoard::setOffsetI(float i) { offsetI = i; }
float MBoard::getOffsetI() { return offsetI; }

void  MBoard::setWiresResistance(float r) { wiresResistance = r; }
float MBoard::getWiresResistance() { return wiresResistance; }

void MBoard::setVoltageVolt( float volts ) { ledcWrite( ch_voltage, volts  * 52.8f ); }        // Подбором для 14,5В (32S) 20180916
//void MBoard::setVoltageVolt( float volts ) { ledcWrite( ch_voltage, 514 ); }        // Test V43

void MBoard::setDischargeAmp( float ampers ) { ledcWrite( ch_discharge, ampers * 300.0f ); }   // от 0,3 до 3,4А

void MBoard::setCurrentPwm( int pwm )   { ledcWrite( ch_current,   pwm ); }
void MBoard::setVoltagePwm( int pwm )   { ledcWrite( ch_voltage,   pwm ); }
void MBoard::setDischargePwm( int pwm ) { ledcWrite( ch_discharge, pwm ); }




// Линейная регрессия       y=58.39108942x+5.36189996                    0.5%
// Квадратичная регрессия   y=−0.02797536x^2 +58.56353881x+5.21770470    0.4%
uint16_t MBoard::pwm( float ampers ) 
{ 
    #ifdef v43    // +38 - компенсация смещения для измерителя INORM3
        uint16_t iPvm = (uint16_t)( ampers * ampers * -0.02797536f + 58.56353881f * ampers + 5.21770470f  + 38);
    
//        uint16_t iPvm = 335 ; //  Test V43
    #else
        uint16_t iPvm = (uint16_t)( ampers * ampers * -0.02797536f + 58.56353881f * ampers + 5.21770470f );
    #endif
    return iPvm; 
}

void MBoard::setCurrentAmp( float ampers ) 
{ 
    ledcWrite( ch_current, pwm( ampers ) ); 
} 

// ====================== KEYS =============================





// ============================= cool ===========================
/*
// // MF52AT MF52 B 3950 NTC термистор 2% 10 kOm
// const float reference_resistance = 10000.0;    // R13 10kOm 1%
// const float nominal_resistance   = 10000.0;    //     10kOm 2%
// const float nominal_temperature  =    25.0;
// const float b_value              =  3950.0;
float celsius = 25.0f;

// Fan parameters
const float fan_pwm_max = 800.0f;
const float fan_pwm_min = 50.0f;
int fanPwm = fan_pwm_max;

// PID
float coolSetPoint;
float coolInput;
float coolOutput;
//const float cool_sample_time = 1000.0;     // ms, as Task period  nu?

const float k_p   = 50.0f;
const float k_i   =  5.0f;
const float k_d   =  0.0f;

//input/output variables passed by reference, so they are updated automatically
AutoPID coolPID(&coolSetPoint, &coolInput, &coolOutput, fan_pwm_min, fan_pwm_max, k_p, k_i, k_d);

void MBoard::setFan(uint16_t val) 
{
  // If less than minimum is specified - stop
  if ( val <= fan_pwm_min ) {
    val = 0;
  }
  fanPwm = val;
  ledcWrite( ch_fan, val );
}

void MBoard::initCool() 
{
  setFan( fan_pwm_max );
  coolPID.setBangBang( 30.0f, 5.0f );
  coolSetPoint = 50.0f;   //60.0f;  //55.0;     //40.0f;    
}

void MBoard::runCool() 
{
  coolInput = celsius;
  coolPID.run();
  setFan((int)coolOutput);
}

void  MBoard::setCelsius( float fvalue ) { celsius = fvalue; }
float MBoard::getCelsius() { return celsius; }

int MBoard::getFanPwm() { return fanPwm; }

*/

void MBoard::setRangeV(int range) { rangeV = range; }
int  MBoard::getRangeV() { return rangeV; }


