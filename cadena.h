#ifndef CADENA_H_INCLUDED
#define CADENA_H_INCLUDED

#include <stddef.h>

#define SEPARADOR_CSV ';'

void arreglar_apyn(char *);      /* normaliza "Apellidos, Nombres" */
void arreglar_titulo(char *);    /* normaliza titulos (sin coma) */
void quitar_salto(char *);       /* elimina '\n' y '\r' finales */
int  separar_campos(char *linea, char *campos[], int max);
int  es_numero(const char *);    /* solo digitos (opcionalmente '-') */
int  cadena_vacia(const char *);
void copiar_cadena(char *dest, const char *orig, size_t tam);

/*============================================================================*/
/*            FUNCIONES A IMPLEMENTAR POR EL ALUMNO (version _ALU)             */
/*============================================================================*/

void arreglar_apyn_ALU(char *s);
int separar_campos_ALU(char *linea, char *campos[], int max);

#define ES_MINUSCULA(c) ((c) >= 'a' && (c) <= 'z')
#define ES_MAYUSCULA(c) ((c) >= 'A' && (c) <= 'Z')
#define ES_LETRA(c)     (ES_MINUSCULA(c) || ES_MAYUSCULA(c))
#define ES_DIGITO(c)    ((c) >= '0' && (c) <= '9')
#define ES_ALFANUM(c)   (ES_LETRA(c) || ES_DIGITO(c))
#define ES_BLANCO(c)    ((c) == ' ' || (c) == '\t')
#define A_MAYUSCULA(c)  (ES_MINUSCULA(c) ? (c) - 'a' + 'A' : (c))
#define A_MINUSCULA(c)  (ES_MAYUSCULA(c) ? (c) - 'A' + 'a' : (c))

#endif /* CADENA_H_INCLUDED */
