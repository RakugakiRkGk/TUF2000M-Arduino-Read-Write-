#include "functions.h"

MODBUS_FUNCTIONS::MODBUS_FUNCTIONS(){
  swSerial.begin(9600);             //  Serial de comunicacón con el sensor
  sensor.begin(SLAVE_ID, swSerial); //  Comunicación con el esclavo MODBUS, mediante el serial swSerial
}

void MODBUS_FUNCTIONS::writeReg(uint16_t register_num, uint16_t data){
  uint16_t result = sensor.writeSingleRegister(register_num, data);

  if (result == sensor.ku8MBSuccess){
    // Serial.println("write success");
    Serial.print(data);
    Serial.println();
  }
  else{
    Serial.print("Write Failure. Code: ");
    Serial.println(result, HEX);
  }
}
/*
REAL4 (32 bits en punto flotante):

| Formato:            |  Dirección de Registro:      |  Descripcion                                          |  Nota                                          |
|---------------------|------------------------------|-------------------------------------------------------|------------------------------------------------|
| IEEE 754 de 32bits  |  Puede ser parte de los      |  Almacena números de punto flotante de 32 bits        |  En el caudalimetro, solo se lee el registro   |
|                     |  registros Holding o Input.  |  Es adecuado para representar valores con decimales.  |  Holding (readHoldingRegister(reg, size))      |
*/
float MODBUS_FUNCTIONS::readFloat(uint16_t register_num, uint16_t data_size) {
  uint16_t j, result;
  uint16_t buf[data_size];
  uint16_t temp;
  float value;

  result = sensor.readHoldingRegisters(register_num, data_size);

  if (result == sensor.ku8MBSuccess)
  {
    for (j = 0; j < data_size; j++)
    {
      buf[j] = sensor.getResponseBuffer(j);
      Serial.print(buf[j]);
      Serial.print(" ");
    }

    // hand-assemble a single-precision float from the bytestream
    memcpy(&value, &buf, sizeof(float));
    Serial.println();
  }
  else {
    Serial.print("Failure. Code: ");
    Serial.println(result,HEX);
  }
  return value;
}

/*
LONG (Entero Largo de 32 bits):

| Formato:            |  Dirección de Registro:      |  Descripcion                                          |  Nota                                          |
|---------------------|------------------------------|-------------------------------------------------------|------------------------------------------------|
| Entero largo de 32  |  Puede ser parte de los      |  Almacena números de enteros de 32 bits con signo     |  En el caudalimetro, solo se lee el registro   |
| bits con signo      |  registros Holding o Input.  |  Es similar a INTEGER pero con un rango mas amplio.   |  Holding (readHoldingRegister(reg, size))      |
*/

int32_t MODBUS_FUNCTIONS::readLong(uint16_t register_num, uint16_t data_size) {
  int16_t j, result;
  int16_t buf[data_size];
  int32_t value;

  result = sensor.readHoldingRegisters(register_num, data_size);
  
  if (result == sensor.ku8MBSuccess)
  {
    for (j = 0; j < data_size; j++)
    {
      buf[j] = sensor.getResponseBuffer(j);
      Serial.print(buf[j]);
      Serial.print(" ");
    }

    // Combine the two 16-bit values into a 32-bit int
    value = ((int32_t)buf[1] << 16) | buf[0];
    
    Serial.println();
  }
  else {
    Serial.print("Failure. Code: ");
    Serial.println(result, HEX);
  }
  return value;
}

/*
readUnsignedLong es para registros sin signo, como por ejemplo el tiempo de trabajo en el registro 1487
*/

uint32_t MODBUS_FUNCTIONS::readUnsignedLong(uint16_t register_num, uint16_t data_size) {
  uint16_t j, result;
  uint32_t buf[data_size];
  uint32_t value;

  result = sensor.readHoldingRegisters(register_num, data_size);
  
  if (result == sensor.ku8MBSuccess)
  {
    for (j = 0; j < data_size; j++)
    {
      buf[j] = sensor.getResponseBuffer(j);
      Serial.print(buf[j]);
      Serial.print(" ");
    }

    // Combine the two 16-bit values into a 32-bit unsigned int
    value = ((uint32_t)buf[1] << 16) | buf[0];
    Serial.println();
  }
  else {
    Serial.print("Failure. Code: ");
    Serial.println(result, HEX);
  }
  return value;
}

/*
INTEGER (Entero de 16 bits):

| Formato:            |  Dirección de Registro:      |  Descripcion                                          |  Nota                                          |
|---------------------|------------------------------|-------------------------------------------------------|------------------------------------------------|
| Entero de 16        |  Puede ser parte de los      |  Almacena números de enteros de 16 bits con signo     |  En el caudalimetro, solo se lee el registro   |
| bits con signo      |  registros Holding o Input.  |                                                       |  Holding (readHoldingRegister(reg, size))      |
*/

int16_t MODBUS_FUNCTIONS::readInt(uint16_t register_num){
  uint8_t result;
  int16_t value;

  result = sensor.readHoldingRegisters(register_num, 1);

  if (result == sensor.ku8MBSuccess)
  {
      value = sensor.getResponseBuffer(0);
      Serial.print(value);
      Serial.println();
  }
  else {
    Serial.print("Failure. Code: ");
    Serial.println(result,HEX);
  }
  return value;
}


//  --->  AQUI SE AGREGARA LA TABLA DE DETALLES DEL REGISTRO DE TIPO BIT  <---


int MODBUS_FUNCTIONS::readBit (uint16_t error_code){
  uint8_t result;
  int value;
  
  result = sensor.readHoldingRegisters(error_code, 1);

  if (result == sensor.ku8MBSuccess)
  {
    value = sensor.getResponseBuffer(0);
    Serial.println(value);
  }
  else{
    Serial.print("Failure. Code: ");
    Serial.println(result, HEX);
  }
  return value;
}

