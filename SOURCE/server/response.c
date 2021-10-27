#include "response.h"

// appende al file IP_FILE la seguente linea:
// <IP da bannare> TIME=[<valore numerico timestamp>] <timestamp in formato %d-%m-%Y %H:%M:%S>
void block_ip(struct in_addr ip){

    char* ip_str = inet_ntoa(ip);
    char block_time_stamp[50];
    time_t now = time(NULL);
    strftime(block_time_stamp, sizeof(block_time_stamp), "%d-%m-%Y %H:%M:%S", localtime(&now));

    FILE *file;
    file = fopen(IP_FILE,"a");
    fprintf(file, "%s TIME=[%jd] %s\n", ip_str, now, block_time_stamp);
    fclose(file);
}

// ritorna 1 se correntemente bloccato, 0 altrimenti
// cerca l'eventuale ban dell'ip in IP_FILE e controlla se risulta ancora bloccato
int is_blocked(struct in_addr ip){
    FILE *file;
    file = fopen(IP_FILE,"r");
    char entry[500];
    char ip_str[INET_ADDRSTRLEN];
    time_t block_time = 0;
    int is_blocked_ = 0;


    while(fgets(entry,sizeof(entry),file) != NULL){ // per ogni linea contenuta nel file

        if (entry[0] == '\n' || entry[0] == '#') continue; // linea vuota o di commento
        sscanf(entry, "%s TIME=[%jd]", ip_str, &block_time);
        struct in_addr ip_i;

        inet_pton(AF_INET, ip_str, &ip_i);
        if (ip_i.s_addr != ip.s_addr) continue;
        if (difftime(time(NULL), block_time) < BLOCK_TIME)
            is_blocked_ = 1;
    }

    fclose(file);
    return is_blocked_;
}

// gestisce una richiesta di login
enum ERROR login(thread_slot* thread_data, char* req_punt, char* res_punt){

    if (strcmp(thread_data->user, "") != 0) return ALREADY_LOGGED;

    if (is_blocked(thread_data->ip)) return BANNED;

    // legge le credenziali nella richiesta
    char user[51] = "";
    char password[51] = "";
    sscanf(req_punt, "USER: %s\nPASSWORD: %s", user, password);

    char usr_dir[MAX_PATH_LEN];
    sprintf(usr_dir, "%s/%s", USERS_DIR, user);

    // controllo se esiste una directory a nome dell'utente
    struct stat st = {0};
    if (stat(usr_dir, &st) == -1){
        // autenticazione fallita
        thread_data->n_try ++;
        if (thread_data->n_try > 2){
            block_ip(thread_data->ip);
            return BANNED;
        }
        else
            return WRONG_CREDENTIALS;
    }

    // cerco il file password.txt associato all'utente
    char usr_filepath[MAX_PATH_LEN];
    sprintf(usr_filepath, "%s/password.txt", usr_dir);
    FILE *usr_file;
    usr_file = fopen(usr_filepath,"r");

    // se non esiste
    if (usr_file == NULL){
        printf("Could not open file: %s\n", usr_filepath);
        printf("%s\n", strerror(errno));
        return SERVER_ERROR;
    }

    // leggo la password contenuta in password.txt
    char saved_password[51] = "";
    fscanf(usr_file,"%s",saved_password);
    fclose(usr_file);

    // verifico la password usata usata per il login
    if (strcmp(password, saved_password) != 0){
        // autenticazione fallita
        thread_data->n_try ++;
        if (thread_data->n_try > 2){
            block_ip(thread_data->ip);
            return BANNED;
        }
        else
            return WRONG_CREDENTIALS;
    }
    
    // segno l'utente loggato
    strcpy(thread_data->user,user);
    // genero la session ID
    const char* alphanum = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    for(int i=0; i < SESS_ID_SIZE-1; i++)
        thread_data->session_id[i] = alphanum[rand() % (sizeof(alphanum)-1)];

    sprintf(res_punt, "SESSION_ID: %s\n", thread_data->session_id);

    return NO_ERROR;
}

// gestisce una richiesta di signup
enum ERROR signup(thread_slot* thread_data, char* req_punt, char* res_punt){
    if (strcmp(thread_data->user, "") != 0)
        return ALREADY_LOGGED;
    char user[51] = "";
    char password[51] = "";
    // leggo le credenziali nella richiesta
    sscanf(req_punt, "USER: %s\nPASSWORD: %s", user, password);

    char usr_dir[MAX_PATH_LEN];
    sprintf(usr_dir, "%s/%s", USERS_DIR, user);
    // controllo se esiste gia' una directory a nome dell'utente
    struct stat st = {0};
    if (stat(usr_dir, &st) != -1) return USER_ALREADY_TAKEN;

