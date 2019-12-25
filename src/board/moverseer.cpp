#include "board/moverseer.h"
#include "board/mboard.h"
#include <AutoPID.h>
#include <Arduino.h>

MOverseer::MOverseer(MBoard * Board) : Board(Board)
{
    coolPID = new AutoPID(&coolSetPoint, &coolInput, &coolOutput, fan_pwm_min, fan_pwm_max, k_p, k_i, k_d);
    
    setFan( fan_pwm_max );
    coolPID->setBangBang( 30.0f, 5.0f );
    coolSetPoint = 50.0f;   //60.0f;  //55.0;     //40.0f;    
}
//AutoPID coolPID(&coolSetPoint, &coolInput, &coolOutput, fan_pwm_min, fan_pwm_max, k_p, k_i, k_d);

MOverseer::~MOverseer()
{
    delete coolPID;
}

void MOverseer::setFan(uint16_t val) 
{
    // If less than minimum is specified - stop
    if ( val <= fan_pwm_min )
        val = 0;

    fanPwm = val;
    ledcWrite( MBoard::ch_fan, val );
}

void MOverseer::runCool() 
{
    coolInput = celsius;
    coolPID->run();
    setFan(coolOutput);
}

void  MOverseer::setCelsius(float value) {celsius = value;}
float MOverseer::getCelsius() const {return celsius;}
int   MOverseer::getFanPwm()  const {return fanPwm;}
