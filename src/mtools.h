#ifndef _MTOOLS_H_
#define _MTOOLS_H_

#define DEBUG_CHARGE

class MBoard;
class MOled;
class MKeyboard;
class Preferences;

class MTools
{
  public:
    MTools(MBoard * board, MOled * oled);
    ~MTools();
   
    //pointers are public - easier to access
    MBoard      * Board        = nullptr; // external
    MOled       * Oled         = nullptr; // external
    MKeyboard   * Keyboard     = nullptr; // local
    Preferences * qPreferences = nullptr; // local

    // Номинальное напряжение батареи, В
    float getVoltageNom() const;
    void  setVoltageNom(float);

    //float capacityNom = 55.0f;   // Номинальная емкость батареи, А
    //float getCapacityNom();
    //void  setCapacityNom(float);

    // Индекс массива выбора батареи    
    int  getAkbInd();
    void setAkbInd(int);

    // Заданная емкость батареи, А-ч
    float getCapacity();
    void  setCapacity(float);

    // Заданное напряжение заряда, В
    float getVoltageMax();
    void  setVoltageMax(float);

    // Заданное минимальное снижение напряжения, В
    float getVoltageMin();
    void  setVoltageMin(float);

    // Заданный максимальный ток заряда, А
    float getCurrentMax();
    void  setCurrentMax(float);

    // Заданный минимальный ток заряда, А
    float getCurrentMin();
    void  setCurrentMin(float);    

    // Заданное напряжение предзаряда, В
    float getVoltagePre();
    void  setVoltagePre(float); 

    // Заданный ток предзаряда, А
    float getCurrentPre();
    void  setCurrentPre(float);    

    // Заданное минимальное напряжение разряда, В
    float getVoltageDis();
    void  setVoltageDis(float); 

    // Заданный ток разряда, А
    float getCurrentDis();
    void  setCurrentDis(float);    

//    float currentOld  =  0.0f;   // Для формирования импульсов в фазе удержания напряжения nu

    // Накопитель для вычисления среднего (тока)
    float getCollectAvr();
    void  setCollectAvr(float);    

    // Средний ток в импульсе
    float getCurrentAvr();
    void  setCurrentAvr(float);    

    //bool  keepCurr    = false;  // Признак окончания роста тока 
    bool  getKeepCurr();
    void  setKeepCurr(bool);    

    // Признак перехода к фазе удержания напряжения nu?
    bool  getToKeepVolt();
    void  setToKeepVolt(bool);    

    // Заданная длительность импульса заряда, с 
    float getDurationOn();
    void  setDurationOn(float);    

    // Заданная пауза или длительность разряда, с
    float getDurationOff();
    void  setDurationOff(float);    

    int   getNumCycles();
    void  setNumCycles(int);

    int   getPause();
    void  setPause(int);
   

    int   getCycle();
    void  setCycle(int);
    void  decCycle(); 


    int   getCycles();
    void  setCycles(int);


    void  initPulse( float _current );
    void  initPause();

//    void  upPulse(float);
    void  runPulse();
    float stopPulse();





    // будут в private
    float powO        =  0.0f;   // резерв

//    int   cycle;
//    int   cycles;               // Всего задано циклов в имульсе заряда, разряда или паузы
    int   postpone    =  0;      // Заданная задержка включения (отложенный старт), ч


    bool getAP();

    void  zeroAhCounter();      // Обнуление счетчика ампер-часов заряда


    int   readNvsInt   (const char * name, const char * key, const int   defaultValue );
    float readNvsFloat (const char * name, const char * key, const float defaultValue );

    void  writeNvsInt  (const char * name, const char * key, const int iValue );
    void  writeNvsFloat(const char * name, const char * key, const float fValue );


    void  clearAllKeys (const char * name);

    void incCurrentOffset( float delta, bool way );
    void decCurrentOffset( float delta, bool way );

    void incVoltageOffset( float delta, bool way );
    void decVoltageOffset( float delta, bool way );

    void incBattery();
    void decBattery();

