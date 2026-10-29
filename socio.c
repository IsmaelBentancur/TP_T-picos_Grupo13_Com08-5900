#include <stdlib.h>
#include <string.h>
#include "socio.h"
#include "cadena.h"
#include "util.h"
#include "auditoria.h"
#include "config.h"
#include "menu.h"

/*============================================================================*/
/*            FUNCIONES A IMPLEMENTAR POR EL ALUMNO (version _ALU)             */
/*============================================================================*/

/******************************************************************************
Descripcion:    convierte una linea CSV de 10 campos en t_socio. Los campos
                con formato invalido quedan con valores que haran fallar la
                validacion correspondiente (dni = 0, fecha 0/0/0, sexo '?',
                etc.). El estado se fija en 'A'.
Parametros:     linea: linea CSV.
                reg: puntero a t_socio destino.
Retorno:        OK si la cantidad de campos es correcta, ERROR en caso
                contrario.
Observaciones:  Se pasa por puntero como parsear a leer_archivo_texto.
******************************************************************************/
int texto_a_socio_ALU(const char *linea, void *reg)
{
    t_socio *s = (t_socio *)reg;
    char aux[TAM_LINEA];
    char *campos[CAMPOS_SOCIO];

    memset(s, 0, sizeof(t_socio));
    copiar_cadena(aux, linea, TAM_LINEA);
    if(separar_campos_ALU(aux, campos, CAMPOS_SOCIO) != CAMPOS_SOCIO)
        return ERROR;

    if(es_numero(campos[SOC_DNI]))
        sscanf(campos[SOC_DNI], "%ld", &s->dni);
    copiar_cadena(s->cuil, campos[SOC_CUIL], TAM_CUIL);
    copiar_cadena(s->apellidos_nombres, campos[SOC_APYN], TAM_APYN);
    texto_a_fecha(campos[SOC_NACIMIENTO], &s->fecha_nacimiento);
    s->sexo = strlen(campos[SOC_SEXO]) == 1 ? *campos[SOC_SEXO] : SEXO_INVALIDO;
    texto_a_fecha(campos[SOC_AFILIACION], &s->fecha_afiliacion);
    copiar_cadena(s->categoria, campos[SOC_CATEGORIA], TAM_CATEGORIA);
    texto_a_fecha(campos[SOC_ULTIMA_CUOTA], &s->fecha_ultima_cuota);
    s->estado = ESTADO_ACTIVO;
    copiar_cadena(s->plan, campos[SOC_PLAN], TAM_PLAN);
    copiar_cadena(s->email_tutor, campos[SOC_EMAIL], TAM_EMAIL);
    return OK;
}

