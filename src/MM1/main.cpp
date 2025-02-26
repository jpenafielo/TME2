/* Definiciones externas para el sistema de colas simple */

#include "lcgrand.h" /* Encabezado para el generador de numeros aleatorios */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "libxl.h"

#define LIMITE_COLA 1000 /* Capacidad maxima de la cola */
#define OCUPADO 1        /* Indicador de Servidor Ocupado */
#define LIBRE 0          /* Indicador de Servidor Libre */

int sig_tipo_evento, num_clientes_espera, num_esperas_requerido, num_eventos,
    num_entra_cola, estado_servidor;
float area_num_entra_cola, area_estado_servidor, media_entre_llegadas,
    media_atencion, tiempo_simulacion, tiempo_llegada[LIMITE_COLA + 1],
    tiempo_ultimo_evento, tiempo_sig_evento[3], total_de_esperas;
FILE *parametros, *resultados;

void inicializar(void);
void controltiempo(void);
void llegada(void);
void salida(void);
void reportes(void);
void actualizar_estad_prom_tiempo(void);
float expon(float mean);

// Write and log events to excel
using namespace libxl;

/* Estructura útil para el registro de eventos y resultados de la simulación */
struct ReporteXLS
{
  Book *book;
  Sheet *tiempos;
  Sheet *registro_eventos;
  int fila_tiempo_llegada_actual;
  int fila_eventos_actual;
  int fila_tiempo_salida_actual;
} *reporte_xls;

int main(void) /* Funcion Principal */
{

  /* Inicializa la simulacion. */

  inicializar();

  /* Corre la simulacion mientras no se llegue al numero de clientes
   * especificado en el archivo de entrada*/

  while (num_clientes_espera < num_esperas_requerido)
  {

    /* Determina el siguiente evento */

    controltiempo();

    /* Actualiza los acumuladores estadisticos de tiempo promedio */

    actualizar_estad_prom_tiempo();

    /* Invoca la funcion del evento adecuado. */

    switch (sig_tipo_evento)
    {
    case 1:
      llegada();
      break;
    case 2:
      salida();
      break;
    }
  }

  /* Invoca el generador de reportes y termina la simulacion. */

  reportes();

  fclose(parametros);
  fclose(resultados);

  return 0;
}

void inicializar(void) /* Funcion de inicializacion. */
{

  /* Inicializa el reloj de la simulacion. */

  tiempo_simulacion = 0.0;

  /* Inicializa las variables de estado */

  estado_servidor = LIBRE;
  num_entra_cola = 0;
  tiempo_ultimo_evento = 0.0;

  /* Inicializa los contadores estadisticos. */

  num_clientes_espera = 0;
  total_de_esperas = 0.0;
  area_num_entra_cola = 0.0;
  area_estado_servidor = 0.0;

  /* Inicializando variables de registro de eventos y reporte de tiempos en Hoja de cálculo */

  reporte_xls = (struct ReporteXLS *)malloc(sizeof(struct ReporteXLS));
  reporte_xls->book = xlCreateBook();

  /*  Cargar hoja de cálculo con resultados */

  if (reporte_xls->book)
  {
    reporte_xls->tiempos = reporte_xls->book->addSheet("Tiempos de clientes");
    reporte_xls->registro_eventos = reporte_xls->book->addSheet("Registro de eventos");

    /*
      Las filas empiezan a contar desde 2, excluyendo el título de la librería usada
      y el título de las columnas que se genera a continuación
    */

    reporte_xls->fila_tiempo_llegada_actual = 2;
    reporte_xls->fila_tiempo_salida_actual = 2;
    reporte_xls->fila_eventos_actual = 2;

    if (reporte_xls->tiempos)
    {
      reporte_xls->tiempos->writeStr(1, 1, "Número de cliente");
      reporte_xls->tiempos->writeStr(1, 2, "Diferencia de tiempo de llegada");
      reporte_xls->tiempos->writeStr(1, 3, "Tiempo de atención");
    }

    if (reporte_xls->registro_eventos)
    {
      reporte_xls->registro_eventos->writeStr(1, 1, "Tiempo");
      reporte_xls->registro_eventos->writeStr(1, 2, "Tipo");
      reporte_xls->registro_eventos->writeStr(1, 3, "Estado del servidor");
      reporte_xls->registro_eventos->writeStr(1, 4, "Número de clientes en cola");
    }
  }

  /*** Lectura de parámetros del sistema ***/

  /* Abre los archivos de entrada y salida */

  parametros = fopen("param.txt", "r");
  resultados = fopen("result.txt", "w");

  /* Especifica el numero de eventos para la funcion controltiempo. */

  num_eventos = 2;

  /* Lee los parametros de enrtrada. */

  fscanf(parametros, "%f %f %d", &media_entre_llegadas, &media_atencion,
         &num_esperas_requerido);

  /* Inicializa la lista de eventos. Ya que no hay clientes, el evento salida
     (terminacion del servicio) no se tiene en cuenta */

  float diferencia_tiempo_llegada = expon(media_entre_llegadas);
  tiempo_sig_evento[1] = tiempo_simulacion + diferencia_tiempo_llegada;
  tiempo_sig_evento[2] = 1.0e+30;

  /* Registrar tiempo de llegada para éste cliente */

  if (reporte_xls->tiempos)
  {
    reporte_xls->tiempos->writeNum(
        reporte_xls->fila_tiempo_llegada_actual,
        1,
        reporte_xls->fila_tiempo_llegada_actual - 1);

    reporte_xls->tiempos->writeNum(
        reporte_xls->fila_tiempo_llegada_actual++,
        2,
        diferencia_tiempo_llegada);
  }
}