    void incCapacity( float delta, bool way );
    void decCapacity( float delta, bool way );

    void incCurrentMax( float delta, bool way );
    void decCurrentMax( float delta, bool way );

    void incCurrentMin( float delta, bool way );
    void decCurrentMin( float delta, bool way );

    // void incCurrentDis( float delta, bool way );
    // void decCurrentDis( float delta, bool way );    

    void incVoltageMax( float delta, bool way );
    void decVoltageMax( float delta, bool way );

    void incVoltageMin( float delta, bool way );
    void decVoltageMin( float delta, bool way );

    void incVoltagePow( float delta, bool way );
    void decVoltagePow( float delta, bool way );

    void incVoltagePre( float delta, bool way );
    void decVoltagePre( float delta, bool way );

    void incCurrentPre( float delta, bool way );
    void decCurrentPre( float delta, bool way );

    void incDurationOn( bool way );
    void decDurationOn( bool way );
    void incDurationOff( bool way );
    void decDurationOff( bool way );

    void incPostpone( int delta );
    void decPostpone( int delta );

    void incCycles();
    void decCycles();

    void incCurrentDis( float delta, bool way );
    void decCurrentDis( float delta, bool way );

    void incVoltageDis( float delta, bool way );
    void decVoltageDis( float delta, bool way );

    void incPause();
    void decPause();

    int   incNum( int v, int h, int d );
    int   decNum( int v, int l, int d );


// inc, dec 201905
    float incFloatValue( float value,  float value_l, float value_h, float delta ); 
    float decFloatValue( float value,  float value_l, float value_h, float delta );
    int   incIntValue( int value, int value_l, int value_h, int delta );
    int   decIntValue( int value, int value_l, int value_h, int delta );


    void saveInt(const char * name, const char * key, const int ivalue );
    void saveFloat(const char * name, const char * key, const float fvalue );

    void activateSelBat();
    void activateSetCapacity();
    void activateSetCurrMax(bool);
    void activateSetVoltMax(bool);
    void activateSetCurrentPre();
    void activateSetVoltagePre();

    void activateSetDurationOn();
    void activateSetDurationOff();
    void activateSetNumCycl();
    void activateSetCurrDis();
    void activateSetVoltDis();
    void activateSetPause();
    void activateExit(const char * s);



    void clrTimeCounter();
    void clrChargeTimeCounter();
    void clrAhCharge();
    int  getTimeCounter();
    void setTimeCounter( int ivalue );


    bool getPidStopped();

    void activateImpulsCharge( float volt, float amp, float delta );
//  void activateImpulsChargeVar2( float volt, float delta );
   
//    void runImpulsCharge();
//    void activateImpulsDischarge( float amp );
//    void runImpulsDischarge( float amp );

// Импульсный заряд, фаза удержания напряжения
    void activateImpulsCharge2( float volt, float amp );
    void runImpulsCharge2();
    void activateImpulsDischarge2( float amp ); 
    void runImpulsDischarge2();

// Общие для импульсного разряда
    void runImpDisch();

    void initCurrentAvr();
    void addCollectAvr( float amp );
    int  cnt = 0;
    
    float calcCurrentAvr();

//================= Measures =====================
//    float readVoltage(float reading);

    void  setCelsius(int ivalue);

//============================ Power ===============================================

    const float pows[6][2] = { { 3.3f, 12.0f},
                              { 5.0f,  8.0f},
                              { 6.3f,  6.0f},
                              { 9.0f,  4.0f}, //**
                              {12.0f,  3.0f},
                              {16.0f,  2.0f} };

        int powInd =  3;
    //static int   selectedDelay   =  0;
    //float timeCounter = 0;

    void activateSelectUI();
    void savePowInd( const char * name ); 
    void savePowO( const char * name ); 

    void upPow();
    void dnPow();
    void liveU();

    void activatePowerPh1( float volt, float amp );
    void runPidVoltage();

    void printPid();                    // input, setPoint, integral, output


