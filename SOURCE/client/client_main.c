#include <arpa/inet.h>
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#include "../condiviso/types.h"
#include "../condiviso/utility.h"
#include "../condiviso/network.h"
#include "help.h"
#include "request.h"

#define min(X, Y)  ((X) < (Y) ? (X) : (Y))

int non_raggiungibile = 0;

void closing_handler(int signal){
	printf("Error: not connected to server\n");
	non_raggiungibile = 1;
}

// data la risposta del server _msg, stampa le giocate richieste
enum ERROR display_giocate(char* _msg){
	const char* formato = "RUOTE_GIOCATE: %s\n"
								 "NUMERI_GIOCATI:%[^\n]\n"
								 "IMPORTI: %d %d %d %d %d\n%n";
	char r_giocate[N_RUOTE+1];
	char n_giocati[50];
	int importi[N_SCALA];
	int num_chars;
	int giocate_counter = 1;

	// ogni iterazione del loop legge una giocata da _msg e ne stampa i valori
	while(1){
		printf("\n");
		int res = sscanf(_msg, formato, r_giocate, n_giocati,
						 &importi[0], &importi[1], &importi[2], &importi[3], &importi[4],
						 &num_chars);
		// giocata mal formattata o fine di _msg
		if (res < 7) break;

		_msg = &_msg[num_chars]; // avanzo il puntatore per parsare le giocate
		printf(" %d)", giocate_counter++);
		// stampa le ruote
		if (strcmp(r_giocate, "XXXXXXXXXXX") == 0)
			printf(" tutte");
		else{
			for (int i=0; i<N_RUOTE; i++){
				if (r_giocate[i] == '-') continue;
				enum RUOTA r = (enum RUOTA ) i;
				printf(" %s", ruota_str(r));
			}
		}
		// stampo i numeri giocati
		printf("%s", n_giocati);
		// stampo gli importi in ordine inverso
		const char* scala[N_SCALA];
		scala[0] = "estratto";
		scala[1] = "ambo";
		scala[2] = "terno";
		scala[3] = "quaterna";
		scala[4] = "cinquina";
		for (int i=N_SCALA-1; i>=0; i--){
			if (importi[i] == 0)
				continue;
			double importo_euro = (float)importi[i] / 100;
			printf(" * %.2f %s", importo_euro, scala[i]);
		}

	}
	return NO_ERROR;
}

/////////////////////////////////////////	MAIN

int main (int argc, char *argv[]) {
	int sid, port;
	char *ip_address;
	struct sockaddr_in server_addr;
	signal(SIGPIPE, closing_handler);

	if(argc != 3){
		printf("ERRORE: specificare [IP] e [porta]\n");
		return 0;
	}
	// estraggo i parametri
	ip_address = argv[1];
	port = atoi(argv[2]);

	char* const msg_buf = (char*)malloc(MAX_MSG_LENGTH); //buffer su cui scrivere/leggere data per/da il server
	char session_id[SESS_ID_SIZE] = "0000000000";
	enum ERROR err = NO_ERROR;
	enum COMANDO command = NO_COMANDO;


	// Creazione socket
	sid = socket(AF_INET, SOCK_STREAM, 0);
	// Creazione indirizzo del server
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	inet_pton(AF_INET, ip_address, &server_addr.sin_addr);

	printf("Connecting to server...\n");
	int i = 0;
	while(i < 10 && connect(sid, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0){
		sleep(1);
		i++;
	}

	if(i == 10){
		printf("Impossibile connettersi al server: %s\n", strerror(errno));
		return 0;
	}

	mostra_help(NO_COMANDO); //stampa info di help

	// MAIN LOOP
	while(command != ESCI && err != BANNED && err != DISCONNECTED && non_raggiungibile == 0){
		// mostra eventuale errore da ciclo precedente
		if (err != NO_ERROR)
			mostra_errore(err);
		err = NO_ERROR;
		// pulitura del buffer, scrittura dell'header
		strcpy(msg_buf, "");
		sprintf(&msg_buf[strlen(msg_buf)], "CLIENT REQUEST\nSESSION_ID: %s\n", session_id);

		printf(">");
		// leggo linea di comando
		int max_input = 1000;
		char input[1000];
		if(fgets(input,max_input,stdin) == NULL)
			continue;
		int input_size = strlen(input);
		input[input_size - 1] = '\0'; // sostiuisco il \n finale con \0
		// sostituisco gli spazi bianchi con \0, dividendo input in sottostringhe
		for(i = 0; i < input_size; i++){
			if(input[i] != ' ')
				continue;

			input[i] = '\0';

			if (input[i+1] == ' '){
				printf("Bad Syntax: troppi spazi\n");
				err = SYNTAX_ERROR;
				break;
			}
		}

		if(err != NO_ERROR) continue; // riprova a leggere l'input

		// estraggo il comando come prima sottostringa
		char* str = input;
		command = str_command(str);
		str = &str[strlen(input) + 1]; // avanzo il puntatore str
		// scrivo il comando
		sprintf(&msg_buf[strlen(msg_buf)], "COMANDO: %d\n", (int)command);
		// finisco di scrivere la richiesta
		err = request(command, str, msg_buf);

		if (command == HELP) continue; // help non necessita di inviare alcuna richiesta
		if(err != NO_ERROR) continue; // se errore, riprova a leggere l'input
		// invio la richiesta
		err = invia_msg(sid, msg_buf);
		if(err != NO_ERROR) continue; //riprova a leggere l'input
		// ricevo la risposta
		err = ric_msg(sid, msg_buf);
		if(err != NO_ERROR) continue; //riprova a leggere l'input

		char* msg_punt = msg_buf; //pointer per parsare la risposta
		msg_punt = prox_riga(msg_punt); // salto la prima linea "SERVER RESPONSE"
		//leggo eventuali errori
		int errore_letto  = sscanf(msg_punt, "ERROR: %d\n", (int*)&err);
		if(errore_letto == 1) continue; //riprova a leggere l'input

		// mostro a video la risposta del server
		// oppure salvo la session_id inviata
		switch(command){
			case INVIA_GIOCATA:
				printf("Giocata inviata con successo\n");
				break;
			case LOGIN:
				sscanf(msg_punt, "SESSION_ID: %s\n", session_id);
				printf("Login avvenuto con successo\n");
				break;
			case SIGNUP:
				printf("Iscrizione avvenuta con successo\n");
				break;
			case VEDI_GIOCATE:
				err = display_giocate(msg_punt);
				break;
			case VEDI_ESTRAZIONI:
				printf("%s\n", msg_punt);
				break;
			case VEDI_VINCITE:
				printf("%s\n", msg_punt);
				break;
			case ESCI:
				printf("%s\n", msg_punt);
				break;
			default:
				break;
		}

	}

	close(sid);
	if(err == NO_ERROR)
		printf("Disconnessione avvenuta con successo\n");
	else{
		mostra_errore(err);
		printf("Disconnessione\n");
	}

	free(msg_buf);
	return 0;
}
