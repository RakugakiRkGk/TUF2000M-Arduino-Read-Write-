#ifndef MODBUS_FUNCTIONS_H_
#define MODBUS_FUNCTIONS_H_

#include <Arduino.h>
#include <HardwareSerial.h>
#include <ModbusMaster.h>
#include "registers.h"

class MODBUS_FUNCTIONS{
  private:
    HardwareSerial swSerial {PA3,PA2};  //  swSerial (RX_pin, TX_pin); stm32f103 ==> rx -> PA3 ; tx -> PA2
    ModbusMaster sensor {};                //  Inicializa el objeto de MODBUS
  protected:

  public: 
    
    MODBUS_FUNCTIONS();

    void writeReg(uint16_t, uint16_t);
    float readFloat(uint16_t, uint16_t);
    int32_t readLong(uint16_t, uint16_t);
    uint32_t readUnsignedLong(uint16_t, uint16_t);
    int16_t readInt(uint16_t);
    int readBit (uint16_t);
};

#endif