    void powShutdown(); 

//============================ Charger ===============================================
    // Значения только для теста (читать дефолтные при чистой памяти)
    int   akbInd = 3;           // в private?

    const float akb[7][2] = { {12.0f, 100.f},    // авто 
                            {12.0f,  90.0f},      //  "
                            {12.0f,  75.0f},      //  "
                            {12.0f,  55.0f},      //  "
                            {12.0f,   9.0f},      // ибп
                            { 6.0f,  12.0f},      // дет.авто
                            { 6.0f,   4.5f} };    // фонарь

    int   getChargeTimeCounter();       // common?
    float getAhCharge();                // common?


    void activateChargeStart();    

    void activatePrecharge( float volt, float amp, bool impuls );

    void activateSuspendCharge();   // to suspend the charge
    void activateResumeCharge(); //to resume the charge

    void setSetPoint( float sp );


    void activateChargePh1( float volt, float amp, bool recharge );
    void runChargePh1();

// новые, с честным подъёмом тока без ПИД
    void activatePostpone( float volt, float amp );
    bool postponeCalculation();
    void activatePreliminaryCharge();
    void runPreliminaryCharge( float volt, float amp, float delta );


  void activateImpuls( float volt, float amp, float delta );


    void activateChargePh2( float volt );
    void runChargePh2();
    void activateChargePh3();
    
    void shutdownDC();
    void shutdownCharge();

    void chargeCalculations();
//    void disChargeCalculations();
    void showUpDn();                // " UP/DN, В-выбор "


//================== Pid 201905 ====================

    void initPid( float oMin, float oMax, 
                  float p, float i, float d, 
                  float bMin, float bMax, 
                  unsigned long tStep );    //, 
                  //float sP );

    void runPid( float in );
    void stopPid();

    bool atSetPoint(float threshold);
    void adjustIntegral( float _corr );
    void saveIntegral(float _integral);
    void restoreIntegral();
    void setIntegral( float _integral ); 
    float getIntegral();
 
    //void adjustOutputRange( float _oMin, float _oMax );
    void  setOutputRange( float fmin, float fmax );


    // Any constants - приборные ограничения
    const float capacity_l          =  10.0f;   // минимальная емкость АКБ, Ач
    const float capacity_h          = 200.0f;   // максимальная емкость АКБ, Ач

    const float min_akb_u           =   3.5f;   // min_AkbU минимальное напряжение АКБ, в
    const float max_akb_u           =  16.5f;   // max_ZU максимально разрешенное напряжение при установке, в
    const float min_akb_i           =   0.2f;
    const float max_akb_i           =  12.0f;   // приборное ограничение тока, А

    const float volt_l              =   1.0f;
    const float volt_h              =  16.0f;

    const int min_akb_delay         =   0;      // Ограничения отсрочки заряда, ч
    const int max_akb_delay         =  24;      //
    const int charge_time_out_limit =  1; //48;      // Ограничение продолжительности заряда, ч

    const float current_increment_in_one_step = 0.50f; // / 0.20; // приращение тока за один шаг, А
//============================ Service ===============================================

    void activatePause();
    bool pauseCalculations();
    void activateDischarge();

//==== PRIVATE ==== PRIVATE ==== PRIVATE ==== PRIVATE ==== PRIVATE ==== PRIVATE ==== PRIVATE ==== PRIVATE ==== PRIVATE ====
private:

// Переменные будут здесь
    float voltageNom  = 12.0f;   // Номинальное напряжение батареи, В
  //float capacityNom = 55.0f;   // Номинальная емкость батареи, А    nu
    float capacity    = 50.f;    // Заданная емкость батареи, А-ч
    float voltageMax  = 14.5f;   // Заданное напряжение заряда, В
    float voltageMin  = 13.2f;   // Заданное минимальное снижение напряжения, В
    float currentMax  =  5.0f;   // Заданный максимальный ток заряда, А
    float currentMin  =  0.5f;   // Заданный минимальный ток заряда, А
    float voltagePre  =  6.0f;   // Заданное напряжение предзаряда, В
    float currentPre  =  3.0f;   // Заданный ток предзаряда, А
    float voltageDis  = 10.7f;   // Заданное минимальное напряжение разряда, В
    float currentDis  =  2.8f;   // Заданный ток разряда, А
    float collectAvr  =  0.0f;   // Накопитель для вычисления среднего (тока)
    float currentAvr  =  0.0f;   // Средний ток в импульсе
    bool  keepCurr    = false;   // Признак окончания роста тока   
    bool  toKeepVolt  = false;   // Признак перехода к фазе удержания напряжения nu?
    float durationOn  = 10.0f;   // Заданная длительность импульса заряда, с 
    float durationOff =  5.0f;   // Заданная пауза или длительность разряда, с