    // creo la directory dell'utente
    if (mkdir(usr_dir, 0700) == -1){
        printf("Could not create directory: %s\n", usr_dir);
        printf("%s\n", strerror(errno));
        return SERVER_ERROR;
    }

    // creo i file relativi all'utente
    char new_filepath[MAX_PATH_LEN];
    FILE *new_file;

    // file contenente la password
    sprintf(new_filepath, "%s/password.txt", usr_dir);
    new_file = fopen(new_filepath,"w");
    if (new_file == NULL){
        printf("Could not create file: %s\n", new_filepath);
        printf("%s\n", strerror(errno));
        return SERVER_ERROR;
    }
    fprintf(new_file,"%s",password);
    fclose(new_file);

    // file delle scommesse attive
    sprintf(new_filepath, "%s/active_bets.txt", usr_dir);
    new_file = fopen(new_filepath,"w");
    if (new_file == NULL){
        printf("Could not create file: %s\n", new_filepath);
        printf("%s\n", strerror(errno));
        return SERVER_ERROR;
    }
    fclose(new_file);

    // file delle scommesse passate
    sprintf(new_filepath, "%s/old_bets.txt", usr_dir);
    new_file = fopen(new_filepath,"w");
    if (new_file == NULL){
        printf("Could not create file: %s\n", new_filepath);
        printf("%s\n", strerror(errno));
        return SERVER_ERROR;
    }
    fclose(new_file);

    // file consuntivo (contiene le vincite comulative per ogni scala, in centesimi)
    sprintf(new_filepath, "%s/consuntivo.txt", usr_dir);
    new_file = fopen(new_filepath,"w");
    if (new_file == NULL){
        printf("Could not create file: %s\n", new_filepath);
        printf("%s\n", strerror(errno));
        return SERVER_ERROR;
    }
    fprintf(new_file,"0 0 0 0 0"); // scrivo i 5 valori iniziali
    fclose(new_file);

    return NO_ERROR;
}

// gestisce l'arrivo di una giocata
enum ERROR invia_giocata(thread_slot* thread_data, char* req_punt, char* res_punt){
    if (strcmp(thread_data->user, "") == 0)
        return NOT_LOGGED_IN;
    // scrivo la scommessa nel file
    char filepath[MAX_PATH_LEN];
    sprintf(filepath, "%s/%s/active_bets.txt", USERS_DIR, thread_data->user);
    FILE *file;
    file = fopen(filepath,"a");
    fprintf(file,"%s",req_punt);
    fclose(file);
    return NO_ERROR;
}

// copia il file [filename] appartenente all'user [user] nel buffer di risposta puntato da [res_punt_p]
// il puntatore del buffer di risposta viene incrementato dall'operazione
enum ERROR print_user_file(const char* _filename,const char* _user, char** res_punt_p){
    char filepath[MAX_PATH_LEN];
    FILE *file;
    sprintf(filepath, "%s/%s/%s", USERS_DIR, _user, _filename);
    file = fopen(filepath,"r");
    if (file == NULL){
        printf("Could not open file: %s\n", filepath);
        printf("%s\n", strerror(errno));
        return SERVER_ERROR;
    }
    // copio il file nella risposta
    char c;
    while ( ( c = fgetc(file) ) != EOF)
    {
        (*res_punt_p)[0] = c;
        (*res_punt_p) ++;
    }
    fclose(file);
    return NO_ERROR;
}

// gestisce una richiesta di "vedi_giocate"
enum ERROR vedi_giocate(thread_slot* thread_data, char* req_punt, char* res_punt){
    if (strcmp(thread_data->user, "") == 0)
        return NOT_LOGGED_IN;
    char tipo;
    // leggo il tipo richiesto
    sscanf(req_punt, "TIPO: %c\n", &tipo);

    // copio il file richiesto nel messaggio di risposta
    enum ERROR err = NO_ERROR;
    if (tipo == '0')
        err = print_user_file("old_bets.txt", thread_data->user, &res_punt);
    else
        err = print_user_file("active_bets.txt", thread_data->user, &res_punt);

    return err;
}

// gestisce una richiesta di "vedi_estrazioni"
enum ERROR vedi_estrazioni(thread_slot* thread_data, char* req_punt, char* res_punt){
    if (strcmp(thread_data->user, "") == 0)
        return NOT_LOGGED_IN;

    int n_estr;
    enum RUOTA ruota;
    // leggi numero di estrazioni e ruota indicata nella richiesta
    if (sscanf(req_punt, "N_ESTRAZIONI: %d\nRUOTA: %d\n", &n_estr, (int*)&ruota) < 2)
        return BAD_REQUEST;

