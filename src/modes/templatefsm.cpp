/*
 * templatefsm.cpp – шаблон конечного автомата.
 * На дисплее при выборе режима отображается как "   Пример FSM   ", которое
 * надо указать в диспетчере выбора режима, как и его позицию в меню.
 * Выбор режима производится коротким нажатием на "В" – "Выбор"
*/

#include "modes/templatefsm.h"
#include "measure/mkeyboard.h"
#include <Arduino.h>

// Переименуйте поле имен для вашего режима.
namespace TemplateFsm
{
    // Старт
    MState * MStart::fsm()
    {
        // В любом состоянии короткое нажатие на кнопку "C" резервируем для выхода из режима.
        // Диспетчер режимов производит вход каждые 100 миллисекунд. 
        if( Keyboard->getKey(MKeyboard::C_CLICK) ) return new MStop(Tools);       // Выбран выход
        if( Keyboard->getKey(MKeyboard::P_CLICK) ) return new MYellow(Tools);     // Выбран желтый
        if( Keyboard->getKey(MKeyboard::B_CLICK) ) return new MBlue(Tools);       // Выбран синий
        return this;                        // Ничего не выбрано, через 100мс проверять снова.
    };

    // Желтый
    MState * MYellow::fsm()
    {
        if( Keyboard->getKey(MKeyboard::C_CLICK) ) return new MStop(Tools);       // Выбран выход
        if( Keyboard->getKey(MKeyboard::UP_CLICK) ) return new MRed(Tools);       // Выбран красный
        if( Keyboard->getKey(MKeyboard::DN_CLICK) ) return new MGreen(Tools);     // Выбран зеленый
        return this;                        // Ничего не выбрано, через 100мс проверять снова.
    };

    // Красный
    MState * MRed::fsm()
    {
        if( Keyboard->getKey(MKeyboard::C_CLICK) ) return new MStop(Tools);       // Выбран выход
        if( Keyboard->getKey(MKeyboard::B_LONG_CLICK) ) return new MStart(Tools); // Выбран Старт, белый
        return this;                        // Ничего не выбрано, через 100мс проверять снова.
    };

    // Зеленый
    MState * MGreen::fsm()
    {
        if( Keyboard->getKey(MKeyboard::C_CLICK) ) return new MStop(Tools);       // Выбран выход
        if( Keyboard->getKey(MKeyboard::B_LONG_CLICK) ) return new MRed(Tools);   // Выбран красный
        if( Keyboard->getKey(MKeyboard::B_CLICK) ) return new MYellow(Tools);     // Выбран желтый

        if( --cnt <= 0 ) return new MStop(Tools);                           // Выход через 10 секунд

        Board->blinkGreen();                                                // Мигать зеленым
        return this;                        // Ничего не выбрано, через 100мс проверять снова.
    };

    // Синий
    MState * MBlue::fsm()
    {
        if( Keyboard->getKey(MKeyboard::C_CLICK) ) return new MStop(Tools);       // Выбран выход
        if( Keyboard->getKey(MKeyboard::B_LONG_CLICK) ) return new MStart(Tools); // Выбран Старт, белый
        return this;                        // Ничего не выбрано, через 100мс проверять снова.
    };

    // Состояние выхода из режима заряда – до короткого нажатия кнопки "С" 
    // удерживается индикация о продолжительности и отданном заряде, 
    // 
    MState * MStop::fsm()
    {
        if(Keyboard->getKey(MKeyboard::C_CLICK)) { 
            Tools->activateExit("   Пример FSM   "); 
            return 0;                                       // Для возврата к выбору режима возвращается 0. 
        }
        return this;
    };

};
