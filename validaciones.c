#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "validaciones.h"
#include "cadena.h"

/*============================================================================*/
/*            FUNCIONES A IMPLEMENTAR POR EL ALUMNO (version _ALU)             */
/*============================================================================*/

/******************************************************************************
Descripcion:    calcula el CUIL con el algoritmo modulo 11 del Anexo A.1:
                prefijo 20 (hombre) o 27 (mujer), DNI a 8 digitos y digito
                verificador. Si el resto es 1 el prefijo pasa a 23 y Z es 9
                (hombre) o 4 (mujer).
Parametros:     dni: numero de documento.
                sexo: 'H' o 'M'.
                dest: buffer de al menos TAM_CUIL bytes donde se deja "XY-
                NNNNNNNN-Z".
Retorno:        No posee.
Observaciones:  -
******************************************************************************/
void calcular_cuil_ALU(long dni, char sexo, char *dest)
{
    const int pesos[CUIL_CANT_DIGITOS] = {5, 4, 3, 2, 7, 6, 5, 4, 3, 2};
    char digitos[TAM_CUIL];
    int prefijo = sexo == 'H' ? PREFIJO_HOMBRE : PREFIJO_MUJER;
    int suma = 0, resto, verificador, i;

    sprintf(digitos, "%02d%08ld", prefijo, dni);
    for(i = 0; i < CUIL_CANT_DIGITOS; i++)
        suma += (digitos[i] - '0') * pesos[i];

    resto = suma % CUIL_MODULO;
    if(resto == 0)
        verificador = 0;
    else if(resto == 1)
    {
        verificador = sexo == 'H' ? CUIL_Z_HOMBRE_RESTO1 : CUIL_Z_MUJER_RESTO1;
        prefijo = PREFIJO_RESTO1;
    }
    else
        verificador = CUIL_MODULO - resto;

    sprintf(dest, "%02d-%08ld-%d", prefijo, dni, verificador);
}

/******************************************************************************
Descripcion:    valida un email segun el Anexo A.3: hasta 30 caracteres sin
                espacios, una unica '@', parte local de letras/digitos/puntos
                que no empieza ni termina con punto, y dominio de
                letras/digitos/guiones/puntos con al menos un punto, que no
                empieza ni termina con punto o guion y cuyo ultimo segmento
                tiene al menos 2 letras.
Parametros:     email: cadena a validar.
Retorno:        1 si es valido, 0 en caso contrario.
Observaciones:  No verifica unicidad; eso lo hace socio_email_existe.
******************************************************************************/
int email_valido_ALU(const char *email)
{
    const char *arroba, *ultimo_punto, *ultimo, *p;
    size_t largo = strlen(email);

    if(largo == 0 || largo > EMAIL_MAX_LEN)
        return 0;
    ultimo = email + largo - 1;

    arroba = strchr(email, '@');
    if(!arroba || strchr(arroba + 1, '@'))
        return 0;

    if(arroba == email || *email == '.' || *(arroba - 1) == '.')
        return 0;
    for(p = email; p < arroba; p++)
        if(!ES_ALFANUM(*p) && *p != '.')
            return 0;

    if(arroba == ultimo || *(arroba + 1) == '.' || *(arroba + 1) == '-' || *ultimo == '.' || *ultimo == '-')
        return 0;
    for(p = arroba + 1; *p; p++)
        if(!ES_ALFANUM(*p) && *p != '-' && *p != '.')
            return 0;

    ultimo_punto = strrchr(arroba + 1, '.');
    if(!ultimo_punto || strlen(ultimo_punto + 1) < EMAIL_MIN_LETRAS_FINAL)
        return 0;
    for(p = ultimo_punto + 1; *p; p++)
        if(!ES_LETRA(*p))
            return 0;
    return 1;
}
