#include "request.h"

#define min(X, Y)  ((X) < (Y) ? (X) : (Y))

// non genera alcuna richiesta
enum ERROR help(char *_inputs, char* msg_){
	if(_inputs[0] == '\0'){
		mostra_help(NO_COMANDO);
		return NO_ERROR;
	}
	enum COMANDO c = str_command(_inputs);
	if(c == NO_COMANDO)
		printf("Comando non esistente\nUsa !help per vedere la lista dei comandi\n");
	else
		mostra_help(c);

	return NO_ERROR;
}

/* corpo della richiesta generata:
USER: [username]\n
PASSWORD: [password]\n
*/
enum ERROR signup(char *_inputs, char* msg_){
	int user_length, pass_length;
	char *user, *password;

	// leggo i parametri
	user = _inputs;
	password = &_inputs[strlen(_inputs) + 1];

	if(*user == '\0' || *password == '\0') return SYNTAX_ERROR;

    for (int i=0; user[i] != '\0'; i++){
        if (!isalnum(user[i])){
            printf("Username deve avere valori alfanumerici");
            return SYNTAX_ERROR;
        }
    }

	user_length = strlen(user) + 1;
	pass_length = strlen(password) + 1;

	// credenziali size tra 5 e 50 char
	if(pass_length < 6 || user_length < 6 || pass_length > 51 || user_length > 51)
		return WRONG_CREDENTIALS_SIZE;

	sprintf(&msg_[strlen(msg_)], "USER: %s\n", user);
	sprintf(&msg_[strlen(msg_)], "PASSWORD: %s\n", password);

	return NO_ERROR;
}

/* corpo della richiesta generata:
USER: [username]\n
PASSWORD: [password]\n
*/
enum ERROR login(char *_inputs, char* msg_){
	char *user, *password;

	// leggo i parametri
	user = _inputs;
	password = &_inputs[strlen(_inputs) + 1];

	if(*user == '\0' || *password == '\0'){
		return SYNTAX_ERROR;
	}

	sprintf(&msg_[strlen(msg_)], "USER: %s\n", user);
	sprintf(&msg_[strlen(msg_)], "PASSWORD: %s\n", password);

	return NO_ERROR;
}

enum ERROR invia_giocata(char *_inputs, char* msg_){
	enum RUOTA r;
	/* r_giocate e' una stringa che rappresenta le ruote giocate
	 Essendo enum, RUOTA e' un intero che va da 0 a N_RUOTE
	 r_giocate[R] == 'X' se la ruota R viene giocata, altrimenti '-'
	 r_giocate ha lunghezza N_RUOTE+1 (per il '\0')*/
	char r_giocate[N_RUOTE+1] = "-----------";

	if(strcmp(_inputs, "-r") != 0)
		return SYNTAX_ERROR;

	// scorro _inputs, leggendo le varie ruote.
	// mi fermo quando arrivo a "-n" o alla fine della stringa
	_inputs = &_inputs[strlen(_inputs) + 1]; //salto la stringa "-r"
	for(; strcmp(_inputs, "-n") != 0; _inputs = &_inputs[strlen(_inputs) + 1]){
		// se e' finita la stringa, errore
		if(_inputs[0] == '\0') return SYNTAX_ERROR;

		r = str_ruota(_inputs);
		if(r == NO_RUOTA){
			printf("Questa ruota non esiste\n");
			return SYNTAX_ERROR;
		}

		if(r == TUTTE)
			strcpy(r_giocate, "XXXXXXXXXXX");
		else
			r_giocate[r] = 'X'; // in quanto enum, RUOTA e' un intero che va da 0 a N_RUOTE
	}
	sprintf(&msg_[strlen(msg_)], "RUOTE_GIOCATE: %s\n", r_giocate);

	// scorro _inputs, leggendo da 1 a 10 interi
	// mi fermo quando arrivo a "-i" o alla fine della stringa
	sprintf(&msg_[strlen(msg_)], "NUMERI_GIOCATI:"); // inizio linea dei numeri giocati
	int num_numeri = 0; // quanti numeri ho gia letto
	_inputs = &_inputs[strlen(_inputs) + 1]; //salto la stringa "-n"