/******************************************************************************
Descripcion:    aplica las validaciones del punto 6.1 en orden (DNI, Sexo,
                CUIL, Apellidos y Nombres, Fecha de nacimiento, Fecha de
                afiliacion, Categoria, Fecha de ultima cuota, Plan, Email
                tutor). Normaliza apellidos y nombres. Se detiene en el primer
                error.
Parametros:     reg: puntero a t_socio a validar (se modifica al normalizar).
                ctx: contexto con indice, archivo binario, fecha de proceso e
                id_modificando.
                tipo_error: salida; nombre de la validacion que fallo.
Retorno:        OK si el socio es valido, ERROR en caso contrario.
Observaciones:  Si ctx->id_modificando == dni no se considera duplicado y se
                excluye ese DNI al verificar unicidad del email. Cumple con el
                puntero validar de leer_archivo_texto.
******************************************************************************/
int valida_socio_ALU(void *reg, const t_contexto *ctx, char *tipo_error)
{
    t_socio *s = (t_socio *)reg;
    t_reg_indice elem;
    char cuil[TAM_CUIL];
    char apyn[TAM_LINEA];
    int anios;

    elem.id = s->dni;
    if(s->dni <= DNI_MIN || s->dni >= DNI_MAX ||(s->dni != ctx->id_modificando && indice_buscar_ALU(ctx->indice, &elem, sizeof(t_reg_indice), compara_reg_indice_ALU) != NO_EXISTE))
        return rechazo(tipo_error, "DNI");

    if(s->sexo != SEXO_HOMBRE && s->sexo != SEXO_MUJER)
        return rechazo(tipo_error, "SEXO");

    calcular_cuil_ALU(s->dni, s->sexo, cuil);
    if(strcmp(cuil, s->cuil) != 0)
        return rechazo(tipo_error, "CUIL");

    strcpy(apyn, s->apellidos_nombres);
    arreglar_apyn_ALU(apyn);
    copiar_cadena(s->apellidos_nombres, apyn, TAM_APYN);
    if(cadena_vacia(s->apellidos_nombres))
        return rechazo(tipo_error, "APELLIDOS_NOMBRES");

    if(!fecha_correcta(&s->fecha_nacimiento) || edad(&s->fecha_nacimiento, ctx->fecha_proceso) < EDAD_MINIMA)
        return rechazo(tipo_error, "FECHA_NACIMIENTO");

    if(!fecha_correcta(&s->fecha_afiliacion) || compara_fechas(&s->fecha_afiliacion, &s->fecha_nacimiento) <= 0 || compara_fechas(&s->fecha_afiliacion, ctx->fecha_proceso) > 0)
        return rechazo(tipo_error, "FECHA_AFILIACION");

    anios = edad(&s->fecha_nacimiento, ctx->fecha_proceso);
    if(strcmp(s->categoria, CAT_MENOR) == 0)
    {
        if(anios >= EDAD_ADULTO)
            return rechazo(tipo_error, "CATEGORIA");
    }
    else if(strcmp(s->categoria, CAT_ADULTO) == 0)
    {
        if(anios < EDAD_ADULTO)
            return rechazo(tipo_error, "CATEGORIA");
    }
    else
        return rechazo(tipo_error, "CATEGORIA");

    if(!fecha_correcta(&s->fecha_ultima_cuota) || compara_fechas(&s->fecha_ultima_cuota, &s->fecha_afiliacion) < 0 || compara_fechas(&s->fecha_ultima_cuota, ctx->fecha_proceso) > 0)
        return rechazo(tipo_error, "FECHA_ULTIMA_CUOTA");

    if(plan_indice(s->plan) == NO_EXISTE)
        return rechazo(tipo_error, "PLAN");

    if(strcmp(s->categoria, CAT_MENOR) == 0)
    {
        if(cadena_vacia(s->email_tutor) || !email_valido_ALU(s->email_tutor) || socio_email_existe(ctx->arch_bin, ctx->indice, s->email_tutor, ctx->id_modificando))
            return rechazo(tipo_error, "EMAIL_TUTOR");
    }
    else if(!cadena_vacia(s->email_tutor))
        return rechazo(tipo_error, "EMAIL_TUTOR");

    return OK;
}

/******************************************************************************
Descripcion:    lee el registro nro_reg del archivo binario de socios.
Parametros:     arch: archivo abierto en modo binario.
                nro_reg: numero de registro.
                s: socio destino.
Retorno:        OK si se pudo leer, ERROR en caso contrario.
Observaciones:  -
******************************************************************************/
int socio_leer_ALU(FILE *arch, unsigned nro_reg, t_socio *s)
{
    if(fseek(arch, (long)nro_reg * (long)sizeof(t_socio), SEEK_SET) != 0)
        return ERROR;
    return fread(s, sizeof(t_socio), 1, arch) == 1 ? OK : ERROR;
}

/******************************************************************************
Descripcion:    escribe el socio en la posicion nro_reg del archivo binario y
                hace flush.
Parametros:     arch: archivo abierto en modo de escritura binaria.
                nro_reg: numero de registro.
                s: socio a grabar.
Retorno:        OK si se pudo grabar, ERROR en caso contrario.
Observaciones:  -
******************************************************************************/
int socio_escribir_ALU(FILE *arch, unsigned nro_reg, const t_socio *s)
{
    if(fseek(arch, (long)nro_reg * (long)sizeof(t_socio), SEEK_SET) != 0)
        return ERROR;
    if(fwrite(s, sizeof(t_socio), 1, arch) != 1)
        return ERROR;
    fflush(arch);
    return OK;
}

