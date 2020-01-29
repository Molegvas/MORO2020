/*


*/

#include "mtools.h"
#include "board/mboard.h"
#include "measure/mkeyboard.h"
#include "display/moled.h"
#include <Preferences.h>
#include "AutoPID.h"            // 201905 double ---> float
#include "Arduino.h"


MTools::MTools(MBoard * board, MOled * oled) : 
    Board(board),
    Oled(oled),
    Keyboard(new MKeyboard),
    qPreferences(new Preferences) {}

MTools::~MTools()
{
    delete qPreferences;
    delete Keyboard;
}

  //analog:
  //MTools::MTools(MBoard * board, MDisplay * display, MKeyboard * keyboard) 
  //{
  //      Board = board;
  //      Display = display; 
  //      Keyboard = keyboard;
  //}

// Доступ к переменным MTools private

bool  MTools::getAP() { return false; }

float MTools::getVoltageNom() const { return voltageNom; }
void  MTools::setVoltageNom(float _voltageNom) { voltageNom = _voltageNom; }

//float MTools::getCapacityNom() { return capacityNom; }
//void  MTools::setCapacityNom(float _capacityNom) { capacityNom = _capacityNom; }

    // Индекс массива выбора батареи    
int  MTools::getAkbInd() { return akbInd; }
void MTools::setAkbInd(int _akbInd) { akbInd = _akbInd; }

float MTools::getCapacity() { return capacity; }
void  MTools::setCapacity(float _capacity) { capacity = _capacity; }

float MTools::getVoltageMax() { return voltageMax; }
void  MTools::setVoltageMax(float _voltageMax) { voltageMax = _voltageMax; }

float MTools::getVoltageMin() { return voltageMin; }
void  MTools::setVoltageMin(float _voltageMin) { voltageMin = _voltageMin; }

float MTools::getCurrentMax() { return currentMax; }
void  MTools::setCurrentMax(float _currentMax) { currentMax = _currentMax; }

float MTools::getCurrentMin() { return currentMin; }
void  MTools::setCurrentMin(float _currentMin) { currentMin = _currentMin; }

float MTools::getVoltagePre() { return voltagePre; }
void  MTools::setVoltagePre(float _voltagePre) { voltagePre = _voltagePre; } 

float MTools::getCurrentPre() { return currentPre; }
void  MTools::setCurrentPre(float _currentPre) { currentPre = _currentPre; }

float MTools::getVoltageDis() { return voltageDis; }
void  MTools::setVoltageDis(float _voltageDis) { voltageDis = _voltageDis; } 

float MTools::getCurrentDis() { return currentDis; }
void  MTools::setCurrentDis(float _currentDis) { currentDis = _currentDis; }

float MTools::getCollectAvr() { return collectAvr; }
void  MTools::setCollectAvr(float _collectAvr) { collectAvr = _collectAvr; }

float MTools::getCurrentAvr() { return currentAvr; }
void  MTools::setCurrentAvr(float _currentAvr) { currentAvr = _currentAvr; }

bool  MTools::getKeepCurr() { return keepCurr; }
void  MTools::setKeepCurr(bool _keepCurr) { keepCurr = _keepCurr; }    

bool  MTools::getToKeepVolt() { return toKeepVolt; }
void  MTools::setToKeepVolt(bool _toKeepVolt) { toKeepVolt = _toKeepVolt; }    

float MTools::getDurationOn() { return durationOn; }
void  MTools::setDurationOn(float _durationOn) { durationOn = _durationOn; }

float MTools::getDurationOff() { return durationOff; }
void  MTools::setDurationOff(float _durationOff) { durationOff = _durationOff; }

int   MTools::getNumCycles() { return numCycles; }
void  MTools::setNumCycles(int _numCycles) { numCycles = _numCycles; }

int   MTools::getPause() { return pause; }
void  MTools::setPause(int _pause) { pause = _pause; }

int   MTools::getCycle() { return cycle; }                              // pulse...
void  MTools::setCycle(int _cycle) { cycle = _cycle; }
void  MTools::decCycle() { cycle--; } 

int   MTools::getCycles() { return cycles; }                            // pulse...
void  MTools::setCycles(int _cycles) { cycles = _cycles; }




void MTools::zeroAhCounter() { timeCounter = 0; ahCharge = 0.0; }      // Обнуление счетчика ампер-часов заряда

int   MTools::getChargeTimeCounter() { return chargeTimeCounter; }
float MTools::getAhCharge()    { return ahCharge; }

void MTools::chargeCalculations()
{
    timeCounter++;
    chargeTimeCounter = ((int)timeCounter / 10);
    ahCharge += Board->getCurrent() / 36000.0;     //    ((float)( 1000 / simpleChargerPeriod ) * 3600.0);
}

// void MTools::disChargeCalculations() {
//     timeCounter++;
//     chargeTimeCounter = ((int)timeCounter / 10);
//     ahCharge -= currentDis / 7200.0;     //    ((float)( 1000 / simpleChargerPeriod ) * 3600.0);
// }



//Preferences qPreferences;

int MTools::readNvsInt(const char * name, const char * key, const int defaultValue )
{
    qPreferences->begin(name, true);                    // RW-mode (second parameter has to be false).
    int val = qPreferences->getInt(key, defaultValue);
    qPreferences->end();                                // Close the Preferences
    return val;  
}

float MTools::readNvsFloat(const char * name, const char * key, const float defaultValue )
{
    qPreferences->begin(name, true);
    float val = qPreferences->getFloat(key, defaultValue);
    qPreferences->end();
    return val;
}

void MTools::writeNvsInt(const char * name, const char * key, const int iValue )
{
    qPreferences->begin(name, false);
    qPreferences->putInt(key, iValue);
//    Serial.println( qPreferences.getInt( key, iValue ));      // test
    qPreferences->end();
}

void MTools::writeNvsFloat(const char * name, const char * key, const float fValue )
{
    qPreferences->begin(name, false);
    qPreferences->putFloat(key, fValue);
//    Serial.println( qPreferences.getFloat( key, fValue ));
    qPreferences->end();
}

void MTools::clearAllKeys(const char * name) {
    qPreferences->begin(name, false);
    qPreferences->clear();
    qPreferences->end();
}


//Saves 201905
void MTools::saveInt(   const char * name, const char * key, const int   ivalue ) { writeNvsInt(   name, key, ivalue ); }
void MTools::saveFloat( const char * name, const char * key, const float fvalue ) { writeNvsFloat( name, key, fvalue ); }


void MTools::showUpDn() { Oled->showLine2Text(" UP/DN, В-выбор "); }

void MTools::activateSelBat()
{
    Oled->showLine4Text("   Батарея   ");
    Oled->showLine3Akb( voltageNom, capacity );          // example: "  12В  55Ач  "
    showUpDn(); // " UP/DN, В-выбор "
}


void MTools::activateSetCapacity()
{
    Oled->showLine4Text("   Ёмкость   ");
    Oled->showLine3Capacity( capacity );
    showUpDn(); // " UP/DN, В-выбор "
}

void MTools::activateSetCurrMax( bool charger )
{
    if( charger ) Oled->showLine4Text(" Ток заряда  ");
    else          Oled->showLine4Text(" Ток не более");
    Oled->showLine3MaxI( currentMax );
    showUpDn(); // " UP/DN, В-выбор "
}

void MTools::activateSetVoltMax( bool charger )
{
    if( charger ) Oled->showLine4Text(" Заряжать до ");
    else          Oled->showLine4Text("  Напряжение ");
    Oled->showLine3MaxU( voltageMax );
    showUpDn(); // " UP/DN, В-выбор "
}

void MTools::activateSetCurrentPre()
{
Oled->showLine4Text(" Предзаряд до");
    Oled->showLine3MaxI( currentPre );
    showUpDn(); // " UP/DN, В-выбор "
}

