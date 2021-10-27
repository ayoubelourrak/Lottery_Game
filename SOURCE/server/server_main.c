#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <dirent.h>
#include <pthread.h>

#include "../condiviso/types.h"
#include "../condiviso/utility.h"
#include "../condiviso/network.h"
#include "response.h"
#include "estrazioni.h"

// massimo num di thread (e dunque utenti connessi) gestibili dal sistema
#define MAX_THREADS 1000

// massimo di richieste di connessione gestibili contemporaneamente
#define MAX_LISTENER 200

// struttura passata al thread che esegue le estrazioni
typedef struct{
	pthread_t tid; // id del thread
	int periodo; // periodo di estrazioni
} st_timer;

// array che contiene i puntatori ai thread_slot (ogni thread che serve un client usa un thread_slot)
thread_slot** thread_slots;
// numero attuale di thread
int N_threads;
// lock per la mutua esclusione sulle precedenti variabili
pthread_mutex_t thread_slots_lock;

// variabili per controllare che i thread che servono i clienti
// non processino l'inserimento di nuove giocate fintanto che c'è un'estrazione in corso
// altrimenti la giocata potrebbe venire persa
int durante_estraz;
pthread_mutex_t estraz_lock;
pthread_cond_t fine_estraz;


// restituisce la pos del primo thread_slot con il tid specificato
// se non lo trova, restituisce -1
int get_tid(pthread_t _tid){
	for (int i=0; i<N_threads; i++){
		if (thread_slots[i]->tid == _tid) return i;
	}

	return -1;
}

// crea il socket che accetta le connessioni dai client
int create_listener(int _port){
	int listener_;

	struct sockaddr_in server_addr; // Indirizzo server
	listener_ = socket(AF_INET, SOCK_STREAM, 0);
	server_addr.sin_family = AF_INET;

	// Mi metto in ascolto su tutte le interfacce (indirizzi IP)
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(_port);

	//effetto il bind
	if(bind(listener_, (struct sockaddr*) &server_addr, sizeof(server_addr)) != 0){
		printf("Bind del socket di ascolto all'uscita: Fallito\n");
		return 0;
	}

	// effettuo il listen
	if(listen(listener_, MAX_LISTENER) != 0){
		printf("Attivazione del socket di ascolto: Fallito\n");
		return 0;
	}

	return listener_;
}

// legge porta e periodo indicati nel comando di avvio
// restituisce 1 in caso di errore, 0 altrimenti
int leggi_dati(int _argc, char *_argv[], int *porta_, int *periodo_){

	if(_argc == 1){
		printf("ERRORE: dati mancanti\n");
		return 1;
	}
	else if(_argc == 2){
		*periodo_ = 300; // 300s = 5min
	    *porta_ = atoi(_argv[1]);
		return 0;
	}
	else if(_argc == 3){
		*periodo_ = atoi(_argv[2]);
		*porta_ = atoi(_argv[1]);
		return 0;
	}
	else if(_argc > 3){
		printf("ERRORE: troppi dati in ingresso\n");
		return 1;
	}
	return 1;
}

// handler in caso di SIG_PIPE
void closing_handler(int signal){

 	pthread_t tid = pthread_self();
	pthread_mutex_lock(&thread_slots_lock);
	int i = get_tid(tid);
	if (i == -1) return; // il thread corrente non e' uno di quelli che servono client
	printf("Thread %d: E' venuta a mancare la connessione con il client\n", i);
	fflush(stdout);
	thread_slots[i]->exit = 1; // indica al thread che deve terminare
	pthread_mutex_unlock(&thread_slots_lock);
}

// libera le risorse associate al thread di index _i nell'array di thread_slots
void libera_thread(int _i){
	int n = _i;
	printf("Thread %d: disconnessione dell'utente %s\n\n", n, thread_slots[n]->user);
	fflush(stdout);

	// rilascio delle risorse
	close(thread_slots[n]->sid);
	free(thread_slots[n]->req_buf);
	free(thread_slots[n]->res_buf);

	free(thread_slots[n]);
	thread_slots[n] = NULL;
	N_threads --;

	if (N_threads == n){
		return; // ho eliminato l'ultimo slot dell'array
	}

	// in caso contrario, devo compattare l'array
	thread_slots[n] = thread_slots[N_threads];
	thread_slots[n]->index = n;
}