/******************************************************************************
Descripcion:    aplica la regla del Anexo A.4: el socio esta al dia si la
                ultima cuota es del mes de la fecha de proceso, o del mes
                anterior cuando el dia de la fecha de proceso es <= 10.
Parametros:     ultima_cuota: fecha de ultima cuota paga.
                fecha_proceso: fecha de proceso.
Retorno:        1 si esta al dia, 0 en caso contrario.
Observaciones:  -
******************************************************************************/
int cuota_al_dia_ALU(const t_fecha *ultima_cuota, const t_fecha *fecha_proceso)
{
    int mes_ant = fecha_proceso->mes == 1 ? 12 : fecha_proceso->mes - 1;
    int anio_ant = fecha_proceso->mes == 1 ? fecha_proceso->anio - 1 : fecha_proceso->anio;

    if(ultima_cuota->mes == fecha_proceso->mes && ultima_cuota->anio == fecha_proceso->anio)
        return 1;
    return fecha_proceso->dia <= DIA_LIMITE_CUOTA &&
           ultima_cuota->mes == mes_ant && ultima_cuota->anio == anio_ant;
}

/******************************************************************************
Descripcion:    opcion Alta: carga un socio por teclado, lo valida con
                valida_socio, lo agrega al final de socios.dat con estado 'A'
                y lo inserta en el indice. Si es rechazado lo audita.
Parametros:     indice: indice de socios.
                path: ruta de socios.dat.
                fecha_proceso: fecha de proceso.
Retorno:        OK si el socio fue agregado, ERROR en caso contrario.
Observaciones:  -
******************************************************************************/
int socio_alta_manual_ALU(t_indice *indice, const char *path, const t_fecha *fecha_proceso)
{
    char campos[CAMPOS_SOCIO][TAM_CAMPO_TECLADO];
    char linea[TAM_LINEA_SOCIO];
    char tipo_error[TAM_TIPO_ERROR];
    t_socio s;
    t_contexto ctx;
    t_reg_indice elem;
    FILE *pf;
    int i;

    pf = fopen(path, "r+b");
    if(!pf)
    {
        printf("No se pudo abrir %s.\n", path);
        return ERROR;
    }

    for(i = 0; i < CAMPOS_SOCIO; i++)
        pedir_campo_socio(i, NULL, campos[i]);
    if(strcmp(campos[SOC_EMAIL], EMAIL_BORRAR) == 0)
        campos[SOC_EMAIL][0] = '\0';
    armar_linea_socio(linea, campos);

    ctx.indice = indice;
    ctx.arch_bin = pf;
    ctx.fecha_proceso = fecha_proceso;
    ctx.id_modificando = 0;

    if(socio_desde_linea(linea, &s, &ctx, tipo_error) != OK)
    {
        printf("\nSocio RECHAZADO. Validacion fallida: %s\n", tipo_error);
        auditoria_registrar_ALU(ARCH_ERRORES, campos[SOC_DNI], tipo_error, fecha_proceso);
        fclose(pf);
        return ERROR;
    }

    fseek(pf, 0L, SEEK_END);
    elem.nro_reg = (unsigned)(ftell(pf) / sizeof(t_socio));
    elem.id = s.dni;
    if(socio_escribir_ALU(pf, elem.nro_reg, &s) != OK || indice_insertar_ALU(indice, &elem, sizeof(t_reg_indice), compara_reg_indice_ALU) != OK)
    {
        printf("\nNo se pudo grabar el socio.\n");
        fclose(pf);
        return ERROR;
    }
    printf("\nSocio agregado (registro %u).\n", elem.nro_reg);
    fclose(pf);
    return OK;
}

