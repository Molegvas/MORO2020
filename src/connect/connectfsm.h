/*
  Сonnections state machine 

*/

#ifndef _CONNECT_H_
#define _CONNECT_H_

#include "mstate.h"

class MTools;

namespace ConnectFsm
{
    class MInit : public MState
    {       
        public:
            MInit(MTools * Tools) : MState(Tools) {}
            virtual MState * fsm() override;
    };

    class MExe : public MState
    {       
        public:
            MExe(MTools * Tools) : MState(Tools) {}
            virtual MState * fsm() override;
    };

};

#endif  // !_CONNECT_H_