#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "indice.h"

/*============================================================================*/
/*            FUNCIONES A IMPLEMENTAR POR EL ALUMNO (version _ALU)             */
/*============================================================================*/

/******************************************************************************
Descripcion:    toma memoria para nmemb elementos e inicializa la estructura
                vacia.
Parametros:     indice: TDA indice.
                nmemb: cantidad inicial de elementos (CANTIDAD_ELEMENTOS).
                tamanyo: espacio en bytes ocupado por cada elemento.
Retorno:        OK si la operacion fue exitosa y ERROR en caso contrario.
Observaciones:  -
******************************************************************************/
int indice_crear_ALU(t_indice *indice, size_t nmemb, size_t tamanyo)
{
    indice->vindice = malloc(nmemb * tamanyo);
    if(!indice->vindice)
    {
        indice->cantidad_elementos_actual = 0;
        indice->cantidad_elementos_maxima = 0;
        return ERROR;
    }
    indice->cantidad_elementos_actual = 0;
    indice->cantidad_elementos_maxima = (unsigned)nmemb;
    return OK;
}

/******************************************************************************
Descripcion:    redimensiona el arreglo interno del indice con realloc. La
                nueva capacidad es la actual multiplicada por INCREMENTO (30%
                mas); la calcula la propia funcion.
Parametros:     indice: TDA indice.
                tamanyo: espacio en bytes ocupado por cada elemento.
Retorno:        OK si la operacion fue exitosa y ERROR en caso contrario.
Observaciones:  Si el producto no aumenta la capacidad (arreglos muy chicos)
                se garantiza al menos un elemento mas.
******************************************************************************/
int indice_redimensionar_ALU(t_indice *indice, size_t tamanyo)
{
    unsigned nueva = (unsigned)(indice->cantidad_elementos_maxima * INCREMENTO);
    void *aux;

    if(nueva <= indice->cantidad_elementos_maxima)
        nueva = indice->cantidad_elementos_maxima + 1;
    aux = realloc(indice->vindice, nueva * tamanyo);
    if(!aux)
        return ERROR;
    indice->vindice = aux;
    indice->cantidad_elementos_maxima = nueva;
    return OK;
}

int indice_buscar_posicion(const t_indice *indice, const void *registro, size_t tamanyo,
                           int (*cmp)(const void *, const void *), unsigned *pos)
{
    const char *base = (const char *)indice->vindice;
    int li = 0, ls = (int)indice->cantidad_elementos_actual - 1, medio, comp;

    while(li <= ls)
    {
        medio = (li + ls) / 2;
        comp = cmp(registro, base + medio * tamanyo);
        if(comp == 0)
        {
            *pos = (unsigned)medio;
            return 1;
        }
        if(comp < 0)
            ls = medio - 1;
        else
            li = medio + 1;
    }
    *pos = (unsigned)li;
    return 0;
}

/******************************************************************************
Descripcion:    inserta un elemento manteniendo el orden segun la clave. Si el
                arreglo esta lleno invoca indice_redimensionar.
Parametros:     indice: TDA indice.
                registro: elemento a insertar.
                tamanyo: espacio en bytes ocupado por el elemento.
                cmp: funcion de comparacion provista.
Retorno:        OK si la operacion fue exitosa y ERROR en caso contrario
                (clave duplicada o falta de memoria).
Observaciones:  -
******************************************************************************/
int indice_insertar_ALU(t_indice *indice, const void *registro, size_t tamanyo,
                    int (*cmp)(const void *, const void *))
{
    char *lugar;
    unsigned pos;

    if(indice_buscar_posicion(indice, registro, tamanyo, cmp, &pos))
        return ERROR;
    if(indice_lleno_ALU(indice) == OK && indice_redimensionar_ALU(indice, tamanyo) != OK)
        return ERROR;

    lugar = (char *)indice->vindice + pos * tamanyo;
    memmove(lugar + tamanyo, lugar, (indice->cantidad_elementos_actual - pos) * tamanyo);
    memcpy(lugar, registro, tamanyo);
    indice->cantidad_elementos_actual++;
    return OK;
}

/******************************************************************************
Descripcion:    elimina el elemento cuya clave coincide con la de registro,
                desplazando los siguientes.
Parametros:     indice: TDA indice.
                registro: elemento a eliminar (solo importa su clave).
                tamanyo: espacio en bytes ocupado por el elemento.
                cmp: funcion de comparacion provista.
Retorno:        OK si la operacion fue exitosa y ERROR si la clave no existe.
Observaciones:  -
******************************************************************************/
int indice_eliminar_ALU(t_indice *indice, const void *registro, size_t tamanyo,
                    int (*cmp)(const void *, const void *))
{
    char *lugar;
    unsigned pos;

    if(!indice_buscar_posicion(indice, registro, tamanyo, cmp, &pos))
        return ERROR;

    lugar = (char *)indice->vindice + pos * tamanyo;
    memmove(lugar, lugar + tamanyo, (indice->cantidad_elementos_actual - pos - 1) * tamanyo);
    indice->cantidad_elementos_actual--;
    return OK;
}

