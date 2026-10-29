#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "fecha.h"
#include "util.h"
#include "indice.h"      /* OK / ERROR */

/*============================================================================*/
/*            FUNCIONES A IMPLEMENTAR POR EL ALUMNO (version _ALU)             */
/*============================================================================*/

/******************************************************************************
Descripcion:    solicita la fecha de proceso por teclado hasta obtener una
                valida. Si el usuario solo presiona Enter toma la fecha actual
                del sistema.
Parametros:     fecha_proceso: fecha destino.
Retorno:        No posee.
Observaciones:  -
******************************************************************************/
void pedir_fecha_proceso_ALU(t_fecha *fecha_proceso)
{
    int resultado;

    do
    {
        printf("Ingrese fecha de proceso (dd/mm/aaaa) [Enter = fecha actual]: ");
        resultado = lee_fecha(fecha_proceso);
        if(resultado == ERROR)
            printf("FECHA FORMALMENTE INCORRECTA\n");
    } while(resultado == ERROR);

    if(resultado == FECHA_VACIA)
        fecha_actual(fecha_proceso);
}
