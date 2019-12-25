/*
  Сonnection manager

*/

#include "mconnmng.h"
#include "mtools.h"
#include "board/mboard.h"
#include "display/moled.h"
#include "connect/connectfsm.h"
#include "Arduino.h"


MConnect::MConnect(MTools * tools) :
  Tools(tools), Board(tools->Board), Oled(tools->Oled)
{
    State = new ConnectFsm::MInit(Tools);
}

void MConnect::run()
{
    MState * newState = State->fsm();      
    if (newState != State)                  //state changed!
    {
      delete State;
      State = newState;
    } 
}
