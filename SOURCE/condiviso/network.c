#include "network.h"

enum ERROR invia_msg(int _sid, char* _msg){

	uint16_t msg_len = strlen(_msg) + 1;
	uint16_t buf_size = htons(msg_len);
	ssize_t ris;

	//invia la lunghezza della stringa che si vuole ricevere
	ris = send(_sid, (void*)&buf_size, sizeof(uint16_t), 0);
	if(ris == -1) return DISCONNECTED;

	//invia la stringa
	ris = send(_sid, (void*)_msg, msg_len, 0);
	if(ris == -1) return DISCONNECTED;

	return NO_ERROR;
}

/* funzione per il server quando invia un messaggio di errore
"
SERVER RESPONSE\n
ERROR: [errore in forma di numero (enum)]
"
*/
enum ERROR invia_errore(int _sid, enum ERROR err){
	
	char buf[100];
	sprintf(buf, "SERVER RESPONSE\nERROR: %d\n", (int)err);
	return invia_msg(_sid, buf);
}

// ricezione di un messaggio testo da _sid e ricopiamento in msg_
enum ERROR ric_msg(int _sid, char* msg_){
	ssize_t ris;
	uint16_t buf_size;
	uint16_t msg_len;

	//ricezione della lunghezza della stringa che si vuole ricevere
	ris = recv(_sid, (void*)&buf_size, sizeof(uint16_t), 0);
	if(ris == -1 || ris == 0) return DISCONNECTED;

	//ricezione della stringa
	msg_len = ntohs(buf_size);
	ris = recv(_sid, (void*)msg_, msg_len, 0);
	if(ris == -1 || ris == 0) return DISCONNECTED;

	return NO_ERROR;
}
