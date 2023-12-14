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

Funciones agregadas (13/12/2023)

|  Tipo    |            Funcion ModBUs                                    | Notas
|----------|--------------------------------------------------------------|--------------------
|  float   |  readFloat(uint8_t register_num, uint8_t data_size)          | probados: reg 1 - 33 - 113
| int32_t  |  readLong(uint16_t register_num, uint8_t data_size)          | probados: reg 9 
| uint32_t |  readUnsignedLong(uint16_t register_num, uint8_t data_size)  | probados: reg 103, 105
| int16_t  |  readInt(uint16_t register_num)                              | probados: reg 158, 1437
|   void   |  writeReg(uint8_t register_num, uint16_t data)               | probados: reg 60, 59
| uint16_t |  readBit (uint16_t error_code)                               | probados: reg 72 ---> PRUEBAS INCONCLUSAS <---


|  Tipo    |            Funcion esteticas                                 | Notas
|----------|--------------------------------------------------------------|--------------------
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
- Se separan las funciones ModBus de las funciones estéticas.
*/

//  Registros obtenidos de (TUF-2000M Ultrasonic Flow Meter User Manual.pdf, seccion 7.1.1 MODBUS REGISTERS TABLE)
#include "registers.h"
#include "aesthetic.h"

AESTHETIC_FUNCTIONS aesthetic;

void setup()
{
  Serial.begin(115200);             //  Serial de comunicacion con el MCU stm32f103xxxx
  Serial.println("Welcome");
  delay(2000);

  //  Configuracion inicial, diametro y grosor del tubo
  aesthetic.GoToMenu(17);
  aesthetic.InputKey(51);
  aesthetic.InputKey(50);
  aesthetic.InputKey(61);
  aesthetic.GoToMenu(18);
  aesthetic.InputKey(50);
  aesthetic.InputKey(61);
  aesthetic.GoToMenu(37);
  delay(2000);
  aesthetic.GoToMenu(0);
  aesthetic.InputKey(61);
  Serial.println();
}

void loop() 
{
  Serial.print("Flujo: ");
  Serial.println(aesthetic.readFlow(), 4);
  delay(10);                                         //  Es importante agregar delay(); para que los registros se lean en orden y de forma correcta
  
  Serial.print("Acumulador positivo: ");
  Serial.println(aesthetic.readPosTot());
  delay(10);
  
  Serial.print("Temperatura: ");
  Serial.println(aesthetic.readTemp());
  delay(10);
  
  Serial.print("Tiempo de trabajo ");
  aesthetic.printTime(aesthetic.readTime());         //  Lectura del tiempo correcta, solucionado quitando byte swap. Tiempo de trabajo total (menu +1) 
  delay(10);                                         //  Se agrega la funcion de imprimir tiempo en formato hh:mm:ss
  
  Serial.print("Pos accumulator long ");
  Serial.println(aesthetic.readLongPosTot());
  delay(10);
  
  Serial.print("Flow Unit ");
  aesthetic.printFlowUnit(aesthetic.flowUnit());     //  Se agrega la funcion de imprimir la unidad de medida del flujo
  delay(10);
  
  Serial.print("Current display window: ");
  Serial.println(aesthetic.currentWindows());        //  El menu actual en el dispositivo se muestra en hexadecimal,  
  delay(10);  
  
  Serial.print("Estado del dispositivo: ");
  aesthetic.printErrorCode(aesthetic.errorCode());   //   Revision y comprobacion de los errores
  delay(10);
 
  Serial.println();
  delay(3000);
}
