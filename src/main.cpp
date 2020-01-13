/*
Q921v502
Апрель 2019

*/

#include "board/mboard.h"
#include "board/moverseer.h"
#include "display/moled.h"
#include "mtools.h"
#include "mdispatcher.h"
#include "mconnmng.h"
#include "measure/mmeasure.h"
#include "connect/connectfsm.h"
#include <Arduino.h>

static MBoard * Board = 0;
static MOled * Oled = 0;
static MTools * Tools = 0;
static MMeasure * Measure = 0;
static MDispatcher * Dispatcher = 0;
static MConnect * Connect = 0;


void connectTask ( void * parameter );
void displayTask ( void * parameter );
void coolTask    ( void * parameter );
void mainTask    ( void * parameter );
void measureTask ( void * parameter );

void setup() {
  Serial.begin(115200);
  delay(10);

  Oled  = new MOled();
  Board = new MBoard(Oled);
  Tools = new MTools(Board, Oled);
  Measure = new MMeasure(Tools);
  Dispatcher = new MDispatcher(Tools);
  Connect = new MConnect(Tools);

  xTaskCreatePinnedToCore ( connectTask, "Connect", 10000, NULL, 1, NULL, 1 );
  xTaskCreatePinnedToCore ( mainTask,    "Main",    10000, NULL, 2, NULL, 1 );
  xTaskCreatePinnedToCore ( displayTask, "Display",  5000, NULL, 2, NULL, 1 );
  xTaskCreatePinnedToCore ( coolTask,    "Cool",     1000, NULL, 2, NULL, 1 );
  xTaskCreatePinnedToCore ( measureTask, "Measure",  5000, NULL, 2, NULL, 1 );
}

void loop() {}

void connectTask( void * parameter ) {
  while(true) {
  //unsigned long start = millis();
    Connect->run(); 
    vTaskDelay( 10 / portTICK_PERIOD_MS );
  //Serial.print("Autoconnect: Core "); Serial.print(xPortGetCoreID()); Serial.print(" Time = "); Serial.print(millis() - start); Serial.println(" mS");
  // Core 1, 2...3 mS
  }
  vTaskDelete( NULL );
}

void displayTask( void * parameter ) {
  while(true)
  {
    //unsigned long start = millis();
    Oled->runDisplay( Board->getVoltage(), 
                      Board->getCurrent(), 
                      Board->Overseer->getCelsius(),
                      Tools->getChargeTimeCounter(),
                      Tools->getAhCharge(), 
                      Tools->getAP() );
    //Serial.print("Display: Core "); Serial.print(xPortGetCoreID()); Serial.print(" Time = "); Serial.print(millis() - start); Serial.println(" mS");
    // Core 1, Time = 47...53 mS
    vTaskDelay( 250 / portTICK_PERIOD_MS );
  }
  vTaskDelete( NULL );
}

void coolTask( void * parameter ) {
  while (true)
  {
    //unsigned long start = millis();
    Board->Overseer->runCool();
    //Serial.print("Cool: Core "); Serial.print(xPortGetCoreID()); Serial.print(" Time = "); Serial.print(millis() - start); Serial.println(" mS");
    // Core 1, 0 mS
    vTaskDelay( 200 / portTICK_PERIOD_MS );
  }
  vTaskDelete( NULL );
}

// 1. Задача обслуживает выбор режима работы.
// 2. Управляет конечным автоматом выбранного режима вплоть да выхода из режима.
void mainTask ( void * parameter ) { 
  while (true)
  {
    // Выдерживается период запуска для вычисления амперчасов
    portTickType xLastWakeTime = xTaskGetTickCount();   // To count the amp hours
    //unsigned long start = millis();
    Dispatcher->run(); 
    //Serial.print("Main: Core "); Serial.print(xPortGetCoreID()); Serial.print(" Time = "); Serial.print(millis() - start); Serial.println(" mS");
    // Core 1, 100...102 mS
    vTaskDelayUntil( &xLastWakeTime, 100 / portTICK_PERIOD_MS );    // период 0,1с
  }
  vTaskDelete( NULL );
}

void measureTask( void * parameter ) {
  while (true)
  {
    //unsigned long start = micros();
    Measure->run();
    //Serial.print(" Time, uS: "); Serial.println(micros() - start);
    // Core 1, 32 260 mkS
    vTaskDelay( 10 / portTICK_PERIOD_MS );
  }
  vTaskDelete(NULL);
}
