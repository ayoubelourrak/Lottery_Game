#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include "types.h"

// invia un messaggio testo _msg via socket _sid
enum ERROR invia_msg(int _sid, char* _msg);
// funzione per il server quando invia un messaggio di errore
enum ERROR invia_errore(int _sid, enum ERROR err);
// ricezione di un messaggio testo da _sid e ricopiamento in msg_
enum ERROR ric_msg(int _sid, char* msg_);