    if (n_estr <= 0) return NO_ERROR;

    // controllo se il file delle estrazioni esiste
    if ( access( ESTRAZ_FILE, F_OK ) == -1){
        sprintf(res_punt, "Non ci sono estrazioni in memoria.\n");
        return NO_ERROR;
    }
    // apro il file delle estrazioni
    FILE *file;
    file = fopen(ESTRAZ_FILE,"r");
    fseek(file, 0, SEEK_END);
    size_t length = ftell(file); //lunghezza del file
    int num_estrazioni_totali = length / 776; // ogni estrazione occupa 776 caratteri nel file
    if (n_estr > num_estrazioni_totali){
        sprintf(res_punt, "Numero indicato troppo grande. In memoria sono presenti %d estrazioni.\n", num_estrazioni_totali);
        fclose(file);
        return NO_ERROR;
    }

    size_t chars_to_read = n_estr*776;
    fseek(file, -chars_to_read, SEEK_END); // mi preparo a leggere le ultime n_estr estrazioni

    if (ruota == TUTTE){
        // copia tutto
        char c;
        while ( ( c = fgetc(file) ) != EOF)
        {
            res_punt[0] = c;
            res_punt ++;
        }
        res_punt[0] = '\0';
    }
    else{
        // copia solo le righe della ruota indicata nel comando
        // la prima riga dell'estrazione contiene 38 caratteri + \n
        // seguono N_RUOTE righe di  66 caratteri + \n ciascuna
        fseek(file, 39 + 67*(int)ruota, SEEK_CUR); // vado alla prima riga di interesse
        while (fgets(res_punt, 67 + 1, file)){
            res_punt += 67;
            fseek(file, 776-67,SEEK_CUR);
        }
    }
    return NO_ERROR;

}

// gestice una richiesta di "vedi_vincite"
enum ERROR vedi_vincite(thread_slot* thread_data, char* req_punt, char* res_punt){
    if (strcmp(thread_data->user, "") == 0)
        return NOT_LOGGED_IN;
    // copio il file vincite.txt nel messaggio di risposta
    enum ERROR err = NO_ERROR;
    err = print_user_file("vincite.txt", thread_data->user, &res_punt);
    if (err != NO_ERROR) return err;
    // stampo il consuntivo
    char tmp[100];
    char* tmp_punt = &tmp[0];
    // leggo il file consuntivo dell'user
    err = print_user_file("consuntivo.txt", thread_data->user, &tmp_punt);
    if (err != NO_ERROR) return err;

    long int guadagni[N_SCALA]; // in centesimi
    sscanf(tmp, "%ld %ld %ld %ld %ld", &guadagni[0], &guadagni[1], &guadagni[2], &guadagni[3], &guadagni[4]);
    const char* scala[N_SCALA];
    scala[0] = "ESTRATTO";
    scala[1] = "AMBO";
    scala[2] = "TERNO";
    scala[3] = "QUATERNA";
    scala[4] = "CINQUINA";

    sprintf(res_punt, "\n");
    res_punt = prox_riga(res_punt);
    for (int i=0; i<N_SCALA; i++){
        sprintf(res_punt, "Vincite su %s: %.2f\n", scala[i], guadagni[i] / 100.0);
        res_punt = prox_riga(res_punt);
    }

    return NO_ERROR;
}

// gestisce una richiesta di "esci"
enum ERROR esci(thread_slot* thread_data, char* req_punt, char* res_punt){
    sprintf(res_punt,"Server: session end.\n");
    return NO_ERROR;
}

// gestisce una richiesta da parte del client
enum ERROR response(thread_slot* thread_data, enum COMANDO command, char* msg_punt, char*res_punt){
    enum ERROR e_;
    switch(command){
        case LOGIN:
            e_ = login(thread_data, msg_punt, res_punt);
            break;
        case SIGNUP:
            e_ = signup(thread_data, msg_punt, res_punt);
            break;
        case INVIA_GIOCATA:
            e_ = invia_giocata(thread_data, msg_punt, res_punt);
            break;
        case VEDI_GIOCATE:
            e_ = vedi_giocate(thread_data, msg_punt, res_punt);
            break;
        case VEDI_ESTRAZIONI:
            e_ = vedi_estrazioni(thread_data, msg_punt, res_punt);
            break;
        case VEDI_VINCITE:
            e_ = vedi_vincite(thread_data, msg_punt, res_punt);
            break;
        case ESCI:
            e_ = esci(thread_data, msg_punt, res_punt);
            break;
        default:
            e_ = NO_COMANDO_FOUND;
            break; //riprova a leggere l'input
    }

    return e_;
}