void MTools::activateSetVoltagePre()
{
Oled->showLine4Text(" Предзаряд до");
    Oled->showLine3MaxU( voltagePre );
    showUpDn(); // " UP/DN, В-выбор "
}






void MTools::activateSetDurationOn()
{
    Oled->showLine4Text("   Заряд     ");
    Oled->showLine3Sec( durationOn );
    showUpDn(); // " UP/DN, В-выбор "
}

void MTools::activateSetDurationOff()
{
    Oled->showLine4Text("Разряд(Пауза)");
    Oled->showLine3Sec( durationOff );
    showUpDn(); // " UP/DN, В-выбор "
}

void MTools::activateSetNumCycl()
{
    Oled->showLine4Text("    Циклы    ");
    Oled->showLine3Num( numCycles );
    showUpDn(); // " UP/DN, В-выбор "
}

void MTools::activateSetCurrDis()
{
    Oled->showLine4Text(" Ток разряда ");
    Oled->showLine3MaxI( currentDis );
    showUpDn(); // " UP/DN, В-выбор "
}

void MTools::activateSetVoltDis()
{
    Oled->showLine4Text(" Разряжать до");
    Oled->showLine3MaxU( voltageDis );
    showUpDn(); // " UP/DN, В-выбор "
}

void MTools::activateSetPause()
{
    Oled->showLine4Text("    Пауза    ");
    Oled->showLine3Delay( pause );
    showUpDn(); // " UP/DN, В-выбор "
}

void MTools::activateExit(const char * s)
{
    // отключения - все в shutdownCharge()

    Oled->showLine4RealVoltage();
    Oled->showLine3RealCurrent();
    Oled->showLine2Text( s ); 
    
    Oled->showLine1Heap(ESP.getFreeHeap());
    #ifdef V22
        Board->ledsOff(); 
    #endif     
}


// ================ PID ==================                      // Всё временно!! перенести 
//pid settings and gains
float input;        // 
float setPoint;     //
//float integral;
float output;       // 
float outputMin; //= min_akb_i;        // 0.6; (min_akb_i =  0.2)
float outputMax; //= max_akb_i;        //       max_akb_i = 12.0

float outputLast;      // для быстрого выхода БП при повторном пуске

// Подобранные значения 
float k_p   = 0.13;
float k_i   = 0.10;
float k_d   = 0.04;

//input/output variables passed by reference, so they are updated automatically
AutoPID Pid(&input, &setPoint, &output, outputMin, outputMax, k_p, k_i, k_d);

//201905
void MTools::initPid( float oMin, float oMax, 
                      float p, float i, float d, 
                      float bMin, float bMax, 
                      unsigned long tStep ) {       //, 
                      //float sP ) { 
    Pid.reset();  
    Pid.setOutputRange( oMin, oMax );
    Pid.setGains( p, i, d );
    Pid.setBangBang( bMin, bMax );
    Pid.setTimeStep( tStep );
    //setPoint = sP;
}

void MTools::runPid( float _input )
{
    input = _input;
    Pid.run();
#ifdef DEBUG_PID
        printPid();

    //Serial.print( setPoint ); Serial.print(" -> "); Serial.print( input ); Serial.print(" -> "); Serial.print(Pid.getIntegral()); Serial.print(" -> "); Serial.println( output ); 
#endif    
    Board->setCurrentAmp( output );
}

void MTools::stopPid() { Pid.stop(); }
//void MTools::adjustOutputRange( float _oMin, float _oMax ) { Pid.setOutputRange( oMin, oMax ); }


void MTools::setSetPoint( float sp ) { setPoint = sp; }
void MTools::adjustIntegral( float _corr ) { Pid.setIntegral( Pid.getIntegral() + _corr ); } 
void MTools::saveIntegral(float _integral) { integral = _integral; }
void MTools::restoreIntegral() { Pid.setIntegral( integral ); }
void MTools::setOutputRange( float _outputMin, float _outputMax ) { Pid.setOutputRange( _outputMin, _outputMax ); }

void MTools::setIntegral( float _integral ) { Pid.setIntegral( _integral ); } 
float MTools::getIntegral() { return Pid.getIntegral(); }
//float MTools::getOutput() {}



bool MTools::atSetPoint(float _threshold) { return Pid.atSetPoint( _threshold ); }     // nu



//------------------PULSE------------------PULSE------------------PULSE------------------PULSE------------------PULSE------------------





// В зависимости от знака тока запускает импульс заряда или разряда заданной длительности
void  MTools::initPulse( float range ) {

    if( range > 0.0f ) {
        // Инициализация импульса заряда
        Board->setVoltageVolt( voltageMax * 1.2f );
        Board->setDischargeAmp( 0.0f );
        #ifdef PID_CHARGE
            Pid.reset();
            Pid.setOutputRange( range * 0.8f, range * 1.2f );
            Pid.setIntegral( range * 9.6f );            // Завалить передний фронт, когда напряжение уменьшено разрядом 
        #endif
    Serial.print("-----#+ range ");   Serial.println( range );

        Board->swOn();                                          // Включение коммутатора. 
        #ifdef V22
            Board->ledsGreen();                                     // Зеленый светодиод - импульс
        #endif
        cycle = cycles = (int)( durationOn * 10.0f );           // Всего циклов в импульсе заряда

        initCurrentAvr();
    }
    else if( range < 0.0f ) {
        // Инициализация импульса разряда

        Board->setCurrentAmp( 0.0f );
        Board->setVoltageVolt( 0.0f );                   //( voltageMax * 1.2f );
        Board->setDischargeAmp( -range );                  //( 0.0f - range );
    Serial.println("-----#- Discharge");
        Board->swOn();                                          // Включение коммутатора. 
        #ifdef V22
            Board->ledsYellow();                                    // Желтый светодиод - разряд
        #endif
        cycle = cycles = (int)( durationOff * 10.0f );         // Всего циклов в паузе разряда
    //    count = 0;      // Может пригодиться 
    }   
}

void MTools::initPause() {
        Board->swOff();                                         // Выключение коммутатора. 

        Board->setCurrentAmp( 0.0f );
        Board->setVoltageVolt( 0.0f );
        Board->setDischargeAmp( 0.0f );
Serial.println("-----#- Pause");
        #ifdef V22
            Board->ledsYellow();                                    // Желтый светодиод - пауза
        #endif
        cycle = cycles = (int)( durationOff * 10.0f );          // Всего циклов в паузе
}


// void MTools::upPulse(float delta) {
//     // Диапазон тока для первого импульса
//     float rangeMin = 0.0f;
//     float rangeMax = delta;

//     if( keepCurr ) {
//         // Ограничение максимума тока средним током предыдущего импульса
//         rangeMax = currentAvr * 1.4f ;
//         if( rangeMax > currentMax ) {
//             rangeMax = currentMax;  //currentMax;  // Или ограничивается
//         }
//     } 
//     else {
//         // Ток очередного импульса растет
//         rangeMax = currentAvr + delta;
//         if( rangeMax > currentMax ) {
//             rangeMax = currentAvr * 1.2;  //currentMax;  // Или ограничивается
//             keepCurr = true;
//         }
//     }
//     // Настройка ПИД-регулятора
// Serial.print("========================================Pulse = ");   Serial.println(rangeMax);
//     Pid.reset();
//     Pid.setOutputRange( rangeMin, rangeMax );
//     Pid.setIntegral( currentAvr * 9.6 );            // Завалить передний фронт, когда напряжение уменьшено разрядом 
// }


// Управление регулятором и накопление отсчетов тока 
void  MTools::runPulse() {
    //collectAvr += Board->getCurrent();  
    //count++;
        // Накопление отсчетов тока
        addCollectAvr( Board->getCurrent() );
    cycle--;

    //... runPidVoltage()
    input = Board->getVoltage();                    // Текущее значение
    #ifdef DEBUG_PID
        printPid();
    #endif
    Pid.run();
    Board->setVoltageVolt( output );
}

