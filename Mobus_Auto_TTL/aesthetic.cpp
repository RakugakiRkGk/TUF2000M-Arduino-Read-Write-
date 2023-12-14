#include "aesthetic.h"

AESTHETIC_FUNCTIONS::AESTHETIC_FUNCTIONS(){
}

float AESTHETIC_FUNCTIONS::readFlow(){
  return functions.readFloat(FLOW_REGISTER, REAL_DATA_SIZE);
}

float AESTHETIC_FUNCTIONS::readTemp(){
  return functions.readFloat(TEMPERATURE_IN_1, REAL_DATA_SIZE);
}

float AESTHETIC_FUNCTIONS::readPosTot(){
  return functions.readFloat(POS_ACCUMULATOR, REAL_DATA_SIZE);
}

uint32_t AESTHETIC_FUNCTIONS::readTime(){
  return functions.readUnsignedLong(WORKING_TIMER, LONG_DATA_SIZE);
}

int32_t AESTHETIC_FUNCTIONS::readLongPosTot(){
  return functions.readLong(POS_ACC_LONG, LONG_DATA_SIZE);
}

int16_t AESTHETIC_FUNCTIONS::flowUnit(){
  return functions.readInt(FLOW_UNIT);
}

int16_t AESTHETIC_FUNCTIONS::currentWindows(){
  return functions.readInt(CURRENT_WINDOW);
}

int AESTHETIC_FUNCTIONS::errorCode(){
  return functions.readBit(ERROR_CODE);
}

void AESTHETIC_FUNCTIONS::GoToMenu(uint16_t menu){
  Serial.print("Menu ");
  functions.writeReg(MENU_REG,menu);
  delay(10);
}

void AESTHETIC_FUNCTIONS::InputKey(uint16_t key){
 Serial.print("Key pressed: ");
 functions.writeReg(KEY_TO_INPUT, key);
 delay(10);
}


void AESTHETIC_FUNCTIONS::printTime (uint32_t TotalSeconds){
  uint16_t hours = TotalSeconds / 3600;
  uint16_t minutes = (TotalSeconds % 3600) / 60;
  uint16_t seconds = TotalSeconds % 60;

  Serial.print(hours);
  Serial.print(":");
  if (minutes < 10) {
    Serial.print("0");  // Agrega un cero si los minutos son menores a 10
  }
  Serial.print(minutes);
  Serial.print(":");
  if (seconds < 10) {
  Serial.print("0");  // Agrega un cero si los segundos son menores a 10
  }
  Serial.println(seconds);
}

void AESTHETIC_FUNCTIONS::printFlowUnit (uint16_t flowUnit){
  switch(flowUnit){
    case 0: Serial.println("m3/s"); break;
    case 1: Serial.println("m3/m"); break;
    case 2: Serial.println("m3/h"); break;
    case 3: Serial.println("m3/d"); break;
    case 4: Serial.println("l/s"); break;
    case 5: Serial.println("l/m"); break;
    case 6: Serial.println("l/h"); break;
    case 7: Serial.println("l/d"); break;
    case 8: Serial.println("Gal/s"); break;
    case 9: Serial.println("Gal/m"); break;
    case 10: Serial.println("Gal/h"); break;
    case 11: Serial.println("Gal/d"); break;
    case 12: Serial.println("IGL/s"); break;
    case 13: Serial.println("IGL/m"); break;
    case 14: Serial.println("IGL/h"); break;
    case 15: Serial.println("IGL/d"); break;
    case 16: Serial.println("Mg/s"); break;
    case 17: Serial.println("Mg/m"); break;
    case 18: Serial.println("Mg/h"); break;
    case 19: Serial.println("Mg/d"); break;
    case 20: Serial.println("cf/s"); break;
    case 21: Serial.println("cf/m"); break;
    case 22: Serial.println("cf/h"); break;
    case 23: Serial.println("cf/d"); break;
    case 24: Serial.println("OB/s"); break;
    case 25: Serial.println("OB/m"); break;
    case 26: Serial.println("OB/h"); break;
    case 27: Serial.println("OB/d"); break;
    case 28: Serial.println("IB/s"); break;
    case 29: Serial.println("IB/m"); break;
    case 30: Serial.println("IB/h"); break;
    case 31: Serial.println("IB/d"); break;
    default: Serial.println("Unidad no reconocida"); break;
  }
}

void AESTHETIC_FUNCTIONS::printErrorCode (uint16_t error_code){
  switch(error_code){
    case 0: Serial.println("Bit0 no received signal"); break;
    case 1: Serial.println("Bit1 low received signal"); break;
    case 2: Serial.println("Bit2 poor received signal"); break;
    case 3: Serial.println("Bit3 pipe empty"); break;
    case 4: Serial.println("Bit4 hardware failure"); break;
    case 5: Serial.println("Bit5 receiving circuits gain in adjusting"); break;
    case 6: Serial.println("Bit6 frequency at the frequency output over flow"); break;
    case 7: Serial.println("Bit7 current at 4-20mA over flow"); break;
    case 8: Serial.println("Bit8 RAM check-sum error"); break;
    case 9: Serial.println("Bit9 main clock or timer clock error"); break;
    case 10: Serial.println("Bit10 parameters check-sum error"); break;
    case 11: Serial.println("Bit11 ROM check-sum error"); break;
    case 12: Serial.println("Bit12 temperature circuits error"); break;
    case 13: Serial.println("Bit13 reserved "); break;
    case 14: Serial.println("Bit14 internal timer over flow"); break;
    case 15: Serial.println("Bit15 analog input over range"); break;
    default: Serial.println("---> ERROR DESCONOCIDO <---"); break;
  }
}
