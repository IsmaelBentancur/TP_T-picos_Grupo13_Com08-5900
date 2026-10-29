#include <stdlib.h>
#include <string.h>
#include "alquiler.h"
#include "cadena.h"
#include "util.h"

/*============================================================================*/
/*            FUNCIONES A IMPLEMENTAR POR EL ALUMNO (version _ALU)             */
/*============================================================================*/

/******************************************************************************
Descripcion:    imprime por pantalla la matriz de consumo con totales por fila
                (plan) y por columna (genero).
Parametros:     m: matriz de consumo.
Retorno:        No posee.
Observaciones:  -
******************************************************************************/
void tablero_mostrar_ALU(t_matriz_consumo m)
{
    int total_columna[CANT_GENEROS] = {0};
    int total_fila, total_general = 0;
    int i, j;

    printf("%-10s", "PLAN");
    for(j = 0; j < CANT_GENEROS; j++)
        printf(" %16s", GENEROS[j]);
    printf(" %8s\n", "TOTAL");

    for(i = 0; i < CANT_PLANES; i++)
    {
        total_fila = 0;
        printf("%-10s", PLANES[i]);
        for(j = 0; j < CANT_GENEROS; j++)
        {
            printf(" %16d", m[i][j]);
            total_fila += m[i][j];
            total_columna[j] += m[i][j];
        }
        printf(" %8d\n", total_fila);
        total_general += total_fila;
    }

    printf("%-10s", "TOTAL");
    for(j = 0; j < CANT_GENEROS; j++)
        printf(" %16d", total_columna[j]);
    printf(" %8d\n", total_general);
}

/******************************************************************************
Descripcion:    ejecuta el flujo de alquiler de un titulo: 1) habilitacion del
                socio (debe estar en el indice), 2) inventario (el titulo debe
                estar en el indice y con stock > 0), 3) descuenta una unidad
                de stock en el archivo, 4) incrementa la celda [plan][genero]
                de la matriz, 5) emite el comprobante.
Parametros:     ind_socios: indice de socios.
                ind_titulos: indice de titulos.
                path_socios: ruta de socios.dat.
                path_titulos: ruta de titulos.dat.
                matriz: matriz de consumo a alimentar.
                fecha_proceso: fecha de proceso para el comprobante.
Retorno:        OK si el alquiler se concreto, ERROR si alguna etapa fallo.
Observaciones:  Si una etapa falla se informa el motivo y no se ejecutan las
                siguientes. No se exige cuota al dia.
******************************************************************************/
int alquilar_ALU(t_indice *ind_socios, t_indice *ind_titulos, const char *path_socios,
             const char *path_titulos, t_matriz_consumo matriz_consumo, const t_fecha *fecha_proceso)
{
    t_reg_indice elem_soc, elem_tit;
    t_socio s;
    t_titulo t;
    FILE *pf;
    long id;
    int fila, columna;

    printf("DNI del socio: ");
    if(!leer_numero(&elem_soc.id))
    {
        printf("Alquiler rechazado: DNI invalido.\n");
        return ERROR;
    }
    if(indice_buscar_ALU(ind_socios, &elem_soc, sizeof(t_reg_indice), compara_reg_indice_ALU) == NO_EXISTE)
    {
        printf("Alquiler rechazado: el socio no existe o esta dado de baja.\n");
        return ERROR;
    }
    pf = fopen(path_socios, "rb");
    if(!pf)
    {
        printf("No se pudo abrir %s.\n", path_socios);
        return ERROR;
    }
    if(socio_leer_ALU(pf, elem_soc.nro_reg, &s) != OK)
    {
        printf("Alquiler rechazado: no se pudo leer el socio.\n");
        fclose(pf);
        return ERROR;
    }
    fclose(pf);

    printf("ID de la pelicula: ");
    if(!leer_numero(&id))
    {
        printf("Alquiler rechazado: ID invalido.\n");
        return ERROR;
    }
    elem_tit.id = id;
    if(indice_buscar_ALU(ind_titulos, &elem_tit, sizeof(t_reg_indice), compara_reg_indice_ALU) == NO_EXISTE)
    {
        printf("Alquiler rechazado: el titulo no existe o esta dado de baja.\n");
        return ERROR;
    }
    pf = fopen(path_titulos, "r+b");
    if(!pf)
    {
        printf("No se pudo abrir %s.\n", path_titulos);
        return ERROR;
    }
    if(titulo_leer(pf, elem_tit.nro_reg, &t) != OK)
    {
        printf("Alquiler rechazado: no se pudo leer el titulo.\n");
        fclose(pf);
        return ERROR;
    }
    if(t.stock <= 0)
    {
        printf("Alquiler rechazado: sin stock de \"%s\".\n", t.titulo);
        fclose(pf);
        return ERROR;
    }

    t.stock--;
    if(titulo_escribir(pf, elem_tit.nro_reg, &t) != OK)
    {
        printf("Alquiler rechazado: no se pudo grabar el titulo.\n");
        fclose(pf);
        return ERROR;
    }
    fclose(pf);

    fila = plan_indice(s.plan);
    columna = genero_indice(t.genero);
    if(fila >= 0 && fila < CANT_PLANES && columna >= 0 && columna < CANT_GENEROS)
        matriz_consumo[fila][columna]++;

    printf("\n========== COMPROBANTE DE ALQUILER ==========\n");
    printf(" Fecha    : %02d/%02d/%04d\n", fecha_proceso->dia, fecha_proceso->mes, fecha_proceso->anio);
    printf(" Socio    : %ld - %s (plan %s)\n", s.dni, s.apellidos_nombres, s.plan);
    printf(" Titulo   : %d - %s (%s)\n", t.id, t.titulo, t.genero);
    printf(" Stock restante: %d\n", t.stock);
    printf("=============================================\n");
    return OK;
}