/******************************************************************************
Descripcion:    busca la clave de registro en el indice. Si existe copia el
                elemento completo (nro_reg incluido) en registro.
Parametros:     indice: TDA indice.
                registro: elemento a buscar (entrada/salida).
                tamanyo: espacio en bytes ocupado por el elemento.
                cmp: funcion de comparacion provista.
Retorno:        NO_EXISTE si la clave no existe; en caso contrario la posicion
                que ocupa dentro del arreglo.
Observaciones:  La comparacion entre claves se realiza exclusivamente mediante
                cmp.
******************************************************************************/
int indice_buscar_ALU(const t_indice *indice, void *registro, size_t tamanyo,
                  int (*cmp)(const void *, const void *))
{
    unsigned pos;

    if(!indice_buscar_posicion(indice, registro, tamanyo, cmp, &pos))
        return NO_EXISTE;
    memcpy(registro, (const char *)indice->vindice + pos * tamanyo, tamanyo);
    return (int)pos;
}

/******************************************************************************
Descripcion:    determina si el indice contiene 0 (cero) elementos.
Parametros:     indice: TDA indice.
Retorno:        OK si esta vacio, ERROR si no lo esta.
Observaciones:  -
******************************************************************************/
int indice_vacio_ALU(const t_indice *indice)
{
    return indice->cantidad_elementos_actual == 0 ? OK : ERROR;
}

/******************************************************************************
Descripcion:    determina si el indice alcanzo su capacidad maxima actual.
Parametros:     indice: TDA indice.
Retorno:        OK si esta lleno, ERROR si no lo esta.
Observaciones:  -
******************************************************************************/
int indice_lleno_ALU(const t_indice *indice)
{
    return indice->cantidad_elementos_actual == indice->cantidad_elementos_maxima ? OK : ERROR;
}

/******************************************************************************
Descripcion:    deja el indice vacio sin liberar la memoria del arreglo.
Parametros:     indice: TDA indice.
Retorno:        No posee.
Observaciones:  -
******************************************************************************/
void indice_vaciar_ALU(t_indice *indice)
{
    indice->cantidad_elementos_actual = 0;
}

/******************************************************************************
Descripcion:    libera toda la memoria dinamica del indice.
Parametros:     indice: TDA indice.
Retorno:        No posee.
Observaciones:  Luego de invocarla el indice no puede usarse sin volver a
                llamar a indice_crear.
******************************************************************************/
void indice_destruir_ALU(t_indice *indice)
{
    free(indice->vindice);
    indice->vindice = NULL;
    indice->cantidad_elementos_actual = 0;
    indice->cantidad_elementos_maxima = 0;
}

/******************************************************************************
Descripcion:    construye el indice a partir de un archivo binario de
                registros. Recorre el archivo secuencialmente y por cada
                registro activo inserta un t_reg_indice (nro_reg = posicion en
                el archivo, id = clave obtenida con obtener_clave).
Parametros:     path: ruta al archivo binario.
                indice: TDA indice (ya creado; se vacia antes de cargar).
                tamanyo_reg: tamanyo en bytes de cada registro del archivo.
                obtener_clave: funcion que devuelve la clave (long) de un
                registro.
                es_activo: funcion que devuelve distinto de 0 si el registro
                tiene estado 'A'.
                cmp: funcion de comparacion provista.
Retorno:        OK si la operacion fue exitosa y ERROR en caso contrario.
Observaciones:  Los registros con estado 'B' se omiten. El archivo no necesita
                estar ordenado: el orden lo garantiza indice_insertar.
******************************************************************************/
int indice_cargar_ALU(const char *path, t_indice *indice, size_t tamanyo_reg,
                  long (*obtener_clave)(const void *),
                  int (*es_activo)(const void *),
                  int (*cmp)(const void *, const void *))
{
    FILE *pf;
    void *reg;
    t_reg_indice elem;
    int resultado = OK;

    pf = fopen(path, "rb");
    if(!pf)
        return ERROR;
    reg = malloc(tamanyo_reg);
    if(!reg)
    {
        fclose(pf);
        return ERROR;
    }

    indice_vaciar_ALU(indice);
    elem.nro_reg = 0;
    while(resultado == OK && fread(reg, tamanyo_reg, 1, pf) == 1)
    {
        if(es_activo(reg))
        {
            elem.id = obtener_clave(reg);
            if(indice_insertar_ALU(indice, &elem, sizeof(t_reg_indice), cmp) != OK)
                resultado = ERROR;
        }
        elem.nro_reg++;
    }

    free(reg);
    fclose(pf);
    return resultado;
}