// Прекращение импульса и подсчет среднего значения тока
float MTools::stopPulse() { 

    Board->swOff();                                      // Выключение коммутатора. 

    Board->setCurrentAmp( 0.0f );
    Board->setVoltageVolt( 0.0f );
    Board->setDischargeAmp( 0.0f );

  Serial.print("#+00 ");    Serial.println( count );
    if( count == 0 ) { return 0.0f; }
    else { 
  Serial.print("#+01 ");    Serial.println( collectAvr / (float)count );

        return collectAvr / (float)count; 
    }
}

//------------------PULSE------------------PULSE------------------PULSE------------------PULSE------------------PULSE------------------
















//==================== common ==========================

void MTools::incCurrentOffset( float delta, bool way )
{
    Board->currentOffset = incfValue( Board->currentOffset, curr_offset_l, curr_offset_h, delta, way );
    //Oled->showLine3MaxI( Board->currentOffset );
    //Oled->showLine4RealVoltage();
    Oled->showLine3RealCurrent();
}
void MTools::decCurrentOffset( float delta, bool way )
{
    Board->currentOffset = decfValue( Board->currentOffset, curr_offset_l, curr_offset_h, delta, way );
    //Oled->showLine3MaxI( Board->currentOffset );
    Oled->showLine3RealCurrent();
}

void MTools::incVoltageOffset( float delta, bool way )
{
    Board->voltageOffset = incfValue( Board->voltageOffset, volt_offset_l, volt_offset_h, delta, way );
    Oled->showLine4RealVoltage();
}
void MTools::decVoltageOffset( float delta, bool way )
{
    Board->voltageOffset = decfValue( Board->voltageOffset, volt_offset_l, volt_offset_h, delta, way );
    Oled->showLine4RealVoltage();
}



void MTools::incBattery()
{
    if(akbInd == 0 ) { akbInd = number_of_batteries - 1; }
    else { akbInd--; } 
    voltageNom  = akb[akbInd][0];
    capacity = akb[akbInd][1];
    Oled->showLine3Akb( voltageNom, capacity );
}

void MTools::decBattery()
{
    if(akbInd == number_of_batteries - 1 ) { akbInd = 0; }
    else { akbInd++; } 
    voltageNom  = akb[akbInd][0];
    capacity = akb[akbInd][1];
    Oled->showLine3Akb( voltageNom, capacity );
}

void MTools::incCapacity( float delta, bool way )
{
    capacity = incfValue( capacity, capacity_l, capacity_h, delta, way );
    Oled->showLine3Capacity( capacity ); 
}
void MTools::decCapacity( float delta, bool way )
{
    capacity = decfValue( capacity, capacity_l, capacity_h, delta, way );
    Oled->showLine3Capacity( capacity ); 
}

void MTools::incCurrentMax( float delta, bool way )
{
    currentMax = incfValue( currentMax, curr_max_l, curr_max_h, delta, way );
//    Oled->showLine3MaxI( currentMax );
}
void MTools::decCurrentMax( float delta, bool way )
{
    currentMax = decfValue( currentMax, curr_max_l, curr_max_h, delta, way );
//    Oled->showLine3MaxI( currentMax );
}

void MTools::incCurrentMin( float delta, bool way )
{
    currentMin = incfValue( currentMin, curr_max_l, curr_max_h, delta, way );   // пределы уточнить
    Oled->showLine3MaxI( currentMin );
}
void MTools::decCurrentMin( float delta, bool way )
{
    currentMin = decfValue( currentMin, curr_max_l, curr_max_h, delta, way );   // пределы уточнить
    Oled->showLine3MaxI( currentMin );
}

// void MTools::incCurrentDis( float delta, bool way )
// {
//     currentDis = incfValue( currentDis, curr_dis_l, curr_dis_h, delta, way );   // пределы уточнить
//     Oled->showLine3MaxI( currentDis );
// }
// void MTools::deccurrentDis( float delta, bool way )
// {
//     currentDis = decfValue( currentDis, curr_dis_l, curr_dis_h, delta, way );   // пределы уточнить
//     Oled->showLine3MaxI( currentDis );
// }




void MTools::incVoltageMax( float delta, bool way )
{
    voltageMax = incfValue( voltageMax, volt_end_l, volt_end_h, delta, way );
    Oled->showLine3MaxU( voltageMax );
}
void MTools::decVoltageMax( float delta, bool way )
{
    voltageMax = decfValue( voltageMax, volt_end_l, volt_end_h, delta, way );
    Oled->showLine3MaxU(voltageMax);
}

void MTools::incVoltageMin( float delta, bool way )
{
    voltageMin = incfValue( voltageMin, volt_end_l, volt_end_h, delta, way );   // пределы уточнить
    Oled->showLine3MaxU( voltageMin );
}
void MTools::decVoltageMin( float delta, bool way )
{
    voltageMin = decfValue( voltageMin, volt_end_l, volt_end_h, delta, way );   // пределы уточнить
    Oled->showLine3MaxU(voltageMin);
}

void MTools::incDurationOn( bool way )
{
    durationOn = incfValue ( durationOn, duration_on_l, duration_on_h, 0.5, way );
    Oled->showLine3Sec(durationOn); 
}
void MTools::decDurationOn( bool way )
{
    durationOn = decfValue ( durationOn, duration_on_l, duration_on_h, 0.5, way );
    Oled->showLine3Sec(durationOn); 
}

void MTools::incDurationOff( bool way)
{
    durationOff = incfValue ( durationOff, duration_off_l, duration_off_h, 0.5, way );
    Oled->showLine3Sec(durationOff); 
}
void MTools::decDurationOff( bool way )
{
    durationOff = decfValue ( durationOff, duration_off_l, duration_off_h, 0.5, way );
    Oled->showLine3Sec(durationOff); 
}

void MTools::incPostpone( int delta )
{
    postpone = inciValue( postpone, postpone_l, postpone_h, delta );
    Oled->showLine3Delay( postpone );
}
void MTools::decPostpone( int delta )
{
    postpone = deciValue( postpone, postpone_l, postpone_h, delta );
    Oled->showLine3Delay( postpone );
}

void MTools::incVoltagePre( float delta, bool way )
{
    voltagePre = incfValue( voltagePre, volt_pre_l, volt_pre_h, delta, way );
    Oled->showLine3MaxU( voltagePre );
}
void MTools::decVoltagePre( float delta, bool way )
{
    voltagePre = decfValue( voltagePre, volt_pre_l, volt_pre_h, delta, way );
    Oled->showLine3MaxU( voltagePre );
}

void MTools::incCurrentPre( float delta, bool way )
{
    currentPre = incfValue( currentPre, curr_pre_l, curr_pre_h, delta, way );
    Oled->showLine3MaxI( currentPre );
}
void MTools::decCurrentPre( float delta, bool way )
{
    currentPre = decfValue( currentPre, curr_pre_l, curr_pre_h, delta, way );
    Oled->showLine3MaxI( currentPre );
}



void MTools::incVoltagePow( float delta, bool way )
{
    voltageMax = incfValue( voltageMax, volt_l, volt_h, delta, way );
    Oled->showLine3MaxU( voltageMax );
}
void MTools::decVoltagePow( float delta, bool way )
{
    voltageMax = decfValue( voltageMax, volt_l, volt_h, delta, way );
    Oled->showLine3MaxU(voltageMax);
}

void MTools::incCycles()
{
    numCycles = inciValue ( numCycles, num_cycl_l, num_cycl_h, 1 );
    Oled->showLine3Num(numCycles); 
}
void MTools::decCycles()
{
    numCycles = deciValue ( numCycles, num_cycl_l, num_cycl_h, 1 );
    Oled->showLine3Num(numCycles); 
}