    int   numCycles   =  3;      // Заданное число циклов, например заряд/разряд
    int   pause       =  1;      // Заданная пауза между зарядом и разрядом, ч
    // float powO        =  0.0f;   // резерв

    int   cycle;
    int   cycles;               // Всего задано циклов в имульсе заряда, разряда или паузы
    // int   postpone    =  0;      // Заданная задержка включения (отложенный старт), ч
    int   count;                // для подсчета числа измерений тока в импульсе

// Пределы регулирования уточнить
// Некоторые пределы зависят от выбора батареи ( const только для приборных ограничений !!! ) 
// Здесь указать приборные ограничения !!!

    const bool onlimit = false;
    const bool onloop  = true;

    const float curr_offset_h =  2.00f;
    const float curr_offset_l = -2.00f;

    const float volt_offset_h =  2.00f;
    const float volt_offset_l = -2.00f;




    const float curr_max_l =  0.2f;
    const float curr_max_h = 12.2f;

  

    const float volt_end_l = 10.0f;
    const float volt_end_h = 16.0f;

    const int   num_cycl_l = 0;
    const int   num_cycl_h = 3;

    const float curr_dis_l =  0.0f;
    const float curr_dis_h =  2.5f;

    const float duration_on_l  =  5.0f;         // для импульсных режимов
    const float duration_on_h  = 10.0f;

    const float duration_off_l  = 2.5f;
    const float duration_off_h  = 5.0f;

    const int   postpone_l =  0;        // задержка включения (Отложенный старт)
    const int   postpone_h = 24;

    const float volt_min_l = 10.0f;
    const float volt_min_h = 16.0f;

    const int   pause_l = 0;
    const int   pause_h = 3;

    const float volt_pre_l  =  1.5f;
    const float volt_pre_h  = 14.0f;

    const float curr_pre_l  =  0.5f;
    const float curr_pre_h  =  6.0f;



    float incfValue( float value, float min, float max, float delta, bool way );
    float decfValue( float value, float min, float max, float delta, bool way );
    int   inciValue( int value,  int min, int max, int delta );
    int   deciValue( int value,  int min, int max, int delta );



    // Power
    const int number_of_powers = 6;

    // Charge
    float integral = 0.0f;
    float temp = 0.0f;
    const int number_of_batteries = 7;
    float ahCharge = 0.0f;
    int   timeCounter = 0;
    int   chargeTimeCounter = 0;

    void activateCharge( float volt, float amp );

    void printAll( const char * s );

//================= Measures =====================

    //const float voltageDivider    = ( 39.0f + 5.6f ) / 5.6f;  // Делитель 39кОм 1% / 5.6кОм 1%
    //const float acs712Multiplier  = -12.50f;                  //  80mV/А  ( 1/0.080)
    //const float acs712Offset      =  20.00;

    int   keyCode = 0;    

// ============================= cool ===========================
    // MF52AT MF52 B 3950 NTC термистор 2% 10 kOm
    const float reference_resistance = 10000.0f;    // R13 10kOm 1%
    const float nominal_resistance   = 10000.0f;    //     10kOm 2%
    const float nominal_temperature  =    25.0f;
    const float b_value              =  3950.0f;

    float readSteinhart( const int adc );

};

#endif //_MTOOLS_H_