/******************************************************************************
Descripcion:    opcion Modificacion: pide el DNI, lo busca en el indice (un
                socio dado de baja no se puede modificar), muestra los datos
                actuales, pide los nuevos (Enter conserva), valida y
                sobrescribe el registro. El DNI y el estado no se modifican.
                Si es rechazado lo audita.
Parametros:     indice: indice de socios.
                path: ruta de socios.dat.
                fecha_proceso: fecha de proceso.
Retorno:        OK si se modifico, ERROR en caso contrario.
Observaciones:  -
******************************************************************************/
int socio_modificacion_manual_ALU(t_indice *indice, const char *path, const t_fecha *fecha_proceso)
{
    char actuales[CAMPOS_SOCIO][TAM_CAMPO_TECLADO];
    char nuevo[TAM_CAMPO_TECLADO];
    char linea[TAM_LINEA_SOCIO];
    char tipo_error[TAM_TIPO_ERROR];
    t_socio s;
    t_contexto ctx;
    t_reg_indice elem;
    FILE *pf;
    int i;

    printf("DNI del socio a modificar: ");
    if(!leer_numero(&elem.id))
    {
        printf("DNI invalido.\n");
        return ERROR;
    }
    if(indice_buscar_ALU(indice, &elem, sizeof(t_reg_indice), compara_reg_indice_ALU) == NO_EXISTE)
    {
        printf("El socio no existe o esta dado de baja.\n");
        return ERROR;
    }
    pf = fopen(path, "r+b");
    if(!pf)
    {
        printf("No se pudo abrir %s.\n", path);
        return ERROR;
    }
    if(socio_leer_ALU(pf, elem.nro_reg, &s) != OK)
    {
        printf("No se pudo leer el registro.\n");
        fclose(pf);
        return ERROR;
    }

    printf("\nDatos actuales:\n");
    socios_encabezado();
    socio_mostrar(&s);
    printf("\n(Enter para conservar el valor actual)\n");

    socio_a_campos(&s, actuales);
    for(i = SOC_CUIL; i < CAMPOS_SOCIO; i++)
    {
        pedir_campo_socio(i, actuales[i], nuevo);
        if(i == SOC_EMAIL && strcmp(nuevo, EMAIL_BORRAR) == 0)
            actuales[i][0] = '\0';
        else if(!cadena_vacia(nuevo))
            strcpy(actuales[i], nuevo);
    }
    armar_linea_socio(linea, actuales);

    ctx.indice = indice;
    ctx.arch_bin = pf;
    ctx.fecha_proceso = fecha_proceso;
    ctx.id_modificando = s.dni;

    if(socio_desde_linea(linea, &s, &ctx, tipo_error) != OK)
    {
        printf("\nModificacion RECHAZADA. Validacion fallida: %s\n", tipo_error);
        auditoria_registrar_ALU(ARCH_ERRORES, actuales[SOC_DNI], tipo_error, fecha_proceso);
        fclose(pf);
        return ERROR;
    }
    if(socio_escribir_ALU(pf, elem.nro_reg, &s) != OK)
    {
        printf("\nNo se pudo grabar el socio.\n");
        fclose(pf);
        return ERROR;
    }
    printf("\nSocio modificado.\n");
    fclose(pf);
    return OK;
}

/******************************************************************************
Descripcion:    baja logica de socio: busca el DNI en el indice, pone estado
                'B' en el registro del archivo y lo elimina del indice.
Parametros:     indice: indice de socios.
                path: ruta de socios.dat.
Retorno:        OK si se dio de baja, ERROR si no existe o ya estaba dado de
                baja.
Observaciones:  -
******************************************************************************/
int socio_baja_ALU(t_indice *indice, const char *path)
{
    t_reg_indice elem;
    t_socio s;
    FILE *pf;

    printf("DNI del socio a dar de baja: ");
    if(!leer_numero(&elem.id))
    {
        printf("DNI invalido.\n");
        return ERROR;
    }
    if(indice_buscar_ALU(indice, &elem, sizeof(t_reg_indice), compara_reg_indice_ALU) == NO_EXISTE)
    {
        printf("El socio no existe o ya esta dado de baja.\n");
        return ERROR;
    }
    pf = fopen(path, "r+b");
    if(!pf)
    {
        printf("No se pudo abrir %s.\n", path);
        return ERROR;
    }
    if(socio_leer_ALU(pf, elem.nro_reg, &s) != OK)
    {
        printf("No se pudo leer el registro.\n");
        fclose(pf);
        return ERROR;
    }
    s.estado = ESTADO_BAJA;
    if(socio_escribir_ALU(pf, elem.nro_reg, &s) != OK)
    {
        printf("No se pudo grabar el registro.\n");
        fclose(pf);
        return ERROR;
    }
    fclose(pf);
    indice_eliminar_ALU(indice, &elem, sizeof(t_reg_indice), compara_reg_indice_ALU);
    printf("Socio %ld (%s) dado de baja.\n", s.dni, s.apellidos_nombres);
    return OK;
}

