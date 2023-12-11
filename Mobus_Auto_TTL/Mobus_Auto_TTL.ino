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
| float    |  readFloat(uint8_t register_num, uint8_t data_size)          | probados: reg 1 - 33 - 113
| int32_t  |  readLong(uint16_t register_num, uint8_t data_size)          | probados: reg 9 
| uint32_t |  readUnsignedLong(uint16_t register_num, uint8_t data_size)  | probados: reg 103, 105
| int16_t  |  readInt(uint16_t register_num)                              | probados: reg 158, 1437
| void     |  writeReg(uint8_t register_num, uint16_t data)               | probados: reg 60 

Dev notes:
- Se elimino el swapBytes ya que en la mayoria de los registros causa problemas
- No he probado aun algun metodo para ingresar el registro con el numero que aparece en el manual
- Probando algunos registros, reinicie el totalizador (¿Como?, ¿Porque?)
*/

#include <HardwareSerial.h>
#include <ModbusMaster.h>

//  Registros obtenidos de (TUF-2000M Ultrasonic Flow Meter User Manual.pdf, seccion 7.1.1 MODBUS REGISTERS TABLE)

#define SLAVE_ID 1            //  ID de esclavo

//Registros tipo REAL4
#define FLOW_REGISTER 0       //  Registro de lectura del flujo
#define TEMPERATURE_IN_1 32   //  Registro de lectura de temperatura
#define POS_ACCUMULATOR 114   //  Registro de lestura del acumulador positivo

//Registros tipo LONG
#define WORKING_TIMER 102     //  Registro de lectura del tiempo de trabajo (en segundos sin signo)
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

HardwareSerial swSerial (PA3,PA2);  //  swSerial (RX_pin, TX_pin); stm32f103 ==> rx -> PA3 ; tx -> PA2
ModbusMaster sensor;                //  Inicializa el objeto de MODBUS

void setup()
{
  Serial.begin(115200);             //  Serial de comunicacion con el MCU stm32f103xxxx
  Serial.println("Welcome");
  swSerial.begin(9600);             //  Serial de comunicacón con el sensor
  delay(2000);
  sensor.begin(SLAVE_ID, swSerial); //  Comunicación con el esclavo MODBUS, mediante el serial swSerial
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
