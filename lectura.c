#include <string.h>
#include "lectura.h"
#include "util.h"
#include "cadena.h"
#include "socio.h"
#include "titulo.h"
#include "config.h"

/*============================================================================*/
/*            FUNCIONES A IMPLEMENTAR POR EL ALUMNO (version _ALU)             */
/*============================================================================*/

/******************************************************************************
Descripcion:    funcion generica de lectura de archivos de texto. Por cada
                linea no vacia: parsear -> validar -> escribir al final del
                archivo binario e insertar en el indice. Ante el primer error
                (de formato o de validacion) se rechaza la linea invocando
                rechazar y se continua con la siguiente.
Parametros:     path_txt: ruta del archivo de texto (CSV).
                arch_bin: archivo binario destino abierto en "w+b" o "r+b".
                indice: indice de la entidad (se usa para validar duplicados y
                se va completando).
                reg: buffer de un registro de la entidad.
                tamanyo: tamanyo en bytes del registro.
                fecha_proceso: fecha de proceso.
                parsear: convierte una linea en registro.
                validar: valida el registro y deja el tipo de error.
                rechazar: accion ante un rechazo (auditar, ignorar, etc.).
                obtener_clave: devuelve la clave del registro para el indice.
                aceptados, rechazados: salida; contadores de registros.
Retorno:        OK si se pudo abrir el archivo de texto, ERROR en caso
                contrario.
Observaciones:  Se reutiliza para Socios.csv y Titulos.csv; lo especifico de
                cada entidad lo resuelven los punteros a funcion.
******************************************************************************/
int leer_archivo_texto_ALU(const char *path_txt, FILE *arch_bin, t_indice *indice,
                       void *reg, size_t tamanyo, const t_fecha *fecha_proceso,
                       int (*parsear)(const char *, void *),
                       int (*validar)(void *, const t_contexto *, char *),
                       void (*rechazar)(const void *, const char *, const char *, const t_fecha *),
                       long (*obtener_clave)(const void *),
                       unsigned *aceptados, unsigned *rechazados)
{
    FILE *pf;
    char linea[TAM_LINEA];
    char tipo_error[TAM_TIPO_ERROR];
    t_contexto ctx;
    t_reg_indice elem;

    *aceptados = 0;
    *rechazados = 0;
    pf = fopen(path_txt, "rt");
    if(!pf)
        return ERROR;

    ctx.indice = indice;
    ctx.arch_bin = arch_bin;
    ctx.fecha_proceso = fecha_proceso;
    ctx.id_modificando = 0;

    while(fgets(linea, TAM_LINEA, pf))
    {
        quitar_salto(linea);
        if(cadena_vacia(linea))
            continue;

        tipo_error[0] = '\0';
        if(parsear(linea, reg) != OK)
        {
            strcpy(tipo_error, TIPO_ERROR_FORMATO);
            rechazar(reg, linea, tipo_error, fecha_proceso);
            (*rechazados)++;
        }
        else if(validar(reg, &ctx, tipo_error) != OK)
        {
            rechazar(reg, linea, tipo_error, fecha_proceso);
            (*rechazados)++;
        }
        else
        {
            fseek(arch_bin, 0L, SEEK_END);
            elem.nro_reg = (unsigned)(ftell(arch_bin) / tamanyo);
            elem.id = obtener_clave(reg);
            if(fwrite(reg, tamanyo, 1, arch_bin) == 1 &&
               indice_insertar_ALU(indice, &elem, sizeof(t_reg_indice), compara_reg_indice_ALU) == OK)
                (*aceptados)++;
            else
                (*rechazados)++;
        }
    }
    fflush(arch_bin);
    fclose(pf);
    return OK;
}

/******************************************************************************
Descripcion:    compara dos t_reg_indice por su clave id.
Parametros:     a, b: punteros a t_reg_indice.
Retorno:        Negativo si a < b, 0 si son iguales, positivo si a > b.
Observaciones:  Se usa como cmp en todas las operaciones del TDA indice.
******************************************************************************/
int compara_reg_indice_ALU(const void *a, const void *b)
{
    const t_reg_indice *ra = (const t_reg_indice *)a;
    const t_reg_indice *rb = (const t_reg_indice *)b;

    if(ra->id < rb->id)
        return -1;
    if(ra->id > rb->id)
        return 1;
    return 0;
}

