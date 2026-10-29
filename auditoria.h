#ifndef AUDITORIA_H_INCLUDED
#define AUDITORIA_H_INCLUDED

#include "fecha.h"

int auditoria_registrar(const char *path, const char *dni_texto,
                        const char *tipo_error, const t_fecha *fecha_proceso);
int auditoria_mostrar(const char *path);

/*============================================================================*/
/*            FUNCIONES A IMPLEMENTAR POR EL ALUMNO (version _ALU)             */
/*============================================================================*/

int auditoria_registrar_ALU(const char *path, const char *dni_texto,
                        const char *tipo_error, const t_fecha *fecha_proceso);
int auditoria_mostrar_ALU(const char *path);

#include "lectura.h"

#define CAMPOS_AUDITORIA    3
#define AUD_DNI             0
#define AUD_TIPO            1
#define TAM_DNI_TEXTO      21
#define INCIDENCIAS_INICIAL 10

typedef struct
{
    char dni[TAM_DNI_TEXTO];
    char tipo[TAM_TIPO_ERROR];
    int  cantidad;
} t_incidencia;

#endif /* AUDITORIA_H_INCLUDED */