void MTools::incCurrentDis( float delta, bool way )
{
    currentDis = incfValue( currentDis, curr_dis_l, curr_dis_h, delta, way );
    Oled->showLine3MaxI( currentDis );
}
void MTools::decCurrentDis( float delta, bool way )
{
    currentDis = decfValue( currentDis, curr_dis_l, curr_dis_h, delta, way );
    Oled->showLine3MaxI( currentDis );
}

void MTools::incVoltageDis( float delta, bool way )
{
    voltageDis = incfValue( voltageDis, volt_min_l, volt_min_h, delta, way );
    Oled->showLine3MaxU( voltageDis );
}
void MTools::decVoltageDis( float delta, bool way )
{
    voltageDis = decfValue( voltageDis, volt_min_l, volt_min_h, delta, way );
    Oled->showLine3MaxU(voltageDis);
}

void MTools::incPause()
{
    pause = inciValue( pause, pause_l, pause_h, 1 ); 
    Oled->showLine3Delay( pause ); 
}
void MTools::decPause()
{
    pause = deciValue( pause, pause_l, pause_h, 1 ); 
    Oled->showLine3Delay( pause ); 
}

int MTools::incNum( int v, int h, int d )
{
    v += d;
    if (v >= h) return h;
    else        return v;
}
int MTools::decNum( int v, int l, int d )
{
    v -= d;
    if (v <= l) return l;
    else        return v;
}

// inc, dec 201905
float MTools::incFloatValue( float value,  float value_l, float value_h, float delta ) {
    if( value >= value_h - delta ) return value_h;
    else return value += delta;
}

float MTools::decFloatValue( float value,  float value_l, float value_h, float delta ) {
    if( value <= value_l + delta ) return value_l;
    else return value -= delta;
}

int MTools::incIntValue( int value, int value_l, int value_h, int delta ) {
    if( value >= value_h - delta ) return value_h;
    else return value += delta;
}

int MTools::decIntValue( int value, int value_l, int value_h, int delta ) {
    if( value <= value_l + delta ) return value_l;
    else return value -= delta;
}



void MTools::clrTimeCounter() { timeCounter = 0; }
void MTools::clrChargeTimeCounter() { chargeTimeCounter = 0; }
void MTools::clrAhCharge() { ahCharge = 0; }
int  MTools::getTimeCounter() { return timeCounter; }
void MTools::setTimeCounter( int ivalue ) { timeCounter = ivalue; }


//================= Power =========================================

void MTools::activateSelectUI()
{
    // Нажата кнопка Р - предлагается выбрать параметры из табличных
    Oled->showLine4Text("  Выбор U&I  ");
                                //  line3 - без изменения
    Oled->showLine2Text(" UP/DN, В-выбор ");    // Подсказка: UP/DN - листать список, В - выбрать
    
    #ifdef DEBUG_POWER
        Serial.println("Power: Select U&I");
    #endif
}

            //Tools->writeNvsInt("s-power", "powInd", Tools->powInd );  // Выбор заносится в энергонезависимую память
void MTools::savePowInd(const char * name) { writeNvsInt( name, "powInd", powInd ); }
//void MTools::savePowU(const char * name) { writeNvsInt( name, "voltMax", voltageMax ); }
//void MTools::savePowI(const char * name) { writeNvsInt( name, "currMax", currentMax ); }
void MTools::savePowO(const char * name) { writeNvsInt( name, "powO", powO ); }



void MTools::upPow()
{
    if(powInd == 0 ) { powInd = number_of_powers - 1; }
    else { powInd--; } 
    voltageMax = pows[powInd][0];
    currentMax = pows[powInd][1];
//    Oled->showLine3Power( voltageMax, currentMax );
}

void MTools::dnPow() 
{
    if(powInd == number_of_powers - 1 ) { powInd = 0; }
    else { powInd++; } 
    voltageMax = pows[powInd][0];
    currentMax = pows[powInd][1];
//    Oled->showLine3Power( voltageMax, currentMax );
}


void MTools::liveU()
{
    setPoint = voltageMax;
}


void MTools::activatePowerPh1( float volt, float amp )
{
// Pid
    setPoint = volt;
    input = Board->getVoltage();            // перестраховка?
  Pid.reset();  
  Pid.setIntegral( (volt - 0.3) * 10.0 );
    Pid.setOutputRange(0.0, volt + 0.6);                        //( 0.6, setPoint + 0.5 );        // уточнить
    Pid.setBangBang  (20.0);                 //( 0.25, 0.25 );
    Pid.setTimeStep(500);

// Обнуляются счетчики времени и количества отданного заряда
    timeCounter = 0;
    ahCharge = 0.0;

    // Задаются начальные напряжение и ток
    Board->setVoltageVolt( volt - 0.2 );   //volt - 0.2 );            // с некоторым запасом на погрешность задания
    Board->setCurrentAmp( amp + 0.2 );                    // Ток сначала будет ограничен ??

// Включение преобразователя и коммутатора. 
    Board->powOn();
    Board->swOn();

    #ifdef V22
        // Зеленый светодиод - процесс заряда запущен
        Board->ledsOff(); Board->ledGOn();
    #endif

    // Задание отображения на экране дисплея построчно (4-я строка - верхняя)
    Oled->showLine4RealVoltage();
    Oled->showLine3RealCurrent();
    Oled->showLine2Text("   Запитано...  ");        // " Время:  Заряд: "
    Oled->showLine1Time( chargeTimeCounter );
    Oled->showLine1Ah( ahCharge );

    // Simple power supply started
    #ifdef DEBUG_SIMPLE_POWER
        Serial.println(" Источник питания стартовал с параметрами:");
        Serial.print("  Voltage max, B  *** = ");  Serial.println(setPoint + 0.5, 2);
        Serial.print("  Current max, A  *** = ");  Serial.println(setPoint);
    #endif

}

void MTools::runPidVoltage() {
    input = Board->getVoltage();                    // Текущее значение
    #ifdef DEBUG_PID
        printPid();
    #endif
    Pid.run();
    //Board->setCurrentAmp(output);
    Board->setVoltageVolt( output );
}

void MTools::printPid() {
    Serial.print( input );              Serial.print("\t");
    Serial.print( setPoint );           Serial.print("\t");
    Serial.print( Pid.getIntegral() );  Serial.print("\t");
    Serial.println( output );
}



//================= Charger =========================================

void MTools::activateChargeStart()
{
    Oled->showLine4Text("   Зарядное  ");
    Oled->showLine3Akb( voltageNom, capacity );              // example: "  12В  55Ач  "
    Oled->showLine2Text(" P-корр.С-старт ");        // Активны две кнопки: P-сменить настройки, и C-старт
    Oled->showLine1Time(0);                         // уточнить
    Oled->showLine1Ah(0.0);                         // уточнить
    #ifdef V22
        Board->ledsOn();                                // Светодиод светится белым до старта заряда - режим выбран
    #endif
}




void MTools::powShutdown() 
{
    //Pid.setIntegral( 0.0 );   
    output = 0.0;
    Board->setCurrentAmp( 0.0 );
    Board->setVoltageVolt( 0.0 );

    Board->powOff();
    Board->swOff();
    #ifdef V22
        Board->ledsOff();
        Board->ledROn();
    #endif
    Oled->showLine2Text(" Время: Отдано: ");
    // Oled->showLine1Time( chargeTimeCounter );
    // Oled->showLine1Ah( ahCharge );
}



// // ================ PID ==================
// //pid settings and gains
// float input;        // 
// float setPoint;     //
// float output;       // 
// float outputMin; //= min_akb_i;        // 0.6; (min_akb_i =  0.2)
// float outputMax; //= max_akb_i;        //       max_akb_i = 12.0

// // Подобранные значения 
// float k_p   = 0.13;
// float k_i   = 0.10;
// float k_d   = 0.04;

// //input/output variables passed by reference, so they are updated automatically
// AutoPID Pid(&input, &setPoint, &output, outputMin, outputMax, k_p, k_i, k_d);

