#ifndef _MOVERSEER_H_
#define _MOVERSEER_H_

#include "stdint.h"

class MBoard;
class AutoPID;

class MOverseer
{
public:
    MOverseer(MBoard * Board);
    ~MOverseer();

    void  runCool();

    void  setFan(uint16_t val);    

    void  setCelsius(float val);
    float getCelsius() const;
    int   getFanPwm()  const;

private:
    MBoard  * Board   = nullptr;
    AutoPID * coolPID = nullptr;
  
    float celsius = 25.0f;

    // Fan parameters
    const float fan_pwm_max = 800.0f;
    const float fan_pwm_min = 50.0f;
    int         fanPwm = fan_pwm_max;

    // PID
    float coolSetPoint;
    float coolInput;
    float coolOutput;
    //const float cool_sample_time = 1000.0;     // ms, as Task period  nu?

    const float k_p   = 50.0f;
    const float k_i   =  5.0f;
    const float k_d   =  0.0f;

    // // MF52AT MF52 B 3950 NTC термистор 2% 10 kOm
    // const float reference_resistance = 10000.0;    // R13 10kOm 1%
    // const float nominal_resistance   = 10000.0;    //     10kOm 2%
    // const float nominal_temperature  =    25.0;
    // const float b_value              =  3950.0;
};

#endif // _MOVERSEER_H_