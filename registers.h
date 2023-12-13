#define SLAVE_ID 1            //  ID de esclavo

//Registros tipo REAL4
#define FLOW_REGISTER 0       //  Registro de lectura del flujo
#define TEMPERATURE_IN_1 32   //  Registro de lectura de temperatura
#define POS_ACCUMULATOR 114   //  Registro de lestura del acumulador positivo

//Registros tipo LONG
#define WORKING_TIMER 102     //  Registro de lectura del tiempo de trabajo (en segundos sin signo)
#define POS_ACC_LONG 8        //  Registro de lectura del acumulador positivo tipo LONG

//Registros de configuracion
#define MENU_REG 59           //  Registro de configuracion Go To Windows# 
#define KEY_TO_INPUT 58       //  Registro de configuracion para controlar el caudalimetro. Referirse a seccion 7.4 KEY VALUE TABLE del manual

//Registros tipo INTEGER
#define FLOW_UNIT 1436        //  Registro de lectura de la unidad de medida del flujo (registro 1437 -- referirse a nota 5 en el manual, seccion 7.1.1)
#define CURRENT_WINDOW 157    //  Registro de lectura del menú actual mostrado en el caudalimetro (registro 158)

#define ERROR_CODE 71         //  Registro de codigos de error. Referirse a nota 4 del manual, seccion 7.1.1

//Cantidad de registros por tipo
#define REAL_DATA_SIZE 2      //  Tamaño del registro REAL4
#define LONG_DATA_SIZE 2      //  Tamaño del registro LONG
// #define INT_DATA_SIZE 1       //  Tamaño del registro INTEGER ---> UNUSED <---
// #define BIT_DATA_SIZE 1       //  Tamaño del registro BIT ---> UNUSED <---