/*
Lectura y escritura de los registros del caudalímetro TUF2000M en MCU STM32F103CBT6
By: RakugakiRkGk

INFORMACION IMPORTANTE.
- Este pograma se desarrolla de la base de "Nick Touran's MODBUS-reading ESP8266 code for the TUF-2000M Ultrasonic flow meter"
- Se utilizan las librerias <HardwareSerial> y <ModbusMaster>
- Se utiliza el convertidor RS485 a TTL automático (4 pínes TTL -- 3 Pines RS485)


Informacion del sensor:
- El caudalimetro tiene de base la ID de esclavo = 1,
- Este sensor, solo admite 2 tipos de funciones MODBUS. 0x03 de lectura y 0x06 de escritura

| Funcion  |               Codigo                  |   Descripcion                                                                              |
|----------|---------------------------------------|--------------------------------------------------------------------------------------------|
|   0x03   |  readHoldingRegister(register, size); |   Se lee la informacion del registro requerido, y la cantidad de registros que lo componen |
|   0x06   |  writeSingleRegister(register, data); |   Envia el dato a escribir al registro de configuracion.                                   |

Tipos de registros del caudalimetro:

|  Tipo   |  Descripcion                          |
|---------|---------------------------------------| 
|  REAL4  | 32 bits de punto flotante             |
|  LONG   | Entero largo de 32 Bits               |
| INTEGER | Entero de 16 bits                     |
|   BCD   | Codigo decimal codificado en binario  |
|   BIT   | Bit individual                        |

Funciones agregadas (12/12/2023)

|  Tipo    |            Funcion                                           | Notas
|----------|--------------------------------------------------------------|--------------------
|  float   |  readFloat(uint8_t register_num, uint8_t data_size)          | probados: reg 1 - 33 - 113
| int32_t  |  readLong(uint16_t register_num, uint8_t data_size)          | probados: reg 9 
| uint32_t |  readUnsignedLong(uint16_t register_num, uint8_t data_size)  | probados: reg 103, 105
| int16_t  |  readInt(uint16_t register_num)                              | probados: reg 158, 1437
|   void   |  writeReg(uint8_t register_num, uint16_t data)               | probados: reg 60, 59
| uint16_t |  readBit (uint16_t error_code)                               | probados: reg 72 ---> PRUEBAS INCONCLUSAS <---
|   void   |  printTime (uint32_t TotalSeconds)                           | formato hh:mm:ss
|   void   |  printFlowUnit (uint16_t flowUnit)                           | casos 0 - 31
|   void   |  printErrorCode (uint16_t error_code)                        | casos 0 - 15
|   void   |  GoToMenu(uint16_t menu)                                     | menu ingresado en decimal
|   void   |  InputKey(uint16_t key)                                      | Tabla 7.4 del manual

Dev notes:
- Se elimino el swapBytes ya que en la mayoria de los registros causa problemas
- Probando algunos registros, reinicie el totalizador (¿Como?, ¿Porque?) (posible respuesta en las nuevas funciones de configuracion)
- La lectura de bits, funciona, pero no estoy seguro que sea la correcta, mas pruebas para corroborar
- Se integra una configuracion inicial usando las funciones GoToMenu() y InputKey()
*/

#include <HardwareSerial.h>
#include <ModbusMaster.h>

//  Registros obtenidos de (TUF-2000M Ultrasonic Flow Meter User Manual.pdf, seccion 7.1.1 MODBUS REGISTERS TABLE)
#include "registers.h"

HardwareSerial swSerial (PA3,PA2);  //  swSerial (RX_pin, TX_pin); stm32f103 ==> rx -> PA3 ; tx -> PA2
ModbusMaster sensor;                //  Inicializa el objeto de MODBUS

