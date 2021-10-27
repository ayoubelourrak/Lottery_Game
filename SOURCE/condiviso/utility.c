#include "utility.h"

// restituisce il puntatore della riga successiva o NULL se non c'è
char* prox_riga(char* str){
	for(int i=0; str[i] != '\0' && i < 10000; i++){
		if (str[i] == '\n'){
			return &str[i+1];
		}
	};
	return NULL;
}

// restituisce un intero che corrisponde all'importo espresso in centesimi di euro
// o -1 in caso di errore
int leggi_importo(const char* str){
	int importo_euro = -1;
	char cifra_cent1 = '-';
	char cifra_cent2 = '-';
	int n_cifre = sscanf(str, "%d.%c%c", &importo_euro, &cifra_cent1, &cifra_cent2);

	if (n_cifre==0 || importo_euro<0){return -1;}

	int importo;
	switch(n_cifre){
		case 1:
			importo = importo_euro*100;
			break;
		case 2:
			if (cifra_cent1 < '0' || cifra_cent1 > '9'){return -1;}
			importo = importo_euro*100 + (cifra_cent1 -'0')*10;
			break;
		case 3:
			if (cifra_cent1 < '0' || cifra_cent1 > '9'){return -1;}
			if (cifra_cent2 < '0' || cifra_cent2 > '9'){return -1;}
			importo = importo_euro*100 + (cifra_cent1 - '0')*10 + (cifra_cent2 - '0');
			break;
	}

	return importo;
}

// NOT_SIGNED, NO_COMANDO_FOUND, CREDENTIAL_TOO_LONG, NO_ERROR, USER_ALREADY_TAKEN
void mostra_errore(enum ERROR err){

	const char *msg = "\0";

	switch(err){
		case NO_ERROR:
		msg = "";
		break;
		case NOT_LOGGED_IN:
		msg = "Utente non ha effettuato il login.\n";
		break;
		case NO_COMANDO_FOUND:
		msg = "Comando non esistente (invia !help per visualizzare i comandi presenti)\n";
		break;
		case WRONG_CREDENTIALS_SIZE:
		msg = "Il nome o/e la password sono troppi lunghi o troppo corti (max 50 caratteri min 5).\n";
		break;
		case USER_ALREADY_TAKEN:
		msg = "Nome utente gia' esistente\n";
		break;
		case WRONG_CREDENTIALS:
		msg = "Nome utente e/o password errati\n";
		break;
		case BAD_REQUEST:
		msg = "Error: Bad Request to Server\n";
		break;
		case BANNED:
		msg = "Raggiunto il numero massimo di tentativi, riprovare piu` tardi.\n";
		break;
		case WRONG_SESSID:
		msg = "Error: Wrong Session ID\n";
		break;
		case SYNTAX_ERROR:
		msg = "Errore di sintassi: comando inserito non correttamente.\n";
		break;
		case ALREADY_LOGGED:
		msg = "Utente gia` connesso.\n";
		break;
		case THREAD_LOGGED_IN:
		msg = "Utente specificato già connesso.\n";
		break;
		case DISCONNECTED:
		msg = "Problemi di connessione\n";
		break;
		case SERVER_ERROR:
		msg = "Ci dispiace, il server ha riscontrato un errore\n";
		break;
		default:
		msg = "Errore non riconosciuto\n";
		break;
	}

	printf("%s", msg);
	fflush(stdout);
}

// HELP, SIGNUP, LOGIN, INVIA_GIOCATA, VEDI_GIOCATE, VEDI_ESTRAZIONE, VEDI_VINCITE, ESCI
enum COMANDO str_command(char *_str){
	if (strcmp("!help", _str) == 0){
		return HELP;
	}

	if (strcmp("!signup", _str) == 0){
		return SIGNUP;
	}

	if (strcmp("!login", _str) == 0){
		return LOGIN;
	}

	if (strcmp("!invia_giocata", _str) == 0){
		return INVIA_GIOCATA;
	}

	if (strcmp("!vedi_giocate", _str) == 0){
		return VEDI_GIOCATE;
	}

	if (strcmp("!vedi_estrazioni", _str) == 0){
		return VEDI_ESTRAZIONI;
	}

	if (strcmp("!vedi_vincite", _str) == 0){
		return VEDI_VINCITE;
	}

	if (strcmp("!esci", _str) == 0){
		return ESCI;
	}

	return NO_COMANDO;
}

//BARI, CAGLIARI, FIRENZE, GENOVA, MILANO, NAPOLI, PALERMO, ROMA, TORINO, VENEZIA, NAZIONALE, TUTTE
enum RUOTA str_ruota(char *_str){

	if (strcmp("bari", _str) == 0){
		return BARI;
	}

	if (strcmp("cagliari", _str) == 0){
		return CAGLIARI;
	}

	if (strcmp("firenze", _str) == 0){
		return FIRENZE;
	}

	if (strcmp("genova", _str) == 0){
		return GENOVA;
	}
	if (strcmp("milano", _str) == 0){
		return MILANO;
	}
	if (strcmp("napoli", _str) == 0){
		return NAPOLI;
	}

	if (strcmp("palermo", _str) == 0){
		return PALERMO;
	}

	if (strcmp("roma", _str) == 0){
		return ROMA;
	}

	if (strcmp("torino", _str) == 0){
		return TORINO;
	}

	if (strcmp("venezia", _str) == 0){
		return VENEZIA;
	}

	if (strcmp("nazionale", _str) == 0){
		return NAZIONALE;
	}

	if (strcmp("tutte", _str) == 0){
		return TUTTE;
	}

	return NO_RUOTA;
}

//BARI, CAGLIARI, FIRENZE, GENOVA, MILANO, NAPOLI, PALERMO, ROMA, TORINO, VENEZIA, NAZIONALE, TUTTE
const char* ruota_str(enum RUOTA _ruota){

	switch (_ruota) {
		case BARI: return "Bari";
		case CAGLIARI: return "Cagliari";
		case FIRENZE: return "Firenze";
		case GENOVA: return "Genova";
		case MILANO: return "Milano";
		case NAPOLI: return "Napoli";
		case PALERMO: return "Palermo";
		case ROMA: return "Roma";
		case TORINO: return "Torino";
		case VENEZIA: return "Venezia";
		case NAZIONALE: return "Nazionale";
		default: return "";
	}

}
