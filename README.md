# Encoder rotativo
Ejemplo de uso de encoder rotativo con (o sin) pulsador.

      
  Programa de test de encoder rotativo con (o sin) pulsador usando interrupciones y sin ellas.
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
