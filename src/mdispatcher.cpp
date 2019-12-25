#include "mdispatcher.h"
#include "mtools.h"
#include "board/mboard.h"
#include "measure/mkeyboard.h"
#include "display/moled.h"

#include "modes/optionsfsm.h"
#ifdef TEMPLATE_ENABLE
  #include "modes/templatefsm.h"
#endif
#ifdef DC_SUPPLY_ENABLE
  #include "modes/dcsupplyfsm.h"
#endif
#ifdef PULSE_GEN_ENABLE
  #include "modes/pulsegenfsm.h"
#endif
#ifdef CCCV_CHARGE_ENABLE
  #include "modes/cccvfsm.h"
#endif
#ifdef PULSE_CHARGE_ENABLE           
  #include "modes/exchargerfsm.h"
#endif
#ifdef RECOVERY_ENABLE
  #include "modes/recoveryfsm.h"
#endif
#ifdef STORAGE_ENABLE
  #include "modes/storagefsm.h"
#endif
//#ifdef SERVICE_ENABLE
  #include "modes/servicefsm.h"
//#endif
#include "Arduino.h"


MDispatcher::MDispatcher(MTools * tools) :
  Tools(tools), Board(tools->Board), Oled(tools->Oled)
{
    mode = Tools->readNvsInt  ("qulon", "mode", 0);                 // Индекс массива
    showMode( mode );

    Board->ledsOff();                                               // при выборе режима

    Tools->powInd = Tools->readNvsInt  ("qulon", "powInd", 3);      // 3 - дефолтный индекс массива

    Tools->akbInd     = Tools->readNvsInt  ("qulon", "akbInd", 3);                  // Индекс массива с набором батарей
//    Tools->voltageNom = Tools->readNvsFloat("qulon", "akbU",   Tools->akb[3][0]);   // Начальный выбор 12 вольт
    Tools->setVoltageNom( Tools->readNvsFloat("qulon", "akbU",   Tools->akb[3][0]) );   // Начальный выбор 12 вольт

    Tools->setCapacity( Tools->readNvsFloat("qulon", "akbAh",  Tools->akb[3][1]) );   //                 55 Ач

    // Калибровки измерителей
    Board->voltageMultiplier  = Tools->readNvsFloat("qulon", "vMult",   1.00f); 
    Board->voltageOffset      = Tools->readNvsFloat("qulon", "vOffset", 0.00f);
    Board->currentMultiplier  = Tools->readNvsFloat("qulon", "cMult",   1.40f); 
    Board->currentOffset      = Tools->readNvsFloat("qulon", "cOffset", 0.00f); 
}

void MDispatcher::run()
{
  //  // Выдерживается период запуска для вычисления амперчасов
  if (State)
  {
    // rabotaem so state mashinoj
    MState * newState = State->fsm();      
    if (newState != State)                  //state changed!
    {
      delete State;
      State = newState;
    } 
    //esli budet 0, na sledujushem cikle uvidim
  }
  else //state ne opredelen (0) - vybiraem ili pokazyvaem rezgim
  {
    if (Tools->Keyboard->getKey(MKeyboard::UP_CLICK))
    { 
      if (mode == (int)SERVICE) mode = OPTIONS;
      else mode++;
      showMode( mode );
    }

    if (Tools->Keyboard->getKey(MKeyboard::DN_CLICK))
    { 
      if (mode == (int)OPTIONS) mode = SERVICE;
      else mode--;
      showMode( mode );
    }

    if (Tools->Keyboard->getKey(MKeyboard::B_CLICK))
    {
      // Запомнить крайний выбор режима
      Tools->writeNvsInt( "qulon", "mode", mode );

      // Serial.print("Available heap: "); Serial.println(ESP.getFreeHeap());
      // Serial.print("Core ID: ");        Serial.println(xPortGetCoreID());

      switch (mode)
      {
        case OPTIONS:
          State = new OptionFsm::MStart(Tools);
          break;

      #ifdef TEMPLATE_ENABLE
        case TEMPLATE:
          State = new TemplateFsm::MStart(Tools);
          break;
      #endif

      #ifdef DC_SUPPLY_ENABLE
        case DCSUPPLY:
          State = new DcSupplyFsm::MStart(Tools);
          break; 
      #endif

      #ifdef PULSE_GEN_ENABLE
        case PULSEGEN:
          State = new PulseGenFsm::MStart(Tools);
          break;
      #endif

      #ifdef CCCV_CHARGE_ENABLE
        case CCCV_CHARGE:
          State = new CcCvFsm::MStart(Tools);
          break;
      #endif

      #ifdef PULSE_CHARGE_ENABLE           
        case PULSECHARGE:
          State = new ExChargerFsm::MStart(Tools);
          break;
      #endif

      #ifdef RECOVERY_ENABLE
        case RECOVERY:
          State = new RecoveryFsm::MInvitation(Tools);
          break;
      #endif

      #ifdef STORAGE_ENABLE
        case STORAGE:
          State = new StorageFsm::MStart(Tools);
          break;
      #endif

//      #ifdef SERVICE_ENABLE
        case SERVICE:
          State = new ServiceFsm::MInvitation(Tools);
          break;
//      #endif

        default:
          break;
      }
    } // !B_CLICK
  }
}

void MDispatcher::showMode(int mode)
{
  String s;
  switch(mode)
  {
    case OPTIONS:     s = "    Настройки   "; break;

  #ifdef TEMPLATE_ENABLE
    case TEMPLATE:    s = "   Пример FSM   "; break;
  #endif

  #ifdef DC_SUPPLY_ENABLE
    case DCSUPPLY:    s = "  DC источник   "; break;
  #endif

  #ifdef PULSE_GEN_ENABLE
    case PULSEGEN:    s = " PULSE источник "; break;
  #endif

  #ifdef CCCV_CHARGE_ENABLE
    case CCCV_CHARGE: s = "  CC/CV  заряд  "; break;
  #endif

  #ifdef PULSE_CHARGE_ENABLE           
    case PULSECHARGE: s = " Импульсный зар."; break;
  #endif

  #ifdef RECOVERY_ENABLE
    case RECOVERY:    s = " Восстановление "; break;
  #endif

  #ifdef STORAGE_ENABLE
    case STORAGE:     s = "    Хранение    "; break;
  #endif

////  #ifdef SERVICE_ENABLE
    case SERVICE:     s = "   Сервис АКБ   "; break;
//  #endif

    default:          s = "  error  "; break;
  }
  Oled->showLine2Text(s);
}
