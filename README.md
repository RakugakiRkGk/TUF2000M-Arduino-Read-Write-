# TUF2000M-Arduino con RS485 a TTL automático

Lectura y escritura de los registros del caudalímetro TUF2000M en MCU STM32F103CBT6

By: RakugakiRkGk

## INFORMACION IMPORTANTE.
- Este pograma se desarrolla de la base de "Nick Touran's MODBUS-reading ESP8266 code for the TUF-2000M Ultrasonic flow meter"
- Se utilizan las librerias `HardwareSerial` y `ModbusMaster`
- Se utiliza el convertidor RS485 a TTL automático (4 pínes TTL -- 3 Pines RS485)

## Informacion del sensor:
- El caudalimetro tiene de base la ID de esclavo = 1,
- Este sensor, solo admite 2 tipos de funciones MODBUS. 0x03 de lectura y 0x06 de escritura

| Funcion  |               Codigo                    |   Descripcion                                                           |
|----------|-----------------------------------------|-------------------------------------------------------------------------|
|   0x03   |  `readHoldingRegister(register, size);` |   Lee el registro requerido, y la cantidad de registros que lo componen |
|   0x06   |  `writeSingleRegister(register, data);` |   Envia el dato a escribir al registro de configuracion.                |

## Tipos de registros del caudalimetro:

|  **Tipo**   | **Descripcion**                       |
|-------------|---------------------------------------| 
|  **REAL4**  | 32 bits de punto flotante             |
|  **LONG**   | Entero largo de 32 Bits               |
| **INTEGER** | Entero de 16 bits                     |
|   **BCD**   | Codigo decimal codificado en binario  |
|   **BIT**   | Bit individual                        |

## Funciones agregadas (12/12/2023)

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

### Dev notes:
- Se elimino el swapBytes ya que en la mayoria de los registros causa problemas
- La lectura de bits, funciona, pero no estoy seguro que sea la correcta, mas pruebas para corroborar
- Se integra una configuracion inicial usando las funciones GoToMenu() y InputKey()
