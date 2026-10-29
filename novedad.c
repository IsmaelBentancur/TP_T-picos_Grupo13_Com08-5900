#include <string.h>
#include "novedad.h"
#include "cadena.h"

/*============================================================================*/
/*            FUNCIONES A IMPLEMENTAR POR EL ALUMNO (version _ALU)             */
/*============================================================================*/

/******************************************************************************
Descripcion:    recorre secuencialmente novedades_catalogo.dat y aplica cada
                operacion sobre el indice y titulos.dat: 'A' agrega si el ID
                no esta en el indice (validando como en la carga), 'B' pone
                estado 'B' en el archivo y elimina del indice, 'M' reemplaza
                titulo y genero y suma el stock (rechaza si quedaria
                negativo). Al final muestra el reporte de procesadas y
                rechazadas.
Parametros:     indice: indice de titulos.
                path_novedades: ruta de novedades_catalogo.dat.
                path_titulos: ruta de titulos.dat.
                fecha_proceso: fecha de proceso (para el contexto de
                validacion).
Retorno:        OK si se pudieron abrir los archivos, ERROR en caso contrario.
Observaciones:  Como el indice solo tiene activos, un 'A' sobre un ID dado de
                baja se agrega como registro nuevo al final del archivo.
******************************************************************************/
int novedades_procesar_ALU(t_indice *indice, const char *path_novedades,
                       const char *path_titulos, const t_fecha *fecha_proceso)
{
    FILE *pnov, *ptit;
    t_novedad nov;
    t_contexto ctx;
    const char *motivo;
    char tipo_error[TAM_TIPO_ERROR];
    unsigned procesadas = 0, rechazadas = 0;

    pnov = fopen(path_novedades, "rb");
    if(!pnov)
    {
        printf("No se encuentra %s.\n", path_novedades);
        return ERROR;
    }
    ptit = fopen(path_titulos, "r+b");
    if(!ptit)
    {
        printf("No se pudo abrir %s.\n", path_titulos);
        fclose(pnov);
        return ERROR;
    }

    ctx.indice = indice;
    ctx.arch_bin = ptit;
    ctx.fecha_proceso = fecha_proceso;
    ctx.id_modificando = 0;

    printf("Procesando %s...\n\n", path_novedades);
    while(fread(&nov, sizeof(t_novedad), 1, pnov) == 1)
    {
        nov.titulo.titulo[TAM_TITULO - 1] = '\0';
        nov.titulo.genero[TAM_GENERO - 1] = '\0';
        switch(nov.codigo_operacion)
        {
            case OP_ALTA:
                motivo = novedad_alta(indice, ptit, &nov.titulo, &ctx, tipo_error);
                break;
            case OP_BAJA:
                motivo = novedad_baja(indice, ptit, &nov.titulo);
                break;
            case OP_MODIFICACION:
                motivo = novedad_modificacion(indice, ptit, &nov.titulo);
                break;
            default:
                motivo = "codigo de operacion invalido";
                break;
        }
        if(motivo)
        {
            printf("  RECHAZADA  op=%c id=%-6d motivo: %s\n", nov.codigo_operacion, nov.titulo.id, motivo);
            rechazadas++;
        }
        else
            procesadas++;
    }
    printf("\nNovedades procesadas: %u   Rechazadas: %u\n", procesadas, rechazadas);

    fclose(ptit);
    fclose(pnov);
    return OK;
}

const char *novedad_alta(t_indice *indice, FILE *ptit, t_titulo *t, const t_contexto *ctx, char *tipo_error)
{
    t_reg_indice elem;

    elem.id = t->id;
    if(indice_buscar_ALU(indice, &elem, sizeof(t_reg_indice), compara_reg_indice_ALU) != NO_EXISTE)
        return "el ID ya existe";
    if(valida_titulo_ALU(t, ctx, tipo_error) != OK)
        return tipo_error;

    t->estado = ESTADO_ACTIVO;
    fseek(ptit, 0L, SEEK_END);
    elem.nro_reg = (unsigned)(ftell(ptit) / sizeof(t_titulo));
    if(titulo_escribir(ptit, elem.nro_reg, t) != OK ||
       indice_insertar_ALU(indice, &elem, sizeof(t_reg_indice), compara_reg_indice_ALU) != OK)
        return "no se pudo leer o grabar el titulo";
    printf("  ALTA       id=%-6d %s\n", t->id, t->titulo);
    return NULL;
}

const char *novedad_baja(t_indice *indice, FILE *ptit, const t_titulo *t)
{
    t_reg_indice elem;
    t_titulo actual;

    elem.id = t->id;
    if(indice_buscar_ALU(indice, &elem, sizeof(t_reg_indice), compara_reg_indice_ALU) == NO_EXISTE)
        return "el ID no existe o ya esta dado de baja";
    if(titulo_leer(ptit, elem.nro_reg, &actual) != OK)
        return "no se pudo leer o grabar el titulo";
    actual.estado = ESTADO_BAJA;
    if(titulo_escribir(ptit, elem.nro_reg, &actual) != OK)
        return "no se pudo leer o grabar el titulo";
    indice_eliminar_ALU(indice, &elem, sizeof(t_reg_indice), compara_reg_indice_ALU);
    printf("  BAJA       id=%-6d %s\n", actual.id, actual.titulo);
    return NULL;
}

const char *novedad_modificacion(t_indice *indice, FILE *ptit, const t_titulo *t)
{
    t_reg_indice elem;
    t_titulo actual;
    char nuevo_titulo[TAM_TITULO];

    elem.id = t->id;
    if(indice_buscar_ALU(indice, &elem, sizeof(t_reg_indice), compara_reg_indice_ALU) == NO_EXISTE)
        return "el ID no existe o esta dado de baja";
    if(titulo_leer(ptit, elem.nro_reg, &actual) != OK)
        return "no se pudo leer o grabar el titulo";

    copiar_cadena(nuevo_titulo, t->titulo, TAM_TITULO);
    arreglar_titulo(nuevo_titulo);
    if(cadena_vacia(nuevo_titulo))
        return "TITULO";
    if(genero_indice(t->genero) == NO_EXISTE)
        return "GENERO";
    if(actual.stock + t->stock < 0)
        return "STOCK (quedaria negativo)";

    copiar_cadena(actual.titulo, nuevo_titulo, TAM_TITULO);
    copiar_cadena(actual.genero, t->genero, TAM_GENERO);
    actual.stock += t->stock;
    if(titulo_escribir(ptit, elem.nro_reg, &actual) != OK)
        return "no se pudo leer o grabar el titulo";
    printf("  MODIFICADA id=%-6d %s (stock %d)\n", actual.id, actual.titulo, actual.stock);
    return NULL;
}
