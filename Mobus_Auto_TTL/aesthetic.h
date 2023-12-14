#ifndef AESTHETIC_FUNCTIONS_H_
#define AESTHETIC_FUNCTIONS_H_

#include <Arduino.h>
#include "functions.h"
#include "registers.h"
// #include <HardwareSerial.h>
// #include <ModbusMaster.h>

class AESTHETIC_FUNCTIONS{
  private:

  MODBUS_FUNCTIONS functions {};
    // HardwareSerial swSerial {PA3,PA2};  //  swSerial (RX_pin, TX_pin); stm32f103 ==> rx -> PA3 ; tx -> PA2
    // ModbusMaster sensor {};                //  Inicializa el objeto de MODBUS
    
  protected:

  public: 
    
    AESTHETIC_FUNCTIONS();
    // Funciones para configurar el sensor
    void GoToMenu(uint16_t);
    void InputKey(uint16_t);
    
    // Funciones para interpretar datos
    void printTime (uint32_t);
    void printFlowUnit (uint16_t);
    void printErrorCode (uint16_t);

    // Funciones para la lectura de registros
    float readFlow();
    float readTemp();
    float readPosTot();
    uint32_t readTime();
    int32_t readLongPosTot();
    int16_t flowUnit();
    int16_t currentWindows();
    int errorCode();
};

#endif