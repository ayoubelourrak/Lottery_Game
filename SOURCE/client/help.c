#include "help.h"

//HELP, SIGNUP, LOGIN, INVIA_GIOCATA, VEDI_GIOCATE, VEDI_ESTRAZIONE, VEDI_VINCITE, ESCI, NO_COMANDO
void mostra_help(enum COMANDO com){
	const char *str, *help, *signup, *login, *invia_giocata, *vedi_giocate, *vedi_estrazione, *vedi_vincite, *esci, *norm;
    norm =           "************************** IL GIOCO DEL LOTTO **************************\n"
                      "I possibili comandi sono i seguenti:\n\n"
                      "1) !help <comando> : mostra i dettagli di uno specifico comando\n"
                      "2) !signup <username> <password> : registra un nuovo utente\n"
                      "3) !login <username> <password> : accedi ad un utente precedentemente registrato\n"
                      "4) !invia_giocata <giocata> : invia una giocata al server\n"
                      "5) !vedi_giocate <tipo> : mostra le giocate in base al tipo \n"
                      "6) !vedi_estrazioni <n> <ruota> : mostra le n precedenti estrazioni \ndi una specifica ruota\n"
					  "7) !vedi_vincite : mostra tutte le vincite dell'utente\n"
	                  "8) !esci : logout dall'utente\n";
    help =            "-!help <comando> : mostra i dettagli di un determinato <comando>\n";
    signup =          "-!signup <username> <password> : registra un nuovo utente\n"
                      " <username> <password> possono utilizzare tutti i carattere ascii tranne i caratteri speciali\n"
                      " e devono avere una lunghezza di almeno 5 caratteri e non più di 50\n";
    login =           "-!login <username> <password> :  accedi ad un utente precedentemente registrato.\n"
                      " RICORDA: dopo 4 tentativi errati bisogna aspettare 30 secondi per una nuova iscrizione";
    invia_giocata =   "-!invia_giocata <giocata> : invia una giocata al server la giocata deve essere nella\n"
                      "   nella forma -r <ruote> -n <numeri> -i <valori> le ruote sono quelle standard piu'\n"
                      "   l'opzione 'tutte' i numeri devono essere da 1 a 10 interi compresi tra 1 e 90 (i numeri che si vogliono\n"
                      "   giocare) e i valori sono un insieme di valori numerici con al piu' due cifre decimali\n"
                      "   che indicano la scommesso sul uscita rispettivamente di ESTRATTO, AMBO, TERNA, QUATERNA,\n"
                      "   CINQUINA, il numero di valori deve essere uguali a quello di numeri (mettere 0 se non si\n"
                      "   vuole scommetere)\n";
    vedi_giocate =    "-!vedi_giocate <tipo> : se <tipo> = 1 mostra le giocate attive ovvero quello di cui non si sa\n"
                      "   ancora il risultato se <tipo> = 0 mostra le giocate effettuate di cui si conosce il risultato\n";
    vedi_estrazione = "-!vedi_estrazioni <n> <routa> : mostra le n estrazioni su una specifica ruota (mettere tutte oppure\n"
                      "   ingorare il campo se si vogliono di tutte le ruote)\n";
    vedi_vincite =    "-!vedi_vincite : mostra tutte le vincite dell'utente\n";
    esci =            "-!esci :  logout dell'utente\n";

	switch(com){
		case HELP:
		str = help;
		break;
		case SIGNUP:
		str = signup;
		break;
		case LOGIN:
		str = login;
		break;
		case INVIA_GIOCATA:
		str = invia_giocata;
		break;
		case VEDI_GIOCATE:
		str = vedi_giocate;
		break;
		case VEDI_ESTRAZIONI:
		str = vedi_estrazione;
		break;
		case VEDI_VINCITE:
		str = vedi_vincite;
		break;
		case ESCI:
		str = esci;
		break;
		default:
		str = norm;
		break;
	}

	printf("%s", str);
}
