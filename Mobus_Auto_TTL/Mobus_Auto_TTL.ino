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

Funciones agregadas (6/12/2023)

|  Tipo    |            Funcion                                           | Notas
|----------|--------------------------------------------------------------|--------------------
| float    |  readFloat(uint8_t register_num, uint8_t data_size)          | Se ingresa el registro como se ve en el manual (probados registros 1 - 33 - 113)
| int32_t  |  readLong(uint16_t register_num, uint8_t data_size)          | --->  NO TESTEADO  <---
| uint32_t |  readUnsignedLong(uint16_t register_num, uint8_t data_size)  | --->  PRUEBAS INCONCLUSAS  <--- 
| int16_t  |  readInt(uint16_t register_num)                              | Se ingresa el registro menos 1 (probados: reg 158, 1437 en manual  -->   157, 1436 en el codigo)
| void     |  writeReg(uint8_t register_num, uint16_t data)               | Se ingresa el registro menos 1 (probados: reg 60 en manual   -->   59 en el codigo)

Dev notes:
- No he probado aun algun metodo para ingresar el registro con el numero que aparece en el manual
- Aún no se prueba la funcion readLong()
- Probando algunos registros, reinicie el totalizador (¿Como?, ¿Porque?)
*/

#include <HardwareSerial.h>
#include <ModbusMaster.h>

//Registros obtenidos de (TUF-2000M Ultrasonic Flow Meter User Manual.pdf, seccion 7.1.1 MODBUS REGISTERS TABLE)

#define SLAVE_ID 1            //  ID de esclavo

//Registros tipo REAL4
#define FLOW_REGISTER 1       //  Registro de lectura del flujo
#define TEMPERATURE_IN_1 33   //  Registro de lectura de temperatura
#define POS_ACCUMULATOR 115   //  Registro de lestura del acumulador positivo

//Registros tipo LONG
#define WORKING_TIMER 103     //  Registro de lectura del tiempo de trabajo (en segundos)
#define POS_ACC_LONG 8        //  Registro de lectura del acumulador positivo tipo LONG

//Registros de configuracion
#define MENU_REG 59           //  Registro de configuracion Go To Windows # (Reg 60)
#define MENU_NUM 50           //  Data a escribir en el registro 60 (50 --> menu 31)

//Registros tipo INTEGER
#define FLOW_UNIT 1436        //  Registro de lectura de la unidad de medida del flujo (registro 1437 -- referirse a nota 5 en el manual seccion 7.1.1)
#define CURRENT_WINDOW 157    //  Registro de lectura del menú actual mostrado en el caudalimetro (registro 158)

//Cantidad de registros por tipo
#define REAL_DATA_SIZE 2      //  Tamaño del registro REAL4
#define LONG_DATA_SIZE 2      //  Tamaño del registro LONG

HardwareSerial swSerial (PA3,PA2);  //swSerial (RX_pin, TX_pin); stm32f103 ==> rx -> PA3 ; tx -> PA2
ModbusMaster sensor;                //Inicializa el objeto de MODBUS

void setup()
{
  Serial.begin(115200);             //Serial de comunicacion con el MCU stm32f103xxxx
  Serial.println("Welcome");
  swSerial.begin(9600);             //Serial de comunicacón con el sensor
  delay(2000);
  sensor.begin(SLAVE_ID, swSerial); //Comunicación con el esclavo MODBUS, mediante el serial swSerial
}

void loop() 
{
  Serial.print("Flujo: ");
  Serial.println(readFloat(FLOW_REGISTER, REAL_DATA_SIZE), 4);
  delay(10);                                                        //Es importante agregar delay(); si los registros no se están leyendo de forma correcta
  Serial.print("Acumulador: ");
  Serial.println(readFloat(POS_ACCUMULATOR, REAL_DATA_SIZE));
  delay(10);
  Serial.print("Temperatura: ");
  Serial.println(readFloat(TEMPERATURE_IN_1, REAL_DATA_SIZE));
  delay(10);
  Serial.print("Tiempo de trabajo ");
  Serial.println(readUnsignedLong(WORKING_TIMER, LONG_DATA_SIZE));  //El registro 103 funciona, pero se satura en 2^(16) y se reinicia. Tiempo de trabajo total (menu +1) 
  delay(10);
  // Serial.print("Pos accumulator long ");
  // Serial.println(readLong(POS_ACC_LONG, LONG_DATA_SIZE));
  // delay(10);
  Serial.print("Flow Unit ");
  Serial.println(readInt(FLOW_UNIT));
  delay(10);
  Serial.print("Current display window: ");
  Serial.println(readInt(CURRENT_WINDOW),HEX);                      //  La ventana actual se muestra en Hexadecimal,  
  delay(10);  
  // Serial.print("Go to Menu: ");
  // writeReg(MENU_REG, MENU_NUM);
  // delay(10);
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
  Serial.print (result);
  //Comprobacion de lectura correcta
  if (result == sensor.ku8MBSuccess)
  {
    for (j = 0; j < data_size; j++)
    {
      buf[j] = sensor.getResponseBuffer(j);
      Serial.print(buf[j]);
      Serial.print(" ");
    }
    // swap bytes because the data comes in Big Endian!
    temp = buf[1];
    buf[1]=buf[0];
    buf[0]=temp;
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

    // swap bytes because the data comes in Big Endian!
    int16_t temp = buf[1];
    buf[1] = buf[0];
    buf[0] = temp;

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

    // swap bytes because the data comes in Big Endian!
    uint16_t temp = buf[1];
    buf[1] = buf[0];
    buf[0] = temp;

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
  int16_t buf[1];
  int16_t value;

  result = sensor.readHoldingRegisters(register_num, 1);
  Serial.print (result);
  if (result == sensor.ku8MBSuccess)
  {
      buf[0] = sensor.getResponseBuffer(0);
      Serial.print(buf[0]);
      Serial.print(" ");
      value = buf [0];
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