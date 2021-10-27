#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "../condiviso/types.h"
#include "../condiviso/utility.h"
#include "help.h"

/*
Il client manda al server le richieste in formato messaggi testo così costituito:

"
------------------------------------------------- <HEADER>
CLIENT REQUEST\n
SESSION_ID: [SESSION ID del messaggio]\n
COMANDO: [comando, in forma di numero (enum)]\n
------------------------------------------------- <BODY>
[CAMPO]: [DATI]\n
[CAMPO]: [DATI]\n
...
-------------------------------------------------
"

*/

/* la funzione request gestisce il comando passato (comm) e la stringa di input (str)
scrive in msg_buf il BODY della richiesta da inviare al server
restituisce NO_ERROR se non ci sono stati errori nella generazione della richiesta
si divide nelle varie sottofunzioni per ogni comando*/
enum ERROR request(enum COMANDO comm, char* str, char* msg_buf);
