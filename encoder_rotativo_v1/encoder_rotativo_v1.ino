/*
#       _\|/_   A ver..., ¿que tenemos por aqui?
#       (O-O)        
# ---oOO-(_)-OOo---------------------------------
 
 
##########################################################
# ****************************************************** #
# *           DOMOTICA PARA PRINCIPIANTES              * #
# *            Test Encoder con pulsador               * #
# *          Autor:  Eulogio López Cayuela             * #
# *                                                    * #
# *       Versión 1.0       Fecha: 19/01/2021          * #
# ****************************************************** #
##########################################################
*/



/*
      ===== NOTAS DE LA VERSION ===== 
      
      Programa de test de encoder rotativo con pulsador usando interrupciones y sin ellas
      El giro incrementa y decrementa un valor.
      La pulsacion resetea dicho valor.

      Este sketch  contiene una funcion para la lectura del pulsador 
      y tres para la la lectura del giro del encoder:
      
        leerEncoder_v1():   acceso tipico con digitalRead a los pines DT y CLK del encoder
                            (tarda aprox. 5.28 us en ejecutarse)
        
        leerEncoder_v2():   lee los pines DT y CLK accediendo al puerto y obteniendo los bit correspondientes 
                            (tarda aprox. 1.45 us en ejecutarse)
        
        updateEncoder_ISR():  lee los pines DT y CLK accediendo al puerto. No devuelve nada, 
                              se usa como atenciona interrupcion es la mas eficaz y la recomendada
                              (tarda aprox. 0.95 us en ejecutarse).
                              Utilizamos unicamente una de las lineas de interrupcion.
                              Podriamos usar la otra para el pulsador (aunque este no es demasiado problematico en su lectura)
                              o destinarla a alguna otra señal que sea de vital importancia no perder de vista.

        Leer el giro del encoder reiteradamente desde el loop puede ocasionar que perdamos lecturas 
        o que las interpretemos en el sentido equivocado si el ciclo loop() tarda demasiado en ejecutarse
        Tiempos de ejecucion de loop de mas de 2 o 3 milisegundos pueden ser problematicos
        sobre todo si el giro del encoder es demasido rapido.
        Por tanto la opcion de interrupcion es la que debemos usar. 
        Queden el resto de opciones como ejemplo que pueden ser utilizadas en programas muy sencillos 
        sin apenas codigo que retrase la ejecucion de loop().
*/



/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//        IMPORTACION DE LIBRERIAS 
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/
                
#include <LiquidCrystal_I2C.h> 



/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//        SECCION DE DEFINICIONES UTILES
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

#define LCD_AZUL_ADDR    0x3F   // Direccion I2C tipica de los LCD con retroiluminacion color azul
#define LCD_VERDE_ADDR   0x27   // Direccion I2C tipica de los LCD con retroiluminacion color verde


/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//        MAPA DE PINES 
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

#define PIN_LED_OnBoard         13
#define PIN_ENCODER_CLK          2
#define PIN_ENCODER_DT           3
#define PIN_ENCODER_SW           4  

#define PUERTO_ENCODER        PIND  // puerto donde esta conectado el encoder
#define BIT_ENCODER_CLK          2
#define BIT_ENCODER_DT           3 
#define BIT_ENCODER_SW           4  


/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//        SECCION DE DECLARACION DE CONSTANTES  Y  VARIABLES GLOBALES
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

volatile int incrementoEncoder = 0;
int valor_mostrado = 0; 


/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//         CREACION DE OBJETOS
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

LiquidCrystal_I2C LCD1602 ( LCD_VERDE_ADDR , 16 , 2 );



/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//        PROTOTIPADO DE FUNCIONES
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

uint8_t leer_pulsador_encoder( void );    // 0.82 us - 1.13 us  (sin/con pulsacion) 
void updateEncoder_ISR( void );           // 0.95 us
int8_t leerEncoder_v1( void );            // 5.28 us
int8_t leerEncoder_v2( void );            // 1.45 us




/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
   ****************************************************************************************************** 
                                    FUNCION DE CONFIGURACION
   ****************************************************************************************************** 
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

void setup()
{

  Serial.begin(115200);
          
  pinMode(PIN_ENCODER_SW, INPUT_PULLUP);
  pinMode(PIN_ENCODER_CLK, INPUT);
  pinMode(PIN_ENCODER_DT, INPUT);  

  
  pinMode(PIN_LED_OnBoard, OUTPUT);				  // declarar el pin 13 como salida
  digitalWrite(PIN_LED_OnBoard, LOW); 			// apagar el led del pin 13

  /* cometr si no se desea usar un LCD */  
  LCD1602.begin();
  LCD1602.clear();
  LCD1602.print(F("Valor: "));
  LCD1602.print(valor_mostrado);
  
  attachInterrupt(digitalPinToInterrupt(BIT_ENCODER_CLK), updateEncoder_ISR, CHANGE);  //linea CLK del encoder
  
  delay(200);
}