/******************************************************************************
Descripcion:    recorre el indice (ordenado por DNI y solo con activos) y
                lista los socios leyendo cada registro del archivo.
Parametros:     indice: indice de socios.
                path: ruta de socios.dat.
Retorno:        OK si se pudo abrir el archivo, ERROR en caso contrario.
Observaciones:  -
******************************************************************************/
int socios_listar_ALU(const t_indice *indice, const char *path)
{
    const t_reg_indice *elem = (const t_reg_indice *)indice->vindice;
    const t_reg_indice *fin = elem + indice->cantidad_elementos_actual;
    t_socio s;
    FILE *pf;

    pf = fopen(path, "rb");
    if(!pf)
    {
        printf("No se pudo abrir %s.\n", path);
        return ERROR;
    }
    socios_encabezado();
    for(; elem < fin; elem++)
        if(socio_leer_ALU(pf, elem->nro_reg, &s) == OK)
            socio_mostrar(&s);
    printf("\nTotal de socios activos: %u\n", indice->cantidad_elementos_actual);
    fclose(pf);
    return OK;
}

/******************************************************************************
Descripcion:    recorre el indice y exporta a un archivo binario los socios
                con plan PREMIUM o VIP y cuota al dia (el estado 'A' lo
                garantiza el indice). Sobrescribe el archivo de exportacion
                anterior.
Parametros:     indice: indice de socios.
                path: ruta de socios.dat.
                path_export: ruta de beta_testers.dat.
                fecha_proceso: fecha de proceso.
Retorno:        OK si se pudo exportar, ERROR en caso contrario.
Observaciones:  -
******************************************************************************/
int socios_exportar_ALU(const t_indice *indice, const char *path, const char *path_export, const t_fecha *fecha_proceso)
{
    const t_reg_indice *elem = (const t_reg_indice *)indice->vindice;
    const t_reg_indice *fin = elem + indice->cantidad_elementos_actual;
    t_socio s;
    FILE *pf, *pexp;
    unsigned exportados = 0;

    pf = fopen(path, "rb");
    if(!pf)
    {
        printf("No se pudo abrir %s.\n", path);
        return ERROR;
    }
    pexp = fopen(path_export, "wb");
    if(!pexp)
    {
        printf("No se pudo abrir %s.\n", path_export);
        fclose(pf);
        return ERROR;
    }
    for(; elem < fin; elem++)
        if(socio_leer_ALU(pf, elem->nro_reg, &s) == OK && (strcmp(s.plan, PLAN_PREMIUM) == 0 || strcmp(s.plan, PLAN_VIP) == 0) && cuota_al_dia_ALU(&s.fecha_ultima_cuota, fecha_proceso) && fwrite(&s, sizeof(t_socio), 1, pexp) == 1)
            exportados++;
    fclose(pexp);
    fclose(pf);
    printf("Socios exportados a %s: %u\n", path_export, exportados);
    return OK;
}

