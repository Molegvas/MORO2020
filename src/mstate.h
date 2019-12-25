#ifndef _MSTATE_H_
#define _MSTATE_H_

class MTools;
class MBoard;
class MOled;
class MKeyboard;

class MState{
  public:
    MState(MTools * Tools);

    virtual ~MState(){}

    virtual MState * fsm() = 0;

   protected:
    MTools    * Tools    = nullptr;
    MBoard    * Board    = nullptr;
    MOled     * Oled     = nullptr;
    MKeyboard * Keyboard = nullptr;
};

#endif //_MSTATE_H_
