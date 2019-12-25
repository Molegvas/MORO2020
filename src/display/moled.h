#ifndef _MOLED_H_
#define _MOLED_H_

#include <Arduino.h>

class U8G2_SSD1309_128X64_NONAME2_F_4W_SW_SPI;

class MOled
{
  public:
    MOled();
    ~MOled();

    void resetLCD();
    void runDisplay(float u, float i, float celsius, int time, float ah, bool ap);
    void showLine4RealVoltage();
    void showLine4Text(String s = "             ");
    void showLine3RealCurrent();
    void showLine3Text(String s = "             ");
    void showLine3Akb(float u, float ah);
    void showLine3MaxI(float i);
    void showLine3MaxU(float u);
    void showLine3Delay(int d);
    void showLine3Num(int n);
    void showLine3Power(float u, float i);
    void showLine3SV(float sv);
    void showLine3SI(float si);
    void showLine3Capacity(float ah);
    void showLine3Sec(float sec);
    void showLine3Avr(float amp);

    void showLine2Text(String s = "                ");
    void showLine1Time(int t);
    void showLine1Heap(int h);                //     uint32_t getFreeHeap();
    void showLine1Ah(float ah);
    void showLine1Celsius(float c);

    bool blink();

  private:
    U8G2_SSD1309_128X64_NONAME2_F_4W_SW_SPI * U8g2 = nullptr;

    enum show_mode { m_work, m_text, m_akb, m_max_i, m_max_u, m_delay,
                     m_number, m_power, m_voltage, m_current, m_time,
                     m_celsius, m_ah, m_heap, m_capacity, m_sec, m_avr_curr };

    show_mode modeLine4  = m_work;
    show_mode modeLine3  = m_work;
    show_mode modeLine2  = m_work;
    show_mode modeLine1l = m_heap;
    show_mode modeLine1r = m_celsius;


    String line4 = "             ";     // 13 characters
    String line3 = "             ";     // 13 characters
    String line2 = "                ";  // 16 characters. The second line at the bottom of the display
    String line1 = "                ";  // 16 characters

    float fPar31 = 0.0f;
    float fPar32 = 0.0f;
    int   iPar31 = 0;
    float fPar11 = 0.0f;
    float fPar12 = 0.0f;
    int   iPar11 = 0;
    int   iPar12 = 0;

    int sec = 0;
};

#endif // _MOLED_H_