# TUF2000M-Arduino con RS485 a TTL automático

Lectura y escritura de los registros del caudalímetro TUF2000M en MCU STM32F103CBT6

By: RakugakiRkGk

## INFORMACION IMPORTANTE.
- Este pograma se desarrolla de la base de "Nick Touran's MODBUS-reading ESP8266 code for the TUF-2000M Ultrasonic flow meter"
- Se utilizan las librerias <HardwareSerial> y <ModbusMaster>
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

## Funciones agregadas (6/12/2023)

| **Tipo** |            **Funcion**                                         | **Notas**
|----------|----------------------------------------------------------------|--------------------
| float    |  `readFloat(uint8_t register_num, uint8_t data_size)`          | Se ingresa el registro como se ve en el manual (probados registros 1 - 33 - 113)
| int32_t  |  `readLong(uint16_t register_num, uint8_t data_size)`          | --->  NO TESTEADO  <---
| uint32_t |  `readUnsignedLong(uint16_t register_num, uint8_t data_size)`  | --->  PRUEBAS INCONCLUSAS  <--- 
| int16_t  |  `readInt(uint16_t register_num)`                              | Se ingresa el registro menos 1 (probados: reg 158, 1437 en manual  -->   157, 1436 en el codigo)
| void     |  `writeReg(uint8_t register_num, uint16_t data)`               | Se ingresa el registro menos 1 (probados: reg 60 en manual   -->   59 en el codigo)

### Dev notes:
- No he probado aun algun metodo para ingresar el registro con el numero que aparece en el manual
- Aún no se prueba la funcion readLong()
- Probando algunos registros, reinicie el totalizador (¿Como?, ¿Porque?)
