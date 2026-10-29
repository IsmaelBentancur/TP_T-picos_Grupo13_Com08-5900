#ifndef NOVEDAD_H_INCLUDED
#define NOVEDAD_H_INCLUDED

#include "titulo.h"

#define OP_ALTA         'A'
#define OP_BAJA         'B'
#define OP_MODIFICACION 'M'

typedef struct
{
    char     codigo_operacion;   /* 'A' Alta, 'B' Baja, 'M' Modificacion */
    t_titulo titulo;
} t_novedad;

int novedades_procesar(t_indice *indice, const char *path_novedades,
                       const char *path_titulos, const t_fecha *fecha_proceso);

/*============================================================================*/
/*            FUNCIONES A IMPLEMENTAR POR EL ALUMNO (version _ALU)             */
/*============================================================================*/

int novedades_procesar_ALU(t_indice *indice, const char *path_novedades,
                       const char *path_titulos, const t_fecha *fecha_proceso);

const char *novedad_alta(t_indice *indice, FILE *ptit, t_titulo *t, const t_contexto *ctx, char *tipo_error);
const char *novedad_baja(t_indice *indice, FILE *ptit, const t_titulo *t);
const char *novedad_modificacion(t_indice *indice, FILE *ptit, const t_titulo *t);

#endif /* NOVEDAD_H_INCLUDED */