void setup()
{
  Serial.begin(115200);             //  Serial de comunicacion con el MCU stm32f103xxxx
  Serial.println("Welcome");
  swSerial.begin(9600);             //  Serial de comunicacón con el sensor
  delay(2000);
  sensor.begin(SLAVE_ID, swSerial); //  Comunicación con el esclavo MODBUS, mediante el serial swSerial

  //  Configuracion inicial, diametro y grosor del tubo
  GoToMenu(17);
  InputKey(51);
  InputKey(50);
  InputKey(61);
  GoToMenu(18);
  InputKey(50);
  InputKey(61);
  GoToMenu(37);
}

void GoToMenu(uint16_t menu){
  writeReg(MENU_REG,menu);
  delay(10);
}
void InputKey(uint16_t key){
  writeReg(KEY_TO_INPUT, key);
  delay(10);
}

void loop() 
{
  Serial.print("Flujo: ");
  Serial.println(readFloat(FLOW_REGISTER, REAL_DATA_SIZE), 4);
  delay(10);                                                        //  Es importante agregar delay(); si los registros no se están leyendo de forma correcta
  Serial.print("Acumulador: ");
  Serial.println(readFloat(POS_ACCUMULATOR, REAL_DATA_SIZE));
  delay(10);
  Serial.print("Temperatura: ");
  Serial.println(readFloat(TEMPERATURE_IN_1, REAL_DATA_SIZE));
  delay(10);
  Serial.print("Tiempo de trabajo ");
  printTime(readUnsignedLong(WORKING_TIMER, LONG_DATA_SIZE));       //  Lectura del tiempo correcta, solucionado quitando byte swap. Tiempo de trabajo total (menu +1) 
  delay(10);                                                        //  Se agrega la funcion de imprimir tiempo en formato hh:mm:ss
  Serial.print("Pos accumulator long ");
  Serial.println(readLong(POS_ACC_LONG, LONG_DATA_SIZE));
  delay(10);
  Serial.print("Flow Unit ");
  printFlowUnit(readInt(FLOW_UNIT));                                //  Se agrega la funcion de imprimir la unidad de medida del flujo
  delay(10);
  Serial.print("Current display window: ");
  Serial.println(readInt(CURRENT_WINDOW),HEX);                      //  El menu actual en el dispositivo se muestra en hexadecimal,  
  delay(10);  
  Serial.print("Estado del dispositivo: ");
  printErrorCode(readBit(ERROR_CODE));                              //   Revision y comprobacion de los errores
  delay(10);
  Serial.println();
  delay(3000);
}

/*
REAL4 (32 bits en punto flotante):

| Formato:            |  Dirección de Registro:      |  Descripcion                                          |  Nota                                          |
|---------------------|------------------------------|-------------------------------------------------------|------------------------------------------------|
| IEEE 754 de 32bits  |  Puede ser parte de los      |  Almacena números de punto flotante de 32 bits        |  En el caudalimetro, solo se lee el registro   |
|                     |  registros Holding o Input.  |  Es adecuado para representar valores con decimales.  |  Holding (readHoldingRegister(reg, size))      |
*/
float readFloat(uint16_t register_num, uint16_t data_size) {
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

int32_t readLong(uint16_t register_num, uint16_t data_size) {
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

uint32_t readUnsignedLong(uint16_t register_num, uint16_t data_size) {
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

int16_t readInt(uint16_t register_num){
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

/*
writeReg() es una funcion que escribe o hace una configuración en el sensor
se utiliza writeSingleRegister(register, data) para realizar la operacion
*/

void writeReg(uint16_t register_num, uint16_t data){
  uint16_t result = sensor.writeSingleRegister(register_num, data);

  if (result == sensor.ku8MBSuccess){
    Serial.println("write success");
    Serial.println(result, HEX);
  }
  else{
    Serial.print("Write Failure. Code: ");
    Serial.println(result, HEX);
  }
}

uint16_t readBit (uint16_t error_code){
  uint8_t result;
  uint16_t value;
  
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

void printTime (uint32_t TotalSeconds){
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

void printFlowUnit (uint16_t flowUnit){
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

void printErrorCode (uint16_t error_code){
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