/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
   ****************************************************************************************************** 
                                  BUCLE PRINCIPAL DEL PROGRAMA
   ****************************************************************************************************** 
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

void loop()
{ 
  boolean FLAG_update_lcd = false;
  
  uint8_t pulsador = leer_pulsador_encoder(); 

  /* comentar la linea de interrupcion del setup para probar estas */
  //incrementoEncoder = leerEncoder_v1();   // (digitalRead) 
  //incrementoEncoder = leerEncoder_v2();   // (port)
  
  //delay(5);  //simular un loop de mucha duracion, por lectura de pines analogicos y sensores...
  
  if(pulsador!=0){ 
    FLAG_update_lcd = true;
    valor_mostrado=0; //reset del valor mostrado al pulsar
  }

  if(incrementoEncoder!=0){
    FLAG_update_lcd = true;
    valor_mostrado+=incrementoEncoder;
    incrementoEncoder=0;
  }
  
  if( FLAG_update_lcd ){ 
    /* Comentar las lineas relativas al LCD y descomentar  esta si no se usa display */
    //Serial.print(F("valor: "));Serial.println(valor_mostrado);
    
    LCD1602.setCursor(7, 0);
    LCD1602.print(valor_mostrado);
    LCD1602.print(F("   ")); //borrar restos de impresiones anteriores
  }
}



/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx 
        BLOQUE DE FUNCIONES: LECTURAS DE SENSORES, COMUNICACION SERIE, INTERRUPCIONES...
   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx 
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/


/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//       FUNCIONES ENCODER
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

//========================================================
//  FUNCION PARA LEER PULSADOR DEL ENCODER   0.82 us - 1.13 us  (sin/con pulsacion) 
//========================================================

uint8_t leer_pulsador_encoder()
{
  static uint8_t estadoSW_old = 1;
  uint8_t estadoSW;
    //estadoSW = digitalRead(PIN_ENCODER_SW)?0:1;  
    estadoSW = bitRead(PUERTO_ENCODER, BIT_ENCODER_SW)?0:1;
    if(estadoSW && !estadoSW_old){
      estadoSW_old=estadoSW;
      return 1;
    }
    estadoSW_old=estadoSW;
    return 0;
}



//========================================================
//  FUNCION PARA LEER ENCODER ROTATORIO (solo para ISR) (USANDO PUERTOS) 0.95 us
//========================================================

void updateEncoder_ISR()
{ 
  int8_t currentStateCLK = bitRead(PUERTO_ENCODER, BIT_ENCODER_CLK);
  static int8_t lastStateCLK = 0;

  if ( currentStateCLK != lastStateCLK ){
    if ( bitRead(PUERTO_ENCODER, BIT_ENCODER_DT)!= currentStateCLK ) { incrementoEncoder++; }  
    else { incrementoEncoder--; }
  }
  /* Recordar el estado actual */
  lastStateCLK = currentStateCLK;
}


//========================================================
//  FUNCION PARA LEER ENCODER ROTATORIO (digitalRead)  5.28 us
//========================================================
int8_t leerEncoder_v1()  
{
  static uint8_t estadoAnteriorCLK = 1;
  uint8_t estadoCLK = digitalRead(PIN_ENCODER_CLK);  
  int8_t _sentidoGiroEncoder = 0;
  
  if ( estadoCLK != estadoAnteriorCLK ) {
    if (digitalRead(PIN_ENCODER_DT) != estadoCLK)  { _sentidoGiroEncoder = 1;  }
    else { _sentidoGiroEncoder = -1;  } 
  }
  estadoAnteriorCLK = estadoCLK;   // Guardar valores para siguiente

  return _sentidoGiroEncoder;
}


//========================================================
//  FUNCION PARA LEER ENCODER ROTATORIO - (usando puertos)   1.45 us
//========================================================
int8_t leerEncoder_v2()  
{
  static uint8_t estadoAnteriorCLK = 1;
  uint8_t estadoCLK = bitRead(PUERTO_ENCODER, BIT_ENCODER_CLK);
  int8_t _sentidoGiroEncoder = 0;
  
  if ( estadoCLK != estadoAnteriorCLK ) {
    if (bitRead(PUERTO_ENCODER, BIT_ENCODER_DT) != estadoCLK)  { _sentidoGiroEncoder = 1;  }
    else { _sentidoGiroEncoder = -1;  } 
  }
  estadoAnteriorCLK = estadoCLK;   // Guardar valores para siguiente

  return _sentidoGiroEncoder;
}



//*******************************************************
//                    FIN DE PROGRAMA
//*******************************************************