void MTools::activatePrecharge( float volt, float amp, bool impuls )
{
    if( impuls ) 
    {
        // Активация импульсного предзаряда

    //    Serial.println(volt);
    //    Serial.println(amp);

        activateCharge( volt, amp );
        cycle = durationOn * 2;

        Oled->showLine2Text(" Имп. предзаряд ");

    }
    else 
    {
        // Активация предзаряда постоянным током
        Board->setDischargeAmp( 0.0 );

        if( ( amp + 0.2 ) > max_akb_i) amp = max_akb_i; // Поправка на погрешность установки ограничения тока
        setPoint = amp;                                 // Ток будет стремиться к этому значению

    // Настройка ПИД-регулятора
    Pid.stop();
    Pid.setOutputRange( 0.0, amp + 0.6 ); // + 0.2 );           // 
    Pid.setBangBang(20.0);                          // отключение min/max
    Pid.setTimeStep(100);   //(1000);                          // Подобрать
output = 0.0;
    // Обнуляются счетчики времени и количества отданного заряда
    //    timeCounter = 0;
    //    ahCharge = 0.0;

        // Задается выходное напряжение
        Board->setVoltageVolt( volt + 0.6 );            // с некоторым запасом на погрешность задания 12.0 * 1.2 + 0.6 = 15.0
        Board->setCurrentAmp( 0.0 );                    // Ток в начале будет ограничен

        // Включение преобразователя и коммутатора. 
        Board->powOn();
        Board->swOn();
        #ifdef V22
            // Зеленый светодиод - процесс заряда запущен
            Board->ledsOff(); Board->ledGOn();
        #endif
    // Задание отображения на экране дисплея построчно (4-я строка - верхняя)
        Oled->showLine4RealVoltage();
        Oled->showLine3RealCurrent();
        Oled->showLine2Text(" Пост. предзаряд");        // " Время:  Заряд: "
        Oled->showLine1Time( chargeTimeCounter );
        Oled->showLine1Ah( ahCharge );
    }
}



// Активация процесса заряда (Первая фаза)
void MTools::activateChargePh1( float volt, float amp, bool recharge )
{   
    activateCharge( volt, amp );
    Oled->showLine2Text(" Заряжается...  ");        // " Время:  Заряд: "

    if( recharge )                  // Будет исполняться chRunPh2()
    {
        setPoint = 14.2;
    }
}





// новые, с честным подъёмом тока ( ПИД - вспомогательная роль)

bool MTools::postponeCalculation()
{
    timeCounter--;
    chargeTimeCounter = timeCounter / 10;   //2;
    if( chargeTimeCounter == 0 )
    {
        return true;
    }
    return false;
}


void MTools::activatePostpone( float volt, float amp )
{
    // volt - напряжение окончания предзаряда, не более того
    // Возможны 2 варианта старта - "быстрый" и по профилю
    voltagePre = volt;
    currentPre = amp;

    timeCounter = postpone * 7200;                      // Отложенный старт в полусекундах

    toKeepVolt = false;                                 // Признак удержания напряжения снять

    // if( ( amp + 0.2 ) > max_akb_i) amp = max_akb_i;  // Поправка на погрешность установки ограничения тока
    setPoint = amp;                                     // Ток будет стремиться к этому значению

    // Настройка ПИД-регулятора
    Pid.stop();
    Pid.setOutputRange( 0.0, amp + 0.6 );               // Пределы регулирования тока
    Pid.setBangBang(20.0);                              // отключение min/max
    Pid.setTimeStep(100);   //(1000);                          // Подобрать
    output = 0.0;

    // Задается выходное напряжение и начальный ток
    Board->setVoltageVolt( voltageNom * 1.234 + 0.6 );  // с некоторым запасом на погрешность задания
    Board->setCurrentAmp( 0.0 );                        // Ток в начале будет ограничен

    Oled->showLine4RealVoltage();
    Oled->showLine3RealCurrent();
    Oled->showLine2Text(" До старта...   ");
}

void MTools::activatePreliminaryCharge()
{
    timeCounter = postpone * 7200;  // Отложенный старт в полусекундах
    Board->setDischargeAmp( 0.0 );

    // Обнуляются счетчики времени и количества отданного заряда
    timeCounter = 0;
    ahCharge = 0.0;

    // Включение преобразователя и коммутатора. 
    Board->powOn();
    Board->swOn();
    #ifdef V22
        // Зеленый светодиод - процесс заряда запущен
        Board->ledsOff(); Board->ledGOn();
    #endif
    // Задание отображения на экране дисплея построчно (4-я строка - верхняя)
    Oled->showLine4RealVoltage();
    Oled->showLine3RealCurrent();
    Oled->showLine2Text(" Заряжается...  ");        // " Время:  Заряд: "
    Oled->showLine1Time( chargeTimeCounter );
    Oled->showLine1Ah( ahCharge );

    setPoint = 0.2;
    Board->setCurrentAmp( setPoint );
}



// импульсный заряд, активация импульса заряда
void MTools::activateImpuls( float volt, float amp, float delta ) //***************************************************
{

//    Serial.println("#активация импульса заряда#");
//    Serial.print(" setPoint = ");    Serial.println(setPoint);

    // Отключение разряда 
    Board->setDischargeAmp( 0.0 );

    Pid.stop();         // Тестовый вариант

    setPoint += delta;
    if( setPoint > currentMax ) setPoint = currentMax; 
    Serial.print(" new setPoint = ");    Serial.println(setPoint);

    Board->setVoltageVolt( volt + 0.0 );                // с некоторым запасом на погрешность задания
//    Serial.print(" setVoltage = ");    Serial.println( volt );

    Board->setCurrentAmp( setPoint );                    // Ток в начале импульса не калиброван

    Board->powOn();
    Board->swOn();

    // Настройка ПИД-регулятора
    Pid.reset();                                 // Проверить
    Pid.setBangBang(20.0);                          // отключение min/max
    Pid.setTimeStep(100);                          // Подобрать    Pid.setTimeStep(500);

    Pid.setOutputRange( setPoint * 0.8 , setPoint * 1.2 );  // Нижний предел сдвигается для ускорения выхода на вершину импульса
 // Pid.setIntegral( setPoint * 10.0 );   // 0.8 );
    input = setPoint;
//    Serial.print(" input  = ");    Serial.println( input );

    Pid.run();
//    Serial.print(" output = ");    Serial.println( output );

    Pid.stop();
    //Board->setCurrentAmp( output );

//   Board->setCurrentAmp( setPoint );

//              Serial.print("First input ");   Serial.println( input );
//              Serial.print("First output ");   Serial.println( output );

  //output = 0.0;                                 // 


    // Включение преобразователя и коммутатора. 
 //   Board->swOn();
    #ifdef V22
        // Зеленый светодиод - процесс заряда запущен
        Board->ledsOff(); Board->ledGOn();
    #endif
    // Задание отображения на экране дисплея построчно (4-я строка - верхняя)
    Oled->showLine4RealVoltage();
    Oled->showLine3RealCurrent();
    Oled->showLine2Text("   Импульс...   ");        // " Время:  Заряд: "   ПОДОБРАТЬ
    Oled->showLine1Time( chargeTimeCounter );
    Oled->showLine1Ah( ahCharge );

    cycle = durationOn * 2 ;

    // Serial.println("Предзаряд закончен, активирован activateImpuls с параметрами: ");
    // Serial.print("setI = ");  Serial.println( currentMax );
    // Serial.print("setU = ");  Serial.println( voltageMax );
    // Serial.print("Stop = ");  Serial.println( getPidStopped() );

    // Serial.print("cycl = ");  Serial.println( cycle );

}







void MTools::activateSuspendCharge()   // to suspend the charge
{
//    Serial.println("#off#");
    Board->swOff();
    #ifdef V22
        Board->ledROn();     // Жёлтый светодиод - процесс заряда приостановлен
    #endif
    cycle = durationOff * 2;
}

