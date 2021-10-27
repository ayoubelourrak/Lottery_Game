#ifndef TYPES
#define TYPES

#define MAX_MSG_LENGTH 10000000
#define N_RUOTE 11
#define N_SCALA 5
#define SESS_ID_SIZE 11

#include <stdint.h>

// enum per i possibili comandi utilizzabili dal client
enum COMANDO{HELP, SIGNUP, LOGIN, INVIA_GIOCATA, VEDI_GIOCATE, VEDI_ESTRAZIONI, VEDI_VINCITE, ESCI, NO_COMANDO};

// enum per le ruote possibili
enum RUOTA{BARI, CAGLIARI, FIRENZE, GENOVA, MILANO, NAPOLI, PALERMO, ROMA, TORINO, VENEZIA, NAZIONALE, TUTTE, NO_RUOTA};

// enum per gli errori possibili causati dal client o dal server
enum ERROR{SERVER_ERROR, BAD_REQUEST, NOT_LOGGED_IN, NO_COMANDO_FOUND, WRONG_CREDENTIALS_SIZE, NO_ERROR, USER_ALREADY_TAKEN, WRONG_CREDENTIALS, WRONG_SESSID, ALREADY_LOGGED, THREAD_LOGGED_IN, GENERIC_ERROR, SYNTAX_ERROR, BANNED, DISCONNECTED, MAX_SIZE_REACH};


#endif