void controltiempo(void) /* Funcion controltiempo */
{
  int i;
  float min_tiempo_sig_evento = 1.0e+29;

  sig_tipo_evento = 0;

  /*  Determina el tipo de evento del evento que debe ocurrir. */

  for (i = 1; i <= num_eventos; ++i)
    if (tiempo_sig_evento[i] < min_tiempo_sig_evento)
    {
      min_tiempo_sig_evento = tiempo_sig_evento[i];
      sig_tipo_evento = i;
    }

  /* Revisa si la lista de eventos esta vacia. */

  if (sig_tipo_evento == 0)
  {

    /* La lista de eventos esta vacia, se detiene la simulacion. */

    fprintf(resultados, "\nLa lista de eventos esta vacia %f",
            tiempo_simulacion);
    exit(1);
  }

  /* TLa lista de eventos no esta vacia, adelanta el reloj de la simulacion. */

  tiempo_simulacion = min_tiempo_sig_evento;
}

void llegada(void) /* Funcion de llegada */
{
  float espera;

  /* Revisa si el servidor esta OCUPADO. */

  if (estado_servidor == OCUPADO)
  {

    /* Servidor OCUPADO, aumenta el numero de clientes en cola */

    ++num_entra_cola;

    /* Verifica si hay condici�n de desbordamiento */

    if (num_entra_cola > LIMITE_COLA)
    {

      /* Se ha desbordado la cola, detiene la simulacion */

      fprintf(resultados,
              "\nDesbordamiento del arreglo tiempo_llegada a la hora");
      fprintf(resultados, "%f", tiempo_simulacion);
      exit(2);
    }

    /* Todavia hay espacio en la cola, se almacena el tiempo de llegada del
            cliente en el ( nuevo ) fin de tiempo_llegada */

    tiempo_llegada[num_entra_cola] = tiempo_simulacion;
  }

  else
  {

    /*  El servidor esta LIBRE, por lo tanto el cliente que llega tiene tiempo
       de eespera=0 (Las siguientes dos lineas del programa son para claridad, y
       no afectan el reultado de la simulacion ) */

    espera = 0.0;
    total_de_esperas += espera;

    /* Incrementa el numero de clientes en espera, y pasa el servidor a ocupado
     */
    ++num_clientes_espera;
    estado_servidor = OCUPADO;

    /* Programa una salida ( servicio terminado ). */

    float tiempo_de_atencion = expon(media_atencion);
    tiempo_sig_evento[2] = tiempo_simulacion + tiempo_de_atencion;

    /* Registrar tiempo de atención para éste cliente */

    if (reporte_xls->tiempos)
    {
      reporte_xls->tiempos->writeNum(
          reporte_xls->fila_tiempo_salida_actual++,
          3,
          tiempo_de_atencion);
    }
  }

  /* Registrar tiempo entre llegadas para éste cliente */

  float diferencia_tiempo_llegada = expon(media_entre_llegadas);

  if (reporte_xls->tiempos)
  {
    reporte_xls->tiempos->writeNum(
        reporte_xls->fila_tiempo_llegada_actual,
        1,
        reporte_xls->fila_tiempo_llegada_actual - 1);

    reporte_xls->tiempos->writeNum(
        reporte_xls->fila_tiempo_llegada_actual++,
        2,
        diferencia_tiempo_llegada);
  }

  /* Registrar ocurrencia de evento */
  if (reporte_xls->registro_eventos)
  {
    reporte_xls->registro_eventos->writeNum(
        reporte_xls->fila_eventos_actual, 1, tiempo_simulacion);
    reporte_xls->registro_eventos->writeStr(
        reporte_xls->fila_eventos_actual, 2, "Llegada");
    reporte_xls->registro_eventos->writeStr(
        reporte_xls->fila_eventos_actual, 3, (estado_servidor == OCUPADO) ? "Ocupado" : "Libre");
    reporte_xls->registro_eventos->writeNum(
        reporte_xls->fila_eventos_actual++, 4, num_entra_cola);
  }

  /* Programa la siguiente llegada. */

  tiempo_sig_evento[1] = tiempo_simulacion + diferencia_tiempo_llegada;
}