void MTools::activateResumeCharge() //to resume the charge
{
//        Serial.println("#on#");

  //   Pid.setIntegral( integral );
  //   output = temp;
    Board->swOn();
    #ifdef V22
        Board->ledsOff(); Board->ledGOn();    // Зеленый светодиод - процесс заряда запущен
    #endif
    cycle = durationOn * 2;
}

// Работа ПИД-регулятора в первой фазе (подъём тока)
void MTools::runChargePh1()
{
//  Board->powOn();
//  Board->swOn();

 //   input = Board->getCurrent();                    // Текущее значение тока
 //   Pid.run();                                      // Сравнение текущего тока с заданной величиной и вычисление порога ограничения

  //Board->setCurrentAmp( currentMax ); // для калибровки 

    //Board->setVoltageVolt(12.0);
    //Board->setCurrentAmp( setPoint );   //output );                 // Коррекция ограничителя тока
    //    Serial.println("************************");

    input = Board->getVoltage();
    Pid.run();
    Board->setCurrentAmp( output );

    #ifdef DEBUG_CHARGE
        printAll( "UpI" );
    #endif
}

// Предзаряд постоянным током
void MTools::runPreliminaryCharge( float volt, float amp, float delta )
{
    if( Board->getVoltage() <= volt ) { setPoint += delta; }

    if( setPoint > amp ) setPoint = amp;

        //if( cycle == ( durationOn * 2.0 ) ) { input = setPoint; }
        //else                                { input = Board->getCurrent(); }    // setPoint;   // !!!
    input = Board->getCurrent();    
        Pid.run();
        Board->setCurrentAmp( output ); // ( setPoint );

    #ifdef DEBUG_CHARGE
        Serial.println("UpI:    input   setPoint integr  output  volt    curr");
        Serial.print("        ");
        Serial.print(input);                Serial.print("    ");
        Serial.print(setPoint);             Serial.print("    ");
        Serial.print(Pid.getIntegral());    Serial.print("    ");
        Serial.print(output);               Serial.print("    ");
        Serial.print(Board->getVoltage());  Serial.print("    ");
        Serial.println(Board->getCurrent());
    #endif

}


// Активация импульса заряда в фазе подъёма тока                                -- не исп в импульсном заряде
void MTools::activateImpulsCharge( float volt, float amp, float delta )
{
    Board->setCurrentAmp( 0.0 );


    // Задать пределы регулирования для импульса
    Board->setVoltageVolt( volt * 1.2 );
    if( Board->getVoltage() <= volt ) { setPoint += delta; }
    if( ( setPoint ) > amp ) setPoint = amp;

    // Настройка ПИД-регулятора
//    float saveIntegral = Pid.getIntegral();

    Pid.reset();
  Pid.setIntegral( setPoint * 10.0 );
    Pid.setOutputRange( setPoint * 0.7, setPoint * 1.1 );

    // Настроить индикацию при импульсе заряда
    Oled->showLine3Avr( currentAvr );           // За предыдущий импульс              TEST  

//    Oled->showLine3RealCurrent();
    Oled->showLine2Text(" Время:  Заряд: ");

    initCurrentAvr();                               // Очистка накопителя среднего

    // Задать длительность, пересчитав секунды в число циклов
    cycle = (int)( durationOn * 10.0 );
}




// // Активация импульса заряда в фазе подъёма тока. Вариант2: "Следим за напряжением, а ток - сколько возьмёт"
// void MTools::activateImpulsChargeVar2( float _setpoint, float delta )
// {
//     // Задать длительность импульса, пересчитав секунды в число циклов
//     cycles = (int)( durationOn * 10.0 );    // Всего циклов в импульсе заряда

//     setPoint = _setpoint * 0.96;            // Выброс напряжения возможен

//     //Serial.print("****** sp = "); Serial.println( setPoint );

//     // Диапазон тока для первого импульса
//     float rangeMin = 0.0;
//     float rangeMax = delta;

//     if( keepCurr )
//     {
//         // Ограничение максимума тока средним током предыдущего импульса
//         rangeMax = currentAvr * 1.4 ;
//         if( rangeMax > currentMax )
//         {
//             rangeMax = currentMax;  //currentMax;  // Или ограничивается
//         }
    
//     } 
//     else
//     {
//         // Ток очередного импульса растет
//         rangeMax = currentAvr + delta;
//         if( rangeMax > currentMax )
//         {
//             rangeMax = currentAvr * 1.2;  //currentMax;  // Или ограничивается
//             keepCurr = true;
//         }
//     }


    
    
// //    Serial.print("::::::::::rangeMax= "); Serial.println(rangeMax);
//     // // Задать пределы регулирования для импульса
//     // if( currentAvr + delta <= currentMax ) 
//     // {   
//     //     rangeMin = 0.0; //currentAvr;
//     //     rangeMax = currentAvr + delta;  
//     // }
//     // else
//     // {
//     //     rangeMin = 0.0; //currentAvr;
//     //     rangeMax = currentAvr;  
//     // }
    

//     // Настройка ПИД-регулятора
//     Pid.reset();
//     Pid.setOutputRange( rangeMin, rangeMax );
//     Pid.setIntegral( currentAvr * 9.6 );            // Завалить передний фронт, когда напряжение уменьшено разрядом 

//     // Настроить индикацию при импульсе заряда
//     Oled->showLine3Avr( currentAvr );               // За предыдущий импульс              TEST  

//     //Oled->showLine3RealCurrent();
//     Oled->showLine2Text(" Время:  Заряд: ");

// //-    initCurrentAvr();                               // Очистка накопителя среднего

//     cycle = cycles; //(int)( durationOn * 10.0 );
// }






// Формирование импульса заряда в фазе подъёма тока
// void MTools::runImpulsCharge()
// {

//     if( cycle == cycles )
//     {
//         // Инициализация импульса заряда
//     Board->swOn();
//         Board->setCurrentAmp( 0.0 );
//         Board->setVoltageVolt( voltageMax * 1.2 );
//         initCurrentAvr();                               // Очистка накопителя среднего
//     Pid.run();      // Пустой цикл - напряжение неверное?
//     }
//     else
//     {
//         // Накопление отсчетов тока
//         addCollectAvr( Board->getCurrent() );

//         input = Board->getVoltage();
//         Pid.run();
//         Board->setCurrentAmp( output ); 
//     }

//     #ifdef DEBUG_CHARGE
//         printAll("#+ : ");
//     #endif

//     // При превышении максимального напряжения импульс заряда завершить
//     if( Board->getVoltage() >= voltageMax ) { cycle = 1; }

//     // Окончание импульса заряда
//     if( cycle == 1 )
//     {
//         // Вычисление среднего тока за время импульса
//         currentAvr = calcCurrentAvr();
//         //Pid.stop();
//     Board->swOff();
//         Board->setVoltageVolt( 0.0 );
//         Board->setCurrentAmp( 0.0 );
//     }
// }

// void MTools::activateImpulsDischarge( float amp )
// {
//     cycles = (int)( durationOff * 10.0 );    // Всего циклов в импульсе разряда


//     Board->setVoltageVolt( 0.0 );
//     Board->setCurrentAmp( 0.0 );

//     //float disI = currentMax * 0.1;
//     Board->setDischargeAmp( amp );

//     Oled->showLine3RealCurrent();         //Oled->showLine3MaxI( -amp );
//     Oled->showLine2Text(" Время: Разряд: ");


//     cycle = cycles;    //(int)(durationOff * 10.0 );  //    cycle = (int)(durationOff * 2.0 );


// }

