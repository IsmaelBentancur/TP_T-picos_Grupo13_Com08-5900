#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>
#include "cadena.h"

/*============================================================================*/
/*            FUNCIONES A IMPLEMENTAR POR EL ALUMNO (version _ALU)             */
/*============================================================================*/

/******************************************************************************
Descripcion:    normaliza un campo Apellidos y Nombres segun el Anexo A.2 (con
                regla de coma).
Parametros:     s: cadena a normalizar (se modifica in place).
Retorno:        No posee.
Observaciones:  -
******************************************************************************/
void arreglar_apyn_ALU(char *s)
{
    char *copia, *lec, *esc = s;
    int palabras = 0, hay_coma;

    copia = (char *)malloc(strlen(s) + 1);
    if(!copia)
        return;
    strcpy(copia, s);
    hay_coma = strchr(copia, ',') != NULL;

    lec = copia;
    while(*lec)
    {
        while(ES_BLANCO(*lec))
            lec++;
        if(*lec == ',')
        {
            *esc++ = ',';
            lec++;
        }
        else if(*lec)
        {
            if(palabras == 1 && !hay_coma)
            {
                *esc++ = ',';
                *esc++ = ' ';
            }
            else if(esc > s)
                *esc++ = ' ';

            *esc++ = A_MAYUSCULA(*lec);
            lec++;
            while(*lec && !ES_BLANCO(*lec) && *lec != ',')
            {
                *esc = *(esc - 1) == '\'' ? A_MAYUSCULA(*lec) : A_MINUSCULA(*lec);
                esc++;
                lec++;
            }
            palabras++;
        }
    }
    *esc = '\0';
    free(copia);
}

/******************************************************************************
Descripcion:    separa una linea CSV in place por SEPARADOR_CSV, reemplazando
                cada separador por '\0' y guardando en campos[] el puntero al
                inicio de cada campo.
Parametros:     linea: linea a separar (se modifica).
                campos: vector donde se guardan los punteros a cada campo.
                max: cantidad maxima de campos a separar.
Retorno:        Cantidad de campos encontrados (los vacios cuentan). Nunca mas
                de max.
Observaciones:  Si la linea tiene mas separadores que max, el ultimo campo
                conserva el resto de la linea.
******************************************************************************/
int separar_campos_ALU(char *linea, char *campos[], int max)
{
    int cant = 0;

    if(max <= 0)
        return 0;
    *campos = linea;
    cant++;
    while(*linea && cant < max)
    {
        if(*linea == SEPARADOR_CSV)
        {
            *linea = '\0';
            *(campos + cant) = linea + 1;
            cant++;
        }
        linea++;
    }
    return cant;
}
