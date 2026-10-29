#include <stdlib.h>
#include <string.h>
#include "titulo.h"
#include "cadena.h"
#include "util.h"

/*============================================================================*/
/*            FUNCIONES A IMPLEMENTAR POR EL ALUMNO (version _ALU)             */
/*============================================================================*/

/******************************************************************************
Descripcion:    convierte una linea CSV de 4 campos en t_titulo. Los campos
                numericos invalidos quedan en valores que fallan la validacion
                (id = 0, stock = -1). El estado se fija en 'A'.
Parametros:     linea: linea CSV.
                reg: puntero a t_titulo destino.
Retorno:        OK si la cantidad de campos es correcta, ERROR en caso
                contrario.
Observaciones:  Se pasa por puntero como parsear a leer_archivo_texto.
******************************************************************************/
int texto_a_titulo_ALU(const char *linea, void *reg)
{
    t_titulo *t = (t_titulo *)reg;
    char aux[TAM_LINEA];
    char *campos[CAMPOS_TITULO];

    memset(t, 0, sizeof(t_titulo));
    copiar_cadena(aux, linea, TAM_LINEA);
    if(separar_campos_ALU(aux, campos, CAMPOS_TITULO) != CAMPOS_TITULO)
        return ERROR;

    if(es_numero(campos[TIT_ID]))
        sscanf(campos[TIT_ID], "%d", &t->id);
    copiar_cadena(t->titulo, campos[TIT_TITULO], TAM_TITULO);
    copiar_cadena(t->genero, campos[TIT_GENERO], TAM_GENERO);
    t->stock = STOCK_INVALIDO;
    if(es_numero(campos[TIT_STOCK]))
        sscanf(campos[TIT_STOCK], "%d", &t->stock);
    t->estado = ESTADO_ACTIVO;
    return OK;
}

/******************************************************************************
Descripcion:    aplica las validaciones del punto 6.2 en orden: ID > 0 y no
                existente en el indice, titulo normalizado no vacio, genero
                dentro de GENEROS[] y stock >= 0. Se detiene en el primer
                error.
Parametros:     reg: puntero a t_titulo (se modifica al normalizar el titulo).
                ctx: contexto con indice e id_modificando.
                tipo_error: salida; nombre de la validacion que fallo.
Retorno:        OK si el titulo es valido, ERROR en caso contrario.
Observaciones:  Se pasa por puntero como validar a leer_archivo_texto. Se reutiliza en las altas del
                lote de novedades.
******************************************************************************/
int valida_titulo_ALU(void *reg, const t_contexto *ctx, char *tipo_error)
{
    t_titulo *t = (t_titulo *)reg;
    t_reg_indice elem;

    elem.id = t->id;
    if(t->id <= 0 ||
       (t->id != ctx->id_modificando &&
        indice_buscar_ALU(ctx->indice, &elem, sizeof(t_reg_indice), compara_reg_indice_ALU) != NO_EXISTE))
        return titulo_error(tipo_error, "ID");

    arreglar_titulo(t->titulo);
    if(cadena_vacia(t->titulo))
        return titulo_error(tipo_error, "TITULO");

    if(genero_indice(t->genero) == NO_EXISTE)
        return titulo_error(tipo_error, "GENERO");

    if(t->stock < 0)
        return titulo_error(tipo_error, "STOCK");

    return OK;
}

/******************************************************************************
Descripcion:    recorre el indice (ordenado por ID y solo con activos) y lista
                los titulos leyendo cada registro del archivo.
Parametros:     indice: indice de titulos.
                path: ruta de titulos.dat.
Retorno:        OK si se pudo abrir el archivo, ERROR en caso contrario.
Observaciones:  -
******************************************************************************/
int titulos_listar_ALU(const t_indice *indice, const char *path)
{
    const t_reg_indice *elem = (const t_reg_indice *)indice->vindice;
    const t_reg_indice *fin = elem + indice->cantidad_elementos_actual;
    t_titulo t;
    FILE *pf;

    pf = fopen(path, "rb");
    if(!pf)
    {
        printf("No se pudo abrir %s.\n", path);
        return ERROR;
    }
    titulos_encabezado();
    for(; elem < fin; elem++)
        if(titulo_leer(pf, elem->nro_reg, &t) == OK)
            titulo_mostrar(&t);
    printf("\nTotal de titulos activos: %u\n", indice->cantidad_elementos_actual);
    fclose(pf);
    return OK;
}

int titulo_error(char *tipo_error, const char *nombre)
{
    copiar_cadena(tipo_error, nombre, TAM_TIPO_ERROR);
    return ERROR;
}