// Импульс разряда в фазе подъема тока
// void MTools::runImpulsDischarge( float amp )
// {
//     if( cycle >= cycles - 1 )
//     {
//         // Инициализация импульса разряда ( пустой цикл )
//         Board->swOn();
//         Board->setCurrentAmp( 0.0 );
//         Board->setVoltageVolt( 0.0);
//         Board->setDischargeAmp( 0.0 );
//     }
//     else
//     {
//         // 
//         Board->setDischargeAmp( amp );
//     }

//     // Окончание импульса - ток снимается раньше
//     if( cycle <= 2 ) 
//     { 
//         Board->swOff();
//         Board->setDischargeAmp( 0.0 ); 
//     }
// }

// ****************


// Активация импульса разряда в фазе поддержания напряжения
void MTools::activateImpulsDischarge2( float amp )
{
    Board->setVoltageVolt( 0.0 );
    Board->setCurrentAmp( 0.0 );            // на время разряда тока заряда не должно быть

    float disI = amp * 0.8;                 // ток разряда test 0.8
    Board->setDischargeAmp( disI );

            #ifdef DEBUG_CHARGE
                Serial.println("ExCharger: Imp Discharge at Keep Voltage activated");
                Serial.print("    Discharge current, A  = "); Serial.println( disI );
                Serial.print("    Discharge duration, s = "); Serial.println( durationOff );
            #endif

    Oled->showLine3RealCurrent();              //Oled->showLine3MaxI( -disI );
    Oled->showLine2Text(" Время: Разряд: ");

    cycle = (int)( durationOff * 10.0 ); //    cycle = (int)( durationOff * 2.0 );

}






void MTools::activateImpulsCharge2( float volt, float amp )
{
    Board->setVoltageVolt( volt + 0.6 );
 //   Board->setCurrentAmp( amp ); //    Board->setCurrentAmp( output );


initCurrentAvr();
    //if( Board->getVoltage() <= 14.2 ) { setPoint += delta; }

    //if( setPoint > amp ) setPoint = amp;



//        float saveIntegral = Pid.getIntegral();
                setPoint = volt;

        Pid.reset();
            Pid.setTimeStep(100);

//        Pid.setIntegral( saveIntegral );
    Pid.setIntegral( amp * 10.0 );
        Pid.setOutputRange( amp * 0.7, amp * 1.1 );
//
            Board->setCurrentAmp( amp );

    //output = setPoint;
    //Board->setCurrentAmp( output );
        //Board->setCurrentAmp( currentOld );
    // Настроить индикацию при импульсе заряда
    Oled->showLine3Avr( currentAvr );           // За предыдущий импульс              TEST  

    //Oled->showLine3RealCurrent();
    Oled->showLine2Text(" Время:  Заряд: ");

    cycle = (int)( durationOn * 10.0 );  //    cycle = (int)( durationOn * 2.0 );

}

void MTools::runImpulsCharge2()
{
    // setPoint задана 14,2 или 14,8

    // Накопление отсчетов тока, исключая первый шаг, где ток нулевой
    //if( cycle < ( (int)( durationOn * 2.0 ) ) )  { addCollectAvr( Board->getCurrent() ); }
    if( cycle < ( (int)( durationOn * 10.0 ) ) )  { addCollectAvr( Board->getCurrent() ); }

    #ifdef DEBUG_CHARGE
        printAll("@+ : ");
    #endif


    input = Board->getVoltage();        // Текущее напряжение
    Pid.run();
    Board->setCurrentAmp( output );


    // незадолго до окончания импульса (средний ток бы)
    if( cycle == 1 ) 
    {
        //Serial.print("@+: Средний ток в импульсе = ");  Serial.println( getCurrentAverage() );
        //currentOld = Board->getCurrent();  // Это может и не понадобится - ПИД должен справиться

  //currentOld = getCurrentAverage();
        currentAvr = calcCurrentAvr();

    //Serial.print(" Средний ток: "); Serial.println( currentAvr );
        //Pid.stop();

 Board->setCurrentAmp( 0.0 );
    }

}

void MTools::initCurrentAvr() { collectAvr = 0.0f;  cnt = 0; count = 0; }       // оставить один счетчик

void MTools::addCollectAvr( float amp ) { collectAvr += amp;  cnt++; count++; }       // оставить один счетчик

float MTools::calcCurrentAvr()
{
    if( cnt == 0 ) return 0.0;
    #ifdef DEBUG_CHARGE
        Serial.print("getCurrentAver : ");  
                            Serial.print( collectAvr ); Serial.print(", ");    
                            Serial.print( cnt );        Serial.print(", ");  
                            Serial.println( collectAvr / (float)cnt );
    #endif
    return collectAvr / (float)cnt;
}

void MTools::runImpDisch()
{
    if( cycle == 1 )
    {
        Board->setDischargeAmp( 0.0 );
        Board->setCurrentAmp( 0.0 );
    }
    cycle--;
}




void MTools::runImpulsDischarge2()
{

    Board->setDischargeAmp( currentAvr / 10.0 );                          //( currentOld * 0.1 );

// Serial.print("@- : "); Serial.print( cycle ); Serial.print(", средний ток в импульсе: "); Serial.println( currentAvr );

    // незадолго до окончания импульса (средний ток бы)
    if( cycle == 2 ) 
    { 
        //setPoint = Board->getCurrent(); 
        //Pid.run();

    }

}

// *****************








// Активация ПИД-регулятора для работы во второй фазе
// Подъем тока закончился, далее регулирование по напряжению
// до достижении уровня 96% от конечного 
// Ограничения тока устанавливается на достигнутом уровне (поэтому и не задаётся). 
// Светодиодная индикация - жёлтый
void MTools::activateChargePh2( float _setPoint )
{
    input = Board->getVoltage();                    // для первого шага - уточнить
    setPoint = _setPoint;                           // это 14.2;
    Pid.setOutputRange( min_akb_i, output );
    float saveIntegral = Pid.getIntegral();         // Для восстановления после перезагрузки ПИД-регулятора
    Pid.reset();
        Pid.setTimeStep(100);

//Pid.setGains( 0.52, 0.4, 0.16 );      // Добавить агрессивности
//Pid.setGains( 0.15, 0.10, 0.04 );      // 
//Pid.setGains( 0.065, 0.05, 0.02 );      // Убавить агрессивности

    Pid.setIntegral( saveIntegral );
    #ifdef V22
        Board->ledROn();                                // G + R = жёлтый
    #endif
    #ifdef DEBUG_CHARGE
        Serial.println(" Переход в фазу 2 стартовал с параметрами:");
        //Serial.print("  Voltage End, B      = ");  Serial.println(voltageMax, 2);
        Serial.print("  setPoint, V         = ");  Serial.println(setPoint);
        Serial.print("  input, V            = ");  Serial.println(input);
        Serial.print("  integral  saved     = ");  Serial.println(saveIntegral);
        Serial.print("  integral  V         = ");  Serial.println(Pid.getIntegral());
        Serial.print("  output, A   ***     = ");  Serial.println(output);
        Serial.print("  voltage, V          = ");  Serial.println(Board->getVoltage());
        Serial.print("  current, A          = ");  Serial.println(Board->getCurrent());
    #endif
    cycle = durationOn * 2;
}

// Работа ПИД-регулятора во второй фазе (подъём напряжения до 96% конечного)
void MTools::runChargePh2()
{
    input = Board->getVoltage();                    // Текущее значение напряжения
    Pid.run();                                      // Сравнение текущего напряжения с заданной величиной, вычисление уровня
    Board->setCurrentAmp( output );                 // ограничением тока 
    #ifdef DEBUG_CHARGE
        Serial.println("Ph2:    input   setPoint integr  output  volt    curr");
        Serial.print("        ");
        Serial.print(input);                Serial.print("    ");
        Serial.print(setPoint);             Serial.print("    ");
        Serial.print(Pid.getIntegral());    Serial.print("    ");
        Serial.print(output);               Serial.print("    ");
        Serial.print(Board->getVoltage());  Serial.print("    ");
        Serial.println(Board->getCurrent());
    #endif
}

