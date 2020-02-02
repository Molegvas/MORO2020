/*
 * moled.cpp
 * Created: 28.12.2018
 * example: PrintUTF8.ino
 * Frame Buffer Examples: clearBuffer/sendBuffer. Fast, but may not work with all Arduino boards because of RAM consumption
 * Page Buffer Examples: firstPage/nextPage. Less RAM usage, should work with all Arduino boards.
 * U8x8 Text Only Example: No RAM usage, direct communication with display controller. No graphics, 8x8 Text only.
 * Author: Ol.Moro 
 */ 

#include "display/moled.h"
#include <U8g2lib.h>
#include "board/mboard.h"
#include "board/mpins.h"
#include <Arduino.h>

//m
//#include <SPI.h>
//SPIClass * hspi = NULL;

MOled::MOled()
{
    // The complete list is available here: https://github.com/olikraus/u8g2/wiki/u8g2setupcpp
#ifdef V22
    U8g2 = new U8G2_SSD1309_128X64_NONAME2_F_4W_SW_SPI( U8G2_R2, MPins::scl_pin, MPins::sda_pin, MPins::cs_pin, MPins::dc_pin, MPins::res_pin );  // OK
#endif

#ifdef V43
    #ifdef HW_HSPI

        //hspi = new SPIClass(HSPI);
        //hspi->begin();

        U8g2 = new U8G2_SSD1309_128X64_NONAME2_F_4W_HW_SPI (U8G2_R2, MPins::cs_pin,
                                                                     MPins::dc_pin,
                                                                     MPins::res_pin);  
    #else
        U8g2 = new U8G2_SSD1309_128X64_NONAME2_F_4W_SW_SPI( U8G2_R2, MPins::scl_pin,
                                                                     MPins::sda_pin,
                                                                     MPins::cs_pin,
                                                                     MPins::dc_pin,
                                                                     MPins::res_pin);
    #endif
#endif

    resetLCD();
}

MOled::~MOled()
{
    delete U8g2;
}

void MOled::resetLCD()
{
    U8g2->begin();                   // Include: initDisplay(); clearDisplay(); setPowerSave(0);
    U8g2->enableUTF8Print();         // enable UTF8 support for the Arduino print() function
    U8g2->setFontMode(0);            // enable transparent mode, which is faster       TEST
}

// Line 4 show
void MOled::showLine4RealVoltage() { modeLine4 = m_work; }                                      // Отображать текущее напряжение
void MOled::showLine4RealCurrent() { modeLine4 = m_current; }                                      // Отображать текущее напряжение

void MOled::showLine4Text(String s) { line4 = s;    modeLine4 = m_text; }                       // Выводить текст

// Line 3 show
void MOled::showLine3RealCurrent() { modeLine3 = m_work; }                                      // Отображать текущий ток
void MOled::showLine3Text(String s) { line3 = s;    modeLine3 = m_text; }
void MOled::showLine3Akb(float u, float ah) { fPar31 = u; fPar32 = ah;  modeLine3 = m_akb; }    // "  12В  55А-ч "
void MOled::showLine3MaxI(float i) { fPar31 = i;  modeLine3 = m_max_i; }                        //
void MOled::showLine3MaxU(float u) { fPar31 = u;   modeLine3 = m_max_u; }                       // "   14,8 B    "
void MOled::showLine3Delay(int d) { iPar31 = d;   modeLine3 = m_delay; }                        // "     24 ч    "
void MOled::showLine3Num(int n) { iPar31 = n;  modeLine3 = m_number; }
void MOled::showLine3Power(float u, float i) { fPar31 = u; fPar32 = i; modeLine3 = m_power; }   // "   12В  10А  "
void MOled::showLine3SV(float sv) { fPar31 = sv;  modeLine3 = m_voltage; }                      // 
void MOled::showLine3SI(float si) { fPar31 = si;  modeLine3 = m_current; }                      //
void MOled::showLine3Capacity(float ah) { fPar31 = ah, modeLine3 = m_capacity; }
void MOled::showLine3Sec(float sec) { fPar31 = sec, modeLine3 = m_sec; }
void MOled::showLine3Avr(float amp) { fPar31 = amp, modeLine3 = m_avr_curr; }

// Line 2 show
void MOled::showLine2Text(String s) { line2 = s; }                                              // For example: " Простой заряд  "

// Line 1 show
void MOled::showLine1Time(int t) { iPar11 = t; modeLine1l = m_time; }
void MOled::showLine1Heap(int h) { iPar12 = h; modeLine1l = m_heap; }                           // uint32_t getFreeHeap();
void MOled::showLine1Ah(float ah) { fPar11 = ah; modeLine1r = m_ah; }
void MOled::showLine1Celsius(float c) { fPar12 = c; modeLine1r = m_celsius; }

bool MOled::blink() { return ( sec % 2 == 0 ); }

