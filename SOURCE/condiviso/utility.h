#ifndef UTILITY
#define UTILITY

#include <string.h>
#include <stdio.h>
#include "types.h"

void mostra_errore(enum ERROR err);

char* prox_riga(char* str);

int leggi_importo(const char* str);

enum COMANDO str_command(char *_str);

enum RUOTA str_ruota(char *_str);

const char* ruota_str(enum RUOTA);

int combinazioni(int N, int k); // calcolo delle possibili combinazioni

#endif
