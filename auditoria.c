#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "auditoria.h"
#include "lectura.h"
#include "cadena.h"
#include "util.h"
#include "indice.h"

/*============================================================================*/
/*            FUNCIONES A IMPLEMENTAR POR EL ALUMNO (version _ALU)             */
/*============================================================================*/

/******************************************************************************
Descripcion:    agrega al final del archivo de auditoria una linea con el
                formato DNI;TIPO_VALIDACION;FECHA_PROCESO.
Parametros:     path: ruta del archivo de texto de errores.
                dni_texto: DNI tal como vino en la entrada.
                tipo_error: nombre de la validacion que fallo.
                fecha_proceso: fecha de proceso.
Retorno:        OK si se pudo grabar, ERROR en caso contrario.
Observaciones:  El archivo se abre en modo append; si no existe se crea.
******************************************************************************/
int auditoria_registrar_ALU(const char *path, const char *dni_texto,
                        const char *tipo_error, const t_fecha *fecha_proceso)
{
    FILE *pf;

    pf = fopen(path, "at");
    if(!pf)
        return ERROR;
    fprintf(pf, "%s;%s;%02d/%02d/%04d\n", dni_texto, tipo_error,
            fecha_proceso->dia, fecha_proceso->mes, fecha_proceso->anio);
    fclose(pf);
    return OK;
}

/******************************************************************************
Descripcion:    lee el archivo de auditoria y muestra, por cada par (DNI, tipo
                de validacion), la cantidad de incidencias registradas y el
                total general.
Parametros:     path: ruta del archivo de texto de errores.
Retorno:        OK si se pudo procesar (aunque el archivo no exista), ERROR si
                fallo la memoria.
Observaciones:  Acumula en un vector dinamico que crece con realloc.
******************************************************************************/
int auditoria_mostrar_ALU(const char *path)
{
    FILE *pf;
    char linea[TAM_LINEA];
    char *campos[CAMPOS_AUDITORIA];
    t_incidencia *vec, *aux;
    int cant = 0, max = INCIDENCIAS_INICIAL, total = 0, pos, i;

    pf = fopen(path, "rt");
    if(!pf)
    {
        printf("No hay errores registrados (no existe %s).\n", path);
        return OK;
    }
    vec = (t_incidencia *)malloc(max * sizeof(t_incidencia));
    if(!vec)
    {
        fclose(pf);
        return ERROR;
    }

    while(fgets(linea, TAM_LINEA, pf))
    {
        quitar_salto(linea);
        if(cadena_vacia(linea) || separar_campos_ALU(linea, campos, CAMPOS_AUDITORIA) < CAMPOS_AUDITORIA - 1)
            continue;

        pos = 0;
        while(pos < cant && (strcmp(vec[pos].dni, campos[AUD_DNI]) != 0 ||
                             strcmp(vec[pos].tipo, campos[AUD_TIPO]) != 0))
            pos++;
        if(pos == cant)
        {
            if(cant == max)
            {
                max += INCIDENCIAS_INICIAL;
                aux = (t_incidencia *)realloc(vec, max * sizeof(t_incidencia));
                if(!aux)
                {
                    free(vec);
                    fclose(pf);
                    return ERROR;
                }
                vec = aux;
            }
            copiar_cadena(vec[pos].dni, campos[AUD_DNI], TAM_DNI_TEXTO);
            copiar_cadena(vec[pos].tipo, campos[AUD_TIPO], TAM_TIPO_ERROR);
            vec[pos].cantidad = 0;
            cant++;
        }
        vec[pos].cantidad++;
        total++;
    }
    fclose(pf);

    printf("%-15s %-22s %s\n", "DNI", "TIPO DE VALIDACION", "CANTIDAD");
    printf("%-15s %-22s %s\n", "---------------", "----------------------", "--------");
    for(i = 0; i < cant; i++)
        printf("%-15s %-22s %8d\n", vec[i].dni, vec[i].tipo, vec[i].cantidad);
    printf("\nTotal de incidencias: %d\n", total);

    free(vec);
    return OK;
}