void MOled::runDisplay(float _voltage, float _current, float _celsius, int _chargeTimeCounter, float _ahCharge, bool _ap) 
{
    sec++;
    U8g2->setFont(u8g2_font_10x20_t_cyrillic);     // use cyrillic font_10x20
    U8g2->firstPage();
    do 
    {

// Line 4 show, the count is made from below (1 ... 4)
        switch (modeLine4) {
            case m_work:
            // Show voltage
                // if( _voltage > 0) { ( _voltage <  10.0) ? U8g2->setCursor(45, 13) : U8g2->setCursor(35, 13); }
                // else            { ( _voltage < -10.0) ? U8g2->setCursor(25, 13) : U8g2->setCursor(35, 13); }
                // U8g2->print( _voltage, 2);    U8g2->print(" В");
                if( _voltage > 0.0f) { ( _voltage <  10.0f) ? U8g2->setCursor(55, 13) : U8g2->setCursor(45, 13); }
                else            { ( _voltage < -10.0f) ? U8g2->setCursor(35, 13) : U8g2->setCursor(45, 13); }
                U8g2->print( _voltage, 2);    U8g2->print(" В");

            break;

            case m_current:
                //U8g2->setCursor(0, 13);
                ( (_current < 0.0f) || (_current >= 10.0f) ) ? U8g2->setCursor(45, 13) : U8g2->setCursor(55, 13);
                U8g2->print( _current, 1);    U8g2->print(" А"); 
            break;

            case m_text:
                U8g2->setCursor(0, 13); U8g2->print(line4);           // ("             ");   // 13 знакомест
            break;

            default:
            break;
        }
//        if(_ap) { U8g2->setCursor(109, 13); U8g2->print("AP"); }

        
// Line 3 show
        switch (modeLine3) {
            case m_work:
           // Show current
                //( (_current < 0.0f) || (_current >= 10.0f) ) ? U8g2->setCursor(35, 32) : U8g2->setCursor(45, 32);
                //U8g2->print( _current, 2);    U8g2->print(" А");
                ( (_current < 0.0f) || (_current >= 10.0f) ) ? U8g2->setCursor(45, 32) : U8g2->setCursor(55, 32);
                U8g2->print( _current, 1);    U8g2->print(" А");
            break;
            
            case m_text:
                U8g2->setCursor(0, 32); U8g2->print(line3);       //( "             " );   // 13 знакомест
            break;
            
            case m_akb:                     // "  12В  55А-ч "
                U8g2->setCursor(20, 32); U8g2->print(fPar31, 0);   U8g2->print("В");
                U8g2->setCursor(70, 32); U8g2->print(fPar32, 0);   U8g2->print("Ач");
            break;

            case m_max_i:                     // "    5,5 A    "
                U8g2->setCursor(45, 32); U8g2->print(fPar31, 1);   U8g2->print(" A");

            break;

            case m_max_u:                     // "   14,8 B    "
                U8g2->setCursor(45, 32); U8g2->print(fPar31, 1);   U8g2->print(" B");
            break;

            case m_delay:                     // "     24 ч    "
                U8g2->setCursor(45, 32); U8g2->print(iPar31);   U8g2->print(" ч");
            break;

            case m_number:                     // "     24      "
                U8g2->setCursor(55, 32); U8g2->print(iPar31);     //45
            break;

            case m_power:                     // "   12В  10А  "
                U8g2->setCursor(20, 32); U8g2->print(fPar31, 1);   U8g2->print("B");
                U8g2->setCursor(80, 32); U8g2->print(fPar32, 1);   U8g2->print("A");
            break;

            case m_voltage:
                U8g2->setCursor(45, 32); U8g2->print(fPar31, 1);   U8g2->print(" B");
            break;

            case m_current:
                U8g2->setCursor(45, 32); U8g2->print(fPar31, 1);   U8g2->print(" B");
            break;

            case m_capacity:
                U8g2->setCursor(45, 32); U8g2->print(fPar31, 0);   U8g2->print(" Ач");
            break;

            case m_sec:
                U8g2->setCursor(45, 32); U8g2->print(fPar31, 1);   U8g2->print(" с");
            break;

            case m_avr_curr:                            // Отображать средний ток (в скобках)
                //U8g2->setCursor(35, 32); U8g2->print("("); U8g2->print(fPar31, 2); U8g2->print(")"); U8g2->print("A");
                U8g2->setCursor(45, 32); U8g2->print("("); U8g2->print(fPar31, 1); U8g2->print(")"); U8g2->print("A");
            break;

            default:
            break;
        }

    if(_ap) { U8g2->setCursor(0, 22); U8g2->print("AP"); }


// Line 2 and Line 1 show
        U8g2->setFont(u8g2_font_8x13_t_cyrillic);     // use cyrillic font_8x13

// Line 2 show
        // The second line at the bottom of the display 
        U8g2->setCursor(0, 46);
        U8g2->print(line2);                     // For example: "  CC/CV заряд   "

// Line 1 show
        switch (modeLine1l)
        {
            case m_time:
                // Show time of charge
                U8g2->setCursor(0, 62);
                U8g2->print (_chargeTimeCounter/60/60);
                U8g2->print (":");
                if (_chargeTimeCounter / 60 % 60 < 10) { U8g2->print ("0"); }
                U8g2->print ((_chargeTimeCounter / 60) % 60);
                U8g2->print (":");
                if (_chargeTimeCounter % 60 < 10)      { U8g2->print ("0"); }
                U8g2->print (_chargeTimeCounter % 60);
            break;

            case m_heap:                                    // Показывать сколько свободной памяти
                U8g2->setCursor(0, 62);
                iPar11 = ESP.getFreeHeap();                                       
                U8g2->print(iPar11);
            break;

            default:
                U8g2->setCursor(0, 62);
                U8g2->print("unknown");
            break;
        }       

        switch (modeLine1r)
        {
            case m_ah:
                // Show Amp * hour
                U8g2->setCursor(80, 62);
                // if(_ahCharge < 10.0f) {
                //     U8g2->print(_ahCharge, 1); 
                // }
                // else {
                //     U8g2->print(_ahCharge, 0); 
                // }   
                // U8g2->print("Ач");
    U8g2->print(_celsius, 1);       U8g2->print("C");    // Показывать температуру радиатора TEST

            break;

            case m_celsius:
                U8g2->setCursor(80, 62);
                U8g2->print(_celsius, 1);       U8g2->print("C");    // Показывать температуру радиатора
            break;

            default:
                U8g2->setCursor(80, 62);
                U8g2->print("na");
        }
    } 
    while ( U8g2->nextPage() );
}
