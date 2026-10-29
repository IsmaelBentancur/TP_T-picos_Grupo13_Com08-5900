#ifndef SOCIO_H_INCLUDED
#define SOCIO_H_INCLUDED

#include <stdio.h>
#include "fecha.h"
#include "indice.h"
#include "lectura.h"
#include "validaciones.h"

#define TAM_APYN      61
#define TAM_CATEGORIA 11
#define TAM_PLAN      11
#define CAMPOS_SOCIO  10

#define DNI_MIN   1000000L
#define DNI_MAX 100000000L

#define CAT_MENOR  "MENOR"
#define CAT_ADULTO "ADULTO"
#define EDAD_ADULTO 18
#define EDAD_MINIMA 10

#define CANT_PLANES 4
#define DIA_LIMITE_CUOTA 10

typedef struct
{
    long    dni;
    char    cuil[TAM_CUIL];
    char    apellidos_nombres[TAM_APYN];
    t_fecha fecha_nacimiento;
    char    sexo;
    t_fecha fecha_afiliacion;
    char    categoria[TAM_CATEGORIA];
    t_fecha fecha_ultima_cuota;
    char    estado;
    char    plan[TAM_PLAN];
    char    email_tutor[TAM_EMAIL];
} t_socio;

extern const char *const PLANES[CANT_PLANES];

/* funciones para la lectura generica */
int  texto_a_socio(const char *linea, void *reg);
int  valida_socio(void *reg, const t_contexto *ctx, char *tipo_error);
void rechaza_socio(const void *reg, const char *linea, const char *tipo_error,
                   const t_fecha *fecha_proceso);
long clave_socio(const void *reg);
int  activo_socio(const void *reg);

/* acceso al archivo binario */
int  socio_leer(FILE *arch, unsigned nro_reg, t_socio *s);
int  socio_escribir(FILE *arch, unsigned nro_reg, const t_socio *s);
int  socio_email_existe(FILE *arch, const t_indice *indice, const char *email, long dni_excluir);

/* utilidades */
int  plan_indice(const char *plan);
int  cuota_al_dia(const t_fecha *ultima_cuota, const t_fecha *fecha_proceso);
void socio_mostrar(const t_socio *s);

/* opciones de menu */
int  socio_alta_manual(t_indice *indice, const char *path, const t_fecha *fecha_proceso);
int  socio_modificacion_manual(t_indice *indice, const char *path, const t_fecha *fecha_proceso);
int  socio_baja(t_indice *indice, const char *path);
int  socios_listar(const t_indice *indice, const char *path);
int  socios_exportar(const t_indice *indice, const char *path, const char *path_export,
                     const t_fecha *fecha_proceso);

void abm_socio(t_indice *indice, const t_fecha *fecha_proceso);

/*============================================================================*/
/*            FUNCIONES A IMPLEMENTAR POR EL ALUMNO (version _ALU)             */
/*============================================================================*/

int texto_a_socio_ALU(const char *linea, void *reg);
int valida_socio_ALU(void *reg, const t_contexto *ctx, char *tipo_error);
int socio_leer_ALU(FILE *arch, unsigned nro_reg, t_socio *s);
int socio_escribir_ALU(FILE *arch, unsigned nro_reg, const t_socio *s);
int cuota_al_dia_ALU(const t_fecha *ultima_cuota, const t_fecha *fecha_proceso);
int socio_alta_manual_ALU(t_indice *indice, const char *path, const t_fecha *fecha_proceso);
int socio_modificacion_manual_ALU(t_indice *indice, const char *path, const t_fecha *fecha_proceso);
int socio_baja_ALU(t_indice *indice, const char *path);
int socios_listar_ALU(const t_indice *indice, const char *path);
int socios_exportar_ALU(const t_indice *indice, const char *path, const char *path_export,
                    const t_fecha *fecha_proceso);
void abm_socio_ALU(t_indice *indice, const t_fecha *fecha_proceso);

#define SOC_DNI           0
#define SOC_CUIL          1
#define SOC_APYN          2
#define SOC_NACIMIENTO    3
#define SOC_SEXO          4
#define SOC_AFILIACION    5
#define SOC_CATEGORIA     6
#define SOC_ULTIMA_CUOTA  7
#define SOC_PLAN          8
#define SOC_EMAIL         9

#define SEXO_HOMBRE   'H'
#define SEXO_MUJER    'M'
#define SEXO_INVALIDO '?'
#define PLAN_PREMIUM  "PREMIUM"
#define PLAN_VIP      "VIP"
#define EMAIL_BORRAR  "-"
#define TAM_CAMPO_TECLADO 64

#define TAM_LINEA_SOCIO (CAMPOS_SOCIO * TAM_CAMPO_TECLADO)

int  rechazo(char *tipo_error, const char *nombre);
int  socio_desde_linea(const char *linea, t_socio *s, const t_contexto *ctx, char *tipo_error);
void socios_encabezado(void);
int  leer_numero(long *numero);
void pedir_campo_socio(int campo, const char *actual, char *dest);
void socio_a_campos(const t_socio *s, char campos[][TAM_CAMPO_TECLADO]);
void armar_linea_socio(char *linea, char campos[][TAM_CAMPO_TECLADO]);

#endif /* SOCIO_H_INCLUDED */