/******************************************************************************
Descripcion:    construye los indices de socios y de titulos a partir de los
                archivos binarios existentes (solo registros activos).
Parametros:     ind_socios: indice de socios.
                ind_titulos: indice de titulos.
Retorno:        1 si ambos binarios existen y se cargaron, 0 en caso
                contrario.
Observaciones:  -
******************************************************************************/
int cargar_indices_ALU(t_indice *ind_socios, t_indice *ind_titulos)
{
    if(!archivo_existe(ARCH_SOCIOS_BIN) || !archivo_existe(ARCH_TITULOS_BIN))
        return 0;
    if(indice_cargar_ALU(ARCH_SOCIOS_BIN, ind_socios, sizeof(t_socio), clave_socio, activo_socio,
                         compara_reg_indice_ALU) != OK)
        return 0;
    if(indice_cargar_ALU(ARCH_TITULOS_BIN, ind_titulos, sizeof(t_titulo), clave_titulo, activo_titulo,
                         compara_reg_indice_ALU) != OK)
        return 0;
    return 1;
}

/******************************************************************************
Descripcion:    opcion 1 del menu: lee Socios.csv y Titulos.csv con la funcion
                generica de lectura, genera socios.dat y titulos.dat
                (sobrescribiendolos) y reconstruye los indices desde los
                binarios generados.
Parametros:     ind_socios: indice de socios.
                ind_titulos: indice de titulos.
                fecha_proceso: fecha de proceso.
Retorno:        1 si los indices quedaron cargados, 0 en caso contrario.
Observaciones:  Los rechazos de socios se auditan en errores_socios.txt; los
                de titulos solo se cuentan.
******************************************************************************/
int generar_binarios_ALU(t_indice *ind_socios, t_indice *ind_titulos, const t_fecha *fecha_proceso)
{
    FILE *pf;
    t_socio socio;
    t_titulo titulo;
    unsigned aceptados, rechazados;

    pf = fopen(ARCH_SOCIOS_BIN, "w+b");
    if(!pf)
    {
        printf("Error de creacion de %s.\n", ARCH_SOCIOS_BIN);
        return 0;
    }
    indice_vaciar_ALU(ind_socios);
    if(leer_archivo_texto_ALU(ARCH_SOCIOS_CSV, pf, ind_socios, &socio, sizeof(t_socio), fecha_proceso,
                              texto_a_socio_ALU, valida_socio_ALU, rechaza_socio, clave_socio,
                              &aceptados, &rechazados) != OK)
    {
        printf("No se encuentra %s.\n", ARCH_SOCIOS_CSV);
        fclose(pf);
        return 0;
    }
    fclose(pf);
    printf("Socios:  %u aceptados, %u rechazados (ver %s)\n", aceptados, rechazados, ARCH_ERRORES);

    pf = fopen(ARCH_TITULOS_BIN, "w+b");
    if(!pf)
    {
        printf("Error de creacion de %s.\n", ARCH_TITULOS_BIN);
        return 0;
    }
    indice_vaciar_ALU(ind_titulos);
    if(leer_archivo_texto_ALU(ARCH_TITULOS_CSV, pf, ind_titulos, &titulo, sizeof(t_titulo), fecha_proceso,
                              texto_a_titulo_ALU, valida_titulo_ALU, rechaza_titulo, clave_titulo,
                              &aceptados, &rechazados) != OK)
    {
        printf("No se encuentra %s.\n", ARCH_TITULOS_CSV);
        fclose(pf);
        return 0;
    }
    fclose(pf);
    printf("Titulos: %u aceptados, %u rechazados\n", aceptados, rechazados);

    return cargar_indices_ALU(ind_socios, ind_titulos);
}