// codice eseguito da ogni thread che serve una connessione utente
void *gestisci_socket(void* _args){

	thread_slot* thread_data = (thread_slot*)_args;
	enum ERROR err = NO_ERROR;
	enum COMANDO command = NO_COMANDO;

	// loop eseguito dal thread
	while(!thread_data->exit && err != BANNED && command!=ESCI){

		char* req_punt = thread_data->req_buf; // buffer for request message
		char* res_punt = thread_data->res_buf; // buffer for response message

		// mostra eventuale errore da ciclo precedente
		if (err != NO_ERROR){
			printf("Thread %d: ",thread_data->index);
			mostra_errore(err);
		}
		err = NO_ERROR;

		err = ric_msg(thread_data->sid, req_punt);
		if (thread_data->exit == 1) err = DISCONNECTED; // ha ricevuto SIG_PIPE
		if (err != NO_ERROR) break;

		printf("Thread %d received:\n", thread_data->index);
		printf("%s\n", req_punt);

		req_punt = prox_riga(req_punt); // salto la prima linea "CLIENT REQUEST"

		// leggo la SESSION ID
		char session_id[SESS_ID_SIZE];
		if (sscanf(req_punt, "SESSION_ID: %s", session_id) == 0){
			invia_errore(thread_data->sid, BAD_REQUEST);
			continue;
		}
		req_punt = prox_riga(req_punt);

		if(strcmp(thread_data->session_id, session_id) != 0){
			invia_errore(thread_data->sid, WRONG_SESSID);
			break; //disconnetti
		}

		// leggo il comando ricevuto
		if (sscanf(req_punt, "COMANDO: %d", (int*)&command) == 0)
			invia_errore(thread_data->sid, NO_COMANDO_FOUND);
		req_punt = prox_riga(req_punt);

		// compongo la risposta
		strcpy(res_punt, "SERVER RESPONSE\n");
		res_punt = prox_riga(res_punt);
		// non posso processare la richiesta mentre e' in corso un estrazione
		pthread_mutex_lock(&estraz_lock);
		if (durante_estraz == 1)
			pthread_cond_wait(&fine_estraz, &estraz_lock);
		pthread_mutex_unlock(&estraz_lock);
		err = response(thread_data, command, req_punt, res_punt);

		if (err == BANNED){
			invia_errore(thread_data->sid, err);
			printf("Tentativo di accesso da ip bloccato %s\n", inet_ntoa(thread_data->ip));
			err = NO_ERROR;
			break;  //disconnetti
		}

		if (err == DISCONNECTED) break;

		// rispondo al client
		if(err != NO_ERROR)
			err = invia_errore(thread_data->sid, err);
		else
			err = invia_msg(thread_data->sid, thread_data->res_buf);
	}

	//terminazione del thread
	//stampa eventuale errore
	if (err!=NO_ERROR){
		printf("Thread %d: ",thread_data->index);
		mostra_errore(err);
	}

	//rilascia risorse
	pthread_mutex_lock(&thread_slots_lock);
	libera_thread(thread_data->index);
	pthread_mutex_unlock(&thread_slots_lock);
	pthread_exit(NULL);
	return NULL;
}

// codice eseguito dal thread che esegue le estrazioni
void *gestisci_timer(void* _args){

	st_timer* timer = (st_timer*)_args;

	while(1){ //ciclo infinto
		sleep(timer->periodo); // aspetto la prossima estrazione
		pthread_mutex_lock(&estraz_lock);
		durante_estraz = 1;
		estrazione();
		durante_estraz = 0;
		pthread_cond_broadcast(&fine_estraz);
		pthread_mutex_unlock(&estraz_lock);
	}

	pthread_exit(NULL);
	return NULL;
}


int main(int argc, char *argv[]){

	int res; //tmp var per tenere il risultato di vari funzioni

	int periodo, porta;
	res = leggi_dati(argc, argv, &porta, &periodo);
	if (res != 0) return 0;
	signal(SIGPIPE, closing_handler);

	printf("Avvio gioco del Lotto\nPorta: %d\nPeriodo: %d sec\n\n", porta, periodo);

	// array di thread_slot, numero di thread attivi, e mutex per queste due variabili
	pthread_mutex_init(&thread_slots_lock, NULL);
	thread_slots = (thread_slot**)malloc(MAX_THREADS*sizeof(thread_slot*));
	N_threads = 0;

	durante_estraz = 0;
	pthread_mutex_init(&estraz_lock, NULL);
	pthread_cond_init (&fine_estraz, NULL);

	int listener; // Socket per l'ascolto
	listener = create_listener(porta);
	if(listener == 0){
		printf("Fallita creazione server: %s\n", strerror(errno));
		return 0;
	}

	printf("Server avviato con successo\n");

	st_timer timer; // parametro per il thread controllore
	timer.tid = 0;
	timer.periodo = periodo;
	pthread_create(&timer.tid, NULL, gestisci_timer, &timer); // avvio il timer

	// ciclo infinito
	while(1){
		if(N_threads < MAX_THREADS){
			struct sockaddr_in cl_addr; // Indirizzo client
			int addrlen = sizeof(cl_addr);
			int new_socket;
			new_socket = accept(listener, (struct sockaddr *)&cl_addr, (socklen_t*)&addrlen);

			pthread_mutex_lock(&thread_slots_lock);

			/* creo i parametri per il thread N-esimo che gestisce un socket */
			thread_slots[N_threads] = (thread_slot*)malloc(sizeof(thread_slot));
			thread_slots[N_threads]->sid = new_socket;
			thread_slots[N_threads]->req_buf = (char*)malloc(MAX_MSG_LENGTH); // buffer for request message
			thread_slots[N_threads]->res_buf = (char*)malloc(MAX_MSG_LENGTH); // buffer for response message
			thread_slots[N_threads]->ip =  cl_addr.sin_addr;
			thread_slots[N_threads]->n_try = 0;
			thread_slots[N_threads]->index = N_threads;
			strcpy(thread_slots[N_threads]->session_id, "0000000000");
			strcpy(thread_slots[N_threads]->user, "");
			thread_slots[N_threads]->exit = 0;

			char str[INET_ADDRSTRLEN];
			inet_ntop( AF_INET, &thread_slots[N_threads]->ip, str, INET_ADDRSTRLEN );
			printf("nuova connessione da: %s, posizione nell'array dei thread: %d \n", str, N_threads);

			 // avvio il gestore del client
			pthread_create(&thread_slots[N_threads]->tid, NULL, gestisci_socket, thread_slots[N_threads]);
			N_threads++;

			pthread_mutex_unlock(&thread_slots_lock);
		}
	}
}