	for(; strcmp(_inputs, "-i") != 0; _inputs = &_inputs[strlen(_inputs) + 1]){
		// se e' finita la stringa, oppure ho gia' letto 10 numeri, errore
		if(_inputs[0] == '\0' || num_numeri >= 10) return SYNTAX_ERROR;
		int num;
		if (sscanf(_inputs, "%d", &num) == 0) return SYNTAX_ERROR;
		if(num > 90 || num <= 0) return SYNTAX_ERROR;
		sprintf(&msg_[strlen(msg_)], " %d", num);
		num_numeri ++;
	}
	sprintf(&msg_[strlen(msg_)], "\n"); // chiudo la linea dei numeri giocati

	// scorro _inputs, leggendo gli importi (tanti quanti (numeri giocati, N_SCALA))
	// mi fermo quando arrivo alla fine della stringa
	sprintf(&msg_[strlen(msg_)], "IMPORTI:"); // inizio linea dei numeri giocati
	int num_importi = 0; // quanti importi ho gia letto
	_inputs = &_inputs[strlen(_inputs) + 1]; // salto la sottostringa "-i"
	for(; _inputs[0] != '\0'; _inputs = &_inputs[strlen(_inputs) + 1]){
		int importo = leggi_importo(_inputs);
		if (importo == -1) return SYNTAX_ERROR;
		sprintf(&msg_[strlen(msg_)], " %d", importo);
		num_importi ++;
	}

	// condizione sul numero di importi che devo aver letto
	if (num_importi != min(num_numeri,N_SCALA)){
			printf("Numero di importi indicati scorretto\n");
			return SYNTAX_ERROR;
	}
	// aggiungo importi nulli finche' num_importi non e' a 5
	for (; num_importi<N_SCALA; num_importi++){
		sprintf(&msg_[strlen(msg_)], " %d", 0);
	}
	sprintf(&msg_[strlen(msg_)], "\n"); // chiudo la linea dei numeri giocati

	return NO_ERROR;
}

enum ERROR vedi_giocate(char *_inputs, char* msg_){
	if (_inputs[0] != '0' && _inputs[0] != '1'){
		printf("TIPO deve valere 0 o 1\n");
		return SYNTAX_ERROR;
	}
	sprintf(&msg_[strlen(msg_)], "TIPO: %c\n", _inputs[0]);
	return NO_ERROR;
}

enum ERROR vedi_estrazioni(char *_inputs, char* msg_){
	if (_inputs[0] == '\0') return SYNTAX_ERROR;

	int n_estrazioni;

	if (sscanf(_inputs, "%d", &n_estrazioni) == 0)
		return SYNTAX_ERROR;
	if (n_estrazioni < 0)
		return SYNTAX_ERROR;
	sprintf(&msg_[strlen(msg_)], "N_ESTRAZIONI: %d\n", n_estrazioni);

	_inputs = &_inputs[strlen(_inputs) + 1]; // vado avanti a leggere _inputs

	enum RUOTA r;
	if(_inputs[0] == '\0') // si ricorda che _inputs finisce con doppio \0
		r = TUTTE;
	else{
		r = str_ruota(_inputs);
		if (r == NO_RUOTA) return SYNTAX_ERROR;
	}

	sprintf(&msg_[strlen(msg_)], "RUOTA: %d\n", (int)r);
	return NO_ERROR;
}

enum ERROR request(enum COMANDO command, char* str, char* msg_buf){
	enum ERROR e_;
	switch(command){
		case HELP:
			e_ = help(str, msg_buf);
			break;
		case LOGIN:
			e_ = login(str, msg_buf);
			break;
		case SIGNUP:
			e_ = signup(str, msg_buf);
			break;
		case INVIA_GIOCATA:
			e_ = invia_giocata(str, msg_buf);
			break;
		case VEDI_GIOCATE:
			e_ = vedi_giocate(str, msg_buf);
			break;
		case VEDI_ESTRAZIONI:
			e_ = vedi_estrazioni(str, msg_buf);
			break;
		case VEDI_VINCITE:
			e_ = NO_ERROR;
			break;
		case ESCI:
			e_ = NO_ERROR;
			break;
		case NO_COMANDO:
			e_ = NO_COMANDO_FOUND; //riprova a leggere l'input
			break;
	}
	return e_;
}