// Активация ПИД-регулятора для работы в третьей фазе
// Регулирование не требуется, ПИД-регулятор останавливается
void MTools::activateChargePh3()
{
    Pid.stop();
    #ifdef DEBUG_CHARGE
        Serial.println("======== chRunPh3:=========");
        Serial.print("  setPoint      = ");  Serial.println(setPoint);
        Serial.print("  input         = ");  Serial.println(input);
        Serial.print("  integral      = ");  Serial.println(Pid.getIntegral());
        Serial.print("  output        = ");  Serial.println(output);
        Serial.print("  voltage, V    = ");  Serial.println(Board->getVoltage());
        Serial.print("  current, A    = ");  Serial.println(Board->getCurrent());
    #endif
}


void MTools::shutdownDC() {
    Board->setCurrentAmp( 0.0 );
    Board->setVoltageVolt( 0.0 );
    Board->setDischargeAmp( 0.0 );

    Board->powOff();
    Board->swOff();
    #ifdef V22
        Board->ledsOff();
    #endif
}



void MTools::shutdownCharge() 
{
    output = 0.0;
    // Board->setCurrentAmp( 0.0 );
    // Board->setVoltageVolt( 0.0 );
    // Board->setDischargeAmp( 0.0 );

    // Board->powOff();
    // Board->swOff();
    // Board->ledsOff();
    shutdownDC();
    #ifdef V22
        Board->ledROn();
    #endif
    Oled->showLine2Text(" Время:  Заряд: ");
}



// Активация любого заряда
void MTools::activateCharge( float volt, float amp )
{
    Board->setDischargeAmp( 0.0f );

   // if( ( amp + 0.2 ) > max_akb_i) amp = max_akb_i; // Поправка на погрешность установки ограничения тока
    setPoint = amp;                                 // Ток будет стремиться к этому значению

    // Настройка ПИД-регулятора
    Pid.stop();
    Pid.setOutputRange( 0.0f, amp + 0.6f ); // + 0.2 );           // 
    Pid.setBangBang(20.0f);                          // отключение min/max
    Pid.setTimeStep(100);   //(1000);                          // Подобрать
output = 0.0;
    // Обнуляются счетчики времени и количества отданного заряда
    timeCounter = 0;
    ahCharge = 0.0;

    // Задается выходное напряжение
    Board->setVoltageVolt( volt + 0.6f );            // с некоторым запасом на погрешность задания
    Board->setCurrentAmp( 0.0f );                    // Ток в начале будет ограничен

    // Включение преобразователя и коммутатора. 
    Board->powOn();
    Board->swOn();

    #ifdef V22
        // Зеленый светодиод - процесс заряда запущен
        Board->ledsOff(); Board->ledGOn();
    #endif
    // Задание отображения на экране дисплея построчно (4-я строка - верхняя)
    Oled->showLine4RealVoltage();
    Oled->showLine3RealCurrent();
//    Oled->showLine2Text(" Заряжается...  ");        // " Время:  Заряд: "
    Oled->showLine1Time( chargeTimeCounter );
    Oled->showLine1Ah( ahCharge );

}

// ========================== Service ====================================
#define DEBUG_SERVICE 1


// mb common
float MTools::incfValue( float value,  float value_l, float value_h, float delta, bool way ) 
{
    if( value >= value_h - delta )
    {
        if( way ) { return value_l; }
        else      { return value_h; }        
    } else return value += delta;
}

float MTools::decfValue( float value,  float value_l, float value_h, float delta, bool way )
{
    if( value <= value_l + delta )
    {
        if( way ) { return value_h; }
        else      { return value_l; } 
    } else return value -= delta;
}

int MTools::inciValue( int value,  int value_l, int value_h, int delta ) 
{
    if( value >= value_h ) return value_l; 
    return value += delta; 
}

int MTools::deciValue( int value,  int value_l, int value_h, int delta ) 
{
    if( value <= value_l ) return value_h; 
    return value -= delta; 
}


void MTools::activatePause()
{
    timeCounter = pause * 900;  //7200; //3600=1/2часа
    Board->setCurrentAmp( 0.0f );
    Board->setVoltageVolt( 0.0f );

    Board->powOff();
    Board->swOn();
    #ifdef V22
        Board->ledsOff();
        Board->ledROn();
        Board->ledGOn();
    #endif
    Oled->showLine2Text(" Пауза:  Заряд: ");
}

bool MTools::pauseCalculations()
{
    timeCounter--;
    chargeTimeCounter = timeCounter / 2;
    if( ( chargeTimeCounter ) == 0 ) 
    {
        #ifdef V22
            Board->ledROff();
        #endif


    //  numCycles--;
        cycle++;
        return true;
    }
    return false;
}

void MTools::activateDischarge()
{
    Board->setDischargeAmp( currentDis );

    Board->setVoltageVolt( 0.0f );   //??
    Board->powOff();    //??
    Board->swOn();
    #ifdef V22
        Board->ledsOff();
        Board->ledROn();
        Board->ledGOn();
    #endif
    Oled->showLine2Text(" Время: Разряд: ");
}


bool MTools::getPidStopped()
{
    return Pid.isStopped();
}

void MTools::printAll( const char * s )
{
    Serial.print(s); Serial.println(":  cycle   input   setPoint integr  output  volt    curr    avr");
    Serial.print("         ");
    Serial.print(cycle);                Serial.print("    ");
    Serial.print(input);                Serial.print("    ");
    Serial.print(setPoint);             Serial.print("    ");
    Serial.print(Pid.getIntegral());    Serial.print("    ");
    Serial.print(output);               Serial.print("    ");
    Serial.print(Board->getVoltage());  Serial.print("    ");
    Serial.print(Board->getCurrent());  Serial.print("    ");
    Serial.println(currentAvr);
}

//================= Measures =====================

// The function ReadVoltage(pin) uses a polynominal equation to convert ADC readings into a linear
// response when the input range is 0 - 3 volts input and gives a result with an error of < 1%
// https://www.youtube.com/watch?v=RlKMJknsNpo
// Tech Note 069 - Using the ESP32 ADC and some of its more advanced functions (by David Bird)
// Accuracy is within ~7%, but can be improved with mathematical functions.
/*
float MTools::readVoltage( float reading )
{
    if ( reading < 1 || reading >= 4095 ) return 0;

    return -0.000000000000016f * reading * reading * reading * reading       //pow(reading, 4)
          + 0.000000000118171f * reading * reading * reading       //pow(reading, 3) 
          - 0.000000301211691f * reading * reading       //pow(reading, 2) 
          + 0.001109019271794f * reading
          + 0.034143524634089f;
}
*/

#include "board/moverseer.h"
void  MTools::setCelsius(int ivalue) { Board->Overseer->setCelsius( readSteinhart( ivalue ) ); }

float MTools::readSteinhart( const int adc )
{
// https://neyasyt.ru/uploads/files/termistor-NTC-10-K-MF52.pdf
  float steinhart;
  //float tr = 4095.0 / adc - 1.0;

//  float tr = 3.30f / readVoltage( adc ) - 1.0f;
  float tr = 3.30f / Board->readVoltage( adc ) - 1.0f;

  tr = reference_resistance / tr;
  steinhart = tr / nominal_resistance;                  // (R/Ro)
  steinhart = log(steinhart);                           // ln(R/Ro)
  steinhart /= b_value;                                 // 1/B * ln(R/Ro)
  steinhart += 1.0f / (nominal_temperature + 273.15f);  // + (1/To)
  steinhart = 1.0f / steinhart;                         // Invert
  steinhart -= 273.15f;
  if ( steinhart == -273.15f ) steinhart = 120.0f;
//  return steinhart;         //( steinhart > 120.0f ) ? 120.0f : steinhart;
    return ( steinhart > 120.0f ) ? 120.0f : steinhart;
}