void salida(void) /* Funcion de Salida. */
{
  int i;
  float espera;

  /* Revisa si la cola esta vacia */

  if (num_entra_cola == 0)
  {

    /* La cola esta vacia, pasa el servidor a LIBRE y
    no considera el evento de salida */

    estado_servidor = LIBRE;
    tiempo_sig_evento[2] = 1.0e+30;
  }
  else
  {
    float tiempo_de_atencion = expon(media_atencion);

    /* La cola no esta vacia, disminuye el numero de clientes en cola. */
    --num_entra_cola;

    /*Calcula la espera del cliente que esta siendo atendido y
    actualiza el acumulador de espera */

    espera = tiempo_simulacion - tiempo_llegada[1];
    total_de_esperas += espera;

    /* Incrementa el numero de clientes en espera */
    ++num_clientes_espera;

    /*
      Mueve cada cliente en la cola ( si los hay ) una posicion hacia adelante
     */
    for (i = 1; i <= num_entra_cola; ++i)
      tiempo_llegada[i] = tiempo_llegada[i + 1];

    /* Registrar tiempo de atención para éste cliente */

    if (reporte_xls->tiempos)
    {
      reporte_xls->tiempos->writeNum(
          reporte_xls->fila_tiempo_salida_actual++,
          3,
          tiempo_de_atencion);
    }

    /* Programar siguiente salida */
    tiempo_sig_evento[2] = tiempo_simulacion + tiempo_de_atencion;
  }

  /* Registrar ocurrencia de evento */

  if (reporte_xls->registro_eventos)
  {
    reporte_xls->registro_eventos->writeNum(
        reporte_xls->fila_eventos_actual, 1, tiempo_simulacion);
    reporte_xls->registro_eventos->writeStr(
        reporte_xls->fila_eventos_actual, 2, "Salida");
    reporte_xls->registro_eventos->writeStr(
        reporte_xls->fila_eventos_actual, 3, (estado_servidor == OCUPADO) ? "Ocupado" : "Libre");
    reporte_xls->registro_eventos->writeNum(
        reporte_xls->fila_eventos_actual++, 4, num_entra_cola);
  }
}

void reportes(void) /* Funcion generadora de reportes. */
{
  /* Escribe en el archivo de salida los encabezados del reporte y los
   * parametros iniciales */

  fprintf(resultados, "Sistema de Colas Simple\n\n");
  fprintf(resultados, "Tiempo promedio de llegada%11.3f minutos\n\n",
          media_entre_llegadas);
  fprintf(resultados, "Tiempo promedio de atencion%16.3f minutos\n\n",
          media_atencion);
  fprintf(resultados, "Numero de clientes%14d\n\n", num_esperas_requerido);

  /* Calcula y estima los estimados de las medidas deseadas de desempe�o */
  fprintf(resultados, "\n\nEspera promedio en la cola%11.3f minutos\n\n",
          total_de_esperas / num_clientes_espera);
  fprintf(resultados, "Numero promedio en cola%10.3f\n\n",
          area_num_entra_cola / tiempo_simulacion);
  fprintf(resultados, "Uso del servidor%15.3f\n\n",
          area_estado_servidor / tiempo_simulacion);
  fprintf(resultados, "Tiempo de terminacion de la simulacion%12.3f minutos",
          tiempo_simulacion);

  /* Guardar reporte de registro de eventos en hoja de cálculo */
  if (reporte_xls->book)
  {
    reporte_xls->book->save("logResultados.xls");
    reporte_xls->book->release();
  }
}

void actualizar_estad_prom_tiempo(void)
/*
  Actualiza los acumuladores de
  area para las estadisticas de tiempo promedio.
*/
{
  float time_since_last_event;

  /* Calcula el tiempo desde el ultimo evento, y actualiza el marcador
      del ultimo evento */

  time_since_last_event = tiempo_simulacion - tiempo_ultimo_evento;
  tiempo_ultimo_evento = tiempo_simulacion;

  /* Actualiza el area bajo la funcion de numero_en_cola */
  area_num_entra_cola += num_entra_cola * time_since_last_event;

  /*Actualiza el area bajo la funcion indicadora de servidor ocupado*/
  area_estado_servidor += estado_servidor * time_since_last_event;
}

float expon(float media) /* Funcion generadora de la exponencias */
{
  /* Retorna una variable aleatoria exponencial con media "media"*/
  return -media * log(lcgrand(21));
}