/******************************************************************************
Descripcion:    opcion 2 del menu: submenu para elegir entre alta y
                modificacion manual de socio.
Parametros:     ind_socios: indice de socios.
                fecha_proceso: fecha de proceso.
Retorno:        No posee.
Observaciones:  -
******************************************************************************/
void abm_socio_ALU(t_indice *indice, const t_fecha *fecha_proceso)
{
    const char opciones[][LENGTH_MENU] =
    {
        "AMV",
        "Alta",
        "Modificacion",
        "Volver"
    };

    switch(menu(opciones, "Alta / Modificacion de socio"))
    {
        case 'A':
            socio_alta_manual_ALU(indice, ARCH_SOCIOS_BIN, fecha_proceso);
            break;
        case 'M':
            socio_modificacion_manual_ALU(indice, ARCH_SOCIOS_BIN, fecha_proceso);
            break;
    }
}

int rechazo(char *tipo_error, const char *nombre)
{
    copiar_cadena(tipo_error, nombre, TAM_TIPO_ERROR);
    return ERROR;
}

int socio_desde_linea(const char *linea, t_socio *s, const t_contexto *ctx, char *tipo_error)
{
    if(texto_a_socio_ALU(linea, s) != OK)
        return rechazo(tipo_error, TIPO_ERROR_FORMATO);
    return valida_socio_ALU(s, ctx, tipo_error);
}

void socios_encabezado(void)
{
    printf("%-9s %-13s %-30s %-10s %s %-10s %-6s %-10s %s %-7s %s\n", "DNI", "CUIL",
           "APELLIDOS Y NOMBRES", "NACIMIENTO", "S", "AFILIACION", "CATEG.", "ULT.CUOTA", "E",
           "PLAN", "EMAIL TUTOR");
}

int leer_numero(long *numero)
{
    char texto[TAM_CAMPO_TECLADO];

    leer_linea(texto, TAM_CAMPO_TECLADO);
    if(cadena_vacia(texto) || !es_numero(texto))
        return 0;
    return sscanf(texto, "%ld", numero) == 1;
}

void pedir_campo_socio(int campo, const char *actual, char *dest)
{
    const char *etiquetas[CAMPOS_SOCIO] =
    {
        "DNI",
        "CUIL (XY-NNNNNNNN-Z)",
        "Apellidos y nombres",
        "Fecha de nacimiento",
        "Sexo (H/M)",
        "Fecha de afiliacion",
        "Categoria (MENOR/ADULTO)",
        "Fecha de ultima cuota paga",
        "Plan (BASIC/PREMIUM/VIP/FAMILY)",
        "Email tutor (vacio si ADULTO, '-' para borrar)"
    };

    if(actual)
        printf("%s [%s]: ", etiquetas[campo], actual);
    else
        printf("%s: ", etiquetas[campo]);
    leer_linea(dest, TAM_CAMPO_TECLADO);
}

void socio_a_campos(const t_socio *s, char campos[][TAM_CAMPO_TECLADO])
{
    sprintf(campos[SOC_DNI], "%ld", s->dni);
    strcpy(campos[SOC_CUIL], s->cuil);
    strcpy(campos[SOC_APYN], s->apellidos_nombres);
    sprintf(campos[SOC_NACIMIENTO], "%02d/%02d/%04d", s->fecha_nacimiento.dia,
            s->fecha_nacimiento.mes, s->fecha_nacimiento.anio);
    sprintf(campos[SOC_SEXO], "%c", s->sexo);
    sprintf(campos[SOC_AFILIACION], "%02d/%02d/%04d", s->fecha_afiliacion.dia,
            s->fecha_afiliacion.mes, s->fecha_afiliacion.anio);
    strcpy(campos[SOC_CATEGORIA], s->categoria);
    sprintf(campos[SOC_ULTIMA_CUOTA], "%02d/%02d/%04d", s->fecha_ultima_cuota.dia,
            s->fecha_ultima_cuota.mes, s->fecha_ultima_cuota.anio);
    strcpy(campos[SOC_PLAN], s->plan);
    strcpy(campos[SOC_EMAIL], s->email_tutor);
}

void armar_linea_socio(char *linea, char campos[][TAM_CAMPO_TECLADO])
{
    sprintf(linea, "%s;%s;%s;%s;%s;%s;%s;%s;%s;%s",
             campos[0], campos[1], campos[2], campos[3], campos[4],
             campos[5], campos[6], campos[7], campos[8], campos[9]);
}
