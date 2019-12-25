#ifndef _TEMPLATEFSM_H_
#define _TEMPLATEFSM_H_

#include "mtools.h"                 // Методы для реализации пользовательских алгоритмов.
#include "board/mboard.h"           // Ресурсы платы управления и методы их использования.  
#include "display/moled.h"          // Ресурсы OLED дисплея и методы отбражения
#include "mstate.h"                 // Методы управления конечным автоматом

// Переименуйте поле имен для вашего режима.
namespace TemplateFsm
{
    // Класс MStart должен быть указан в меню диспетчера режимов,
    // напрмер: State = new TemplateFsm::MStart(Tools); 
    // для запуска вашего режима.
    class MStart : public MState
    {       
        public:
            MStart(MTools * Tools) : MState(Tools) {
                // Здесь, в конструкторе класса, разместите код инициализации 
                // состояния MStart, который будет исполняться всякий раз при 
                // входе в это состояние из иного состояния.

                // Индикация: укажем что показывать в каждой из 4-х строк дисплея.
                // Отсчет строк идет снизу. Строки 1 и 2 по 16 знакомест,
                // строки 3 и 4 – по 13, но шрифт крупнее.
                // Никакого контроля за размером строк нет, будьте аккуратны. 
                Oled->showLine4Text(" Мой пример. ");
                Oled->showLine3Text("  LED  WHITE ");
                Oled->showLine2Text(" P-YELL. B-BLUE ");        // Подсказка какие кнопки активны: P и B
                Oled->showLine1Time( 0 );                       // Зададим отображать время и отданный заряд и не будем
                Oled->showLine1Ah( 0.0f );                      // трогать на следующих шагах вплоть до выхода из режима.
                Board->ledsOn();                                // Светодиод светится белым как индикатор входа в режим
            }
        virtual MState * fsm() override;
    };

    class MYellow : public MState
    {
        public:     
            MYellow(MTools * Tools) : MState(Tools) {
                Board->ledsOff();    Board->ledROn();   Board->ledGOn();

                // Индикация
                Oled->showLine3Text("  LED YELLOW ");
                Oled->showLine2Text(" UP-RED  DN-GRN ");        // Подсказка какие кнопки активны: UP и DN
            }   
        virtual MState * fsm() override;
    };
       
    class MRed : public MState
    {
        public:   
            MRed(MTools * Tools) : MState(Tools) {
                Board->ledsOff();    Board->ledROn();
                // Индикация
                Oled->showLine3Text("  LED  RED   ");
                Oled->showLine2Text(" LONG B-BLUE BL ");        // Подсказка какие кнопки активны: LONG B
            }     
        virtual MState * fsm() override;
    };
    
    class MGreen : public MState
    {
        public:   
            MGreen(MTools * Tools) : MState(Tools) {
                Oled->showLine3Text("  BLINK GRN  ");
                Oled->showLine2Text(" LONG B-R  B-Y  ");        // Подсказка какие кнопки активны: LONG B и B
                cnt = duration;
            }     
        virtual MState * fsm() override;

        private:
        // Здесь располагаются переменные и константы, доступные в этом классе 
        int cnt;    // счетчик
        const int duration = 10 * 10;   // 10 секунд по 10 вызовов за секунду.
    };
    
    class MBlue : public MState
    {
        public:   
            MBlue(MTools * Tools) : MState(Tools) {
            Board->ledsOff();    Board->ledBOn();
                Oled->showLine3Text("  LED  BLUE  ");
                Oled->showLine2Text("  LONG B-Start  ");        // Подсказка какие кнопки активны: UP и DN
            }     
        virtual MState * fsm() override;
    };

    // Выключение преобразователя и коммутатора, установка в начальное состояние 
    // ШИМ-генераторов, включение красного светодиода тоже здесь.
    class MStop : public MState
    {
        public:  
            MStop(MTools * Tools) : MState(Tools) {
            Tools->shutdownCharge();                    
            }      
        virtual MState * fsm() override;
    };

};

#endif // !_TEMPLATEFSM_H_