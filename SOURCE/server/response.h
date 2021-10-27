#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <time.h>
#include <dirent.h>
#include <unistd.h>
#include <ctype.h>

#include "../condiviso/types.h"
#include "../condiviso/utility.h"
#include "parametri.h"

typedef struct thread_slot thread_slot;

// ad ogni thread che serve un client corrisponde un thread_slot
// il thread_slot contiene informazioni relative al thread e alle sue risorse
struct thread_slot{
	pthread_t tid; // id del thread
	int sid; // id del socket relativo
	char* req_buf; // buffer for request message
	char* res_buf; // buffer for response message
	char session_id[11]; // id di sessione
	char user[50]; // nome dell'user connesso
	struct in_addr ip;
	int index, n_try; // ip del client (ridondante), index indice del parametro nell'array
	int exit; // quando posto 1, causa la terminazione del thread
};



/*
Un thread associato ad un client riceve messaggi testuali (richieste) dal client
Per ogni richiesta, genera un messaggio testuale di risposta, nel seguente formato:

"
------------------------------------------------- <HEADER>
SERVER RESPONSE\n
------------------------------------------------- <BODY>
[CAMPO]: [DATI]\n
[CAMPO]: [DATI]\n
...
[TEXT TO DISPLAY]
-------------------------------------------------
"

Alcune risposte includono [TEXT TO DISPLAY], ovvero un corpo di testo
che non viene processato dal client, ma direttamente stampato a video

*/



/* la funzione response legge il corpo della richiesta passata
 e scrive in res_punt il corpo della risposta da mandare al client
 restituisce NO_ERROR se non ci sono stati errori nella generazione della risposta
 si divide in sottofunzioni, una per ogni comando inviabile dal client*/
enum ERROR response(thread_slot* _thread_data, enum COMANDO command, char* msg_punt, char*res_punt);
