#include "estrazioni.h"

// legge la prima giocata dal filestream giocate_f
// restituisce 0 se non riesce ad eseguire la lettura
// i valori vengono restituiti nei 3 array
// r_giocate e' una stringa di N_RUOTE caratteri
// r_giocate[ruota_i] vale 'X' se ruota_i e' stata giocata, '-' altrimenti
// numeri_ viene riempto con 0 a destra se i numeri giocati sono meno di 10
// importi_ viene riempto con 0 a destra se gli importi giocati sono meno di N_SCALA
int scommesse(FILE* giocate_f, char r_giocate_[N_RUOTE+1], int numeri_[10], int importi_[N_SCALA]){
    const char* formato;
    formato = "RUOTE_GIOCATE: %s\nNUMERI_GIOCATI:%[^\n]\nIMPORTI:%[^\n]\n";
    char numeri_str[50];
    char importi_str[100];
    int res = fscanf(giocate_f, formato, r_giocate_, numeri_str, importi_str);

    if (res < 3) return 0;

    int i;
    // inizializzo a 0 i numeri giocati
    for (i=0; i<10; i++) numeri_[i] = 0;
    // estrapolo i numeri giocati
    int num_chars = 0;
    char* n_str_punt = &numeri_str[0];
    i = 0;
    while (sscanf(n_str_punt, "%d%n", &numeri_[i], &num_chars) == 1){
        n_str_punt += num_chars;
        i++;
    }

    // inizializzo a 0 i numeri giocati
    for (i=0; i<5; i++) importi_[i] = 0;
    // estrapolo gli importi giocati
    num_chars = 0;
    char* imp_str_punt = &importi_str[0];
    i = 0;
    while (sscanf(imp_str_punt, "%d%n", &importi_[i], &num_chars) == 1){
        imp_str_punt += num_chars;
        i++;
    }

    return 1;
}

// appende il contenuto del filestream file_scom al file old_bets.txt dell'user
void archivia_scommesse(FILE* file_scom, char* user){

    // apro il file old_bets
    char filepath[MAX_PATH_LEN];
    sprintf(filepath, "%s/%s/old_bets.txt", USERS_DIR, user);
    FILE* file_old_scom = fopen(filepath,"a");

    // copio file_scom in file_old_scom
    fseek(file_scom, 0, SEEK_SET);
    char c;
    while( ( c = fgetc(file_scom) ) != EOF )
       fputc(c, file_old_scom);

    fclose(file_old_scom);
}

// calcola le combinazioni di k elementi su N
// C(N, k) = N! / (N - k)!
int combinazioni(int N, int k){

	int i, res = 1;
	for(i = N - k + 1; i <= N; i++){
		res *= i;
    }

	return res;
}

// aggiorna il file consuntivo dell'user
// sommando il vettore delle vincite a quello presente nel file
// i guadagni sono salvati in centesimi (dunque interi)
void aggiorna_consuntivo(double vincite[N_SCALA], char* user){

    long int guadagni[N_SCALA]; // in centesimi

    // leggo il file consuntivo.txt dell'user
    char filepath[MAX_PATH_LEN];
    sprintf(filepath, "%s/%s/consuntivo.txt", USERS_DIR, user);
    FILE* file = fopen(filepath,"r+");
    fscanf(file,"%ld %ld %ld %ld %ld", &guadagni[0], &guadagni[1], &guadagni[2], &guadagni[3], &guadagni[4]);

    // sommo le vincite appena fatte a quelle lette
    for (int i=0; i<N_SCALA; i++) guadagni[i] += (int) (vincite[i]*100);

    // salvo nel file
    fseek(file, 0, SEEK_SET);
    fprintf(file,"%ld %ld %ld %ld %ld", guadagni[0], guadagni[1], guadagni[2], guadagni[3], guadagni[4]);
    fclose(file);
}

// applico l'estrazione alle scommesse attive degli utenti
// numeri_estratti -> numeri dell'estrazione
// time_stamp -> stringa contenente il timestamp dell'estrazione
void estraz_effetto(int numeri_estratti[N_RUOTE][5], const char* _time_stamp){

    // apro la cartella USERS_DIR
    DIR * users_dir = opendir(USERS_DIR);
    struct dirent *u_dir;

    // per ogni cartella trovata, applico l'estrazione al relativo utente:
    while( (u_dir=readdir(users_dir)) )
    {
        if (strcmp(u_dir->d_name, ".") == 0 || strcmp(u_dir->d_name, "..") == 0)
            continue; // salta questi due directory
        char user[50];
        strcpy(user, u_dir->d_name);
        char filepath[MAX_PATH_LEN];

        int vincita = 0; // 1 se in questa estrazione l'utente ha fatto una vincita
        double vincite[N_SCALA]; // soldi vinti sul rispettivo tipo di scala
        for (int i=0; i<N_SCALA; i++) vincite[i] = 0;

        // apro il file delle scommesse
        sprintf(filepath, "%s/%s/active_bets.txt", USERS_DIR, user);
        FILE *scommesse_f;
        scommesse_f = fopen(filepath,"r");

        // apro il file delle vincite
        sprintf(filepath, "%s/%s/vincite.txt", USERS_DIR, user);
        FILE *vincite_f;
        vincite_f = fopen(filepath,"a");

        if (scommesse_f == NULL || vincite_f == NULL){
            printf("Could not open files for user: %s\n", user);
            printf("%s\n", strerror(errno));
            continue;
        }

        // leggo ed applico ogni giocata contenuta nel file
        char r_giocate[N_RUOTE+1];
        int n_giocati[10];
        int importi[N_SCALA];
        while(scommesse(scommesse_f, r_giocate, n_giocati, importi)){
            int indovinati_counter = 0; // numero dei numeri indovinati
            // per ogni numero giocato
            for (int i=0; i<10; i++){
                int trovato = 0;
                // per ogni ruota giocata
                for (int j=0; j<N_RUOTE; j++){
                    if (r_giocate[j] == '-') continue; // ruota non giocata
                    // cerco il numero giocato tra i 5 di questa ruota
                    for (int z=0; z<5; z++){
                        if(n_giocati[i] == numeri_estratti[j][z]){
                            indovinati_counter ++; // numero indovinato!
                            trovato = 1;
                            break;
                        }
                        if (trovato) break;
                    }
                    if (trovato) break;
                }
            }

            // controllo se ho vinto su almeno una scala
            // faccio avanzare i all'indice del primo importo non nullo
            int i = 0;
            for (i=0;i<N_SCALA && importi[i] == 0; i++);
            if (indovinati_counter <= i) continue; // ho attivato solo scala inferiori su cui non ho puntato

            if (vincita == 0){ // prima giocata dell'estrazione a causare una vincita
                fprintf(vincite_f, "Estrazione del %s\n", _time_stamp);
                vincita = 1; // setto a 1 vincita
            }

            // STAMPO LA GIOCATA
            // stampa (e conta) le ruote
            int r_giocate_counter = 0;
            if (strcmp(r_giocate, "XXXXXXXXXXX") == 0){
                fprintf(vincite_f," tutte");
                r_giocate_counter = N_RUOTE;
            }
            else{
                for (int i=0; i<N_RUOTE; i++){
                    if (r_giocate[i] == '-') continue;
                    enum RUOTA r = (enum RUOTA ) i;
                    fprintf(vincite_f," %s", ruota_str(r));
                    r_giocate_counter ++;
                }
            }

            fprintf(vincite_f, "   ");

            // stampo (e conto) i numeri giocati
            int n_giocati_counter = 0;
            for (int i=0; i<10 && n_giocati[i] != 0; i++){
                fprintf(vincite_f, " %d", n_giocati[i]);
                n_giocati_counter ++;
            }

            fprintf(vincite_f, " >> ");
            // CALCOLO LE VINCITE
            double valore[N_SCALA]; // valore della scala
            valore[0] = 11.23;
            valore[1] = 250;
            valore[2] = 4500;
            valore[3] = 120000;
            valore[4] = 6000000;
            const char* scala[N_SCALA];
            scala[0] = "Estratto";
            scala[1] = "Ambo";
            scala[2] = "Terno";
            scala[3] = "Quaterna";
            scala[4] = "Cinquina";

            for (int i=N_SCALA-1; i>=0; i--){
                if (importi[i] == 0) continue;
                fprintf(vincite_f, " %s", scala[i]);
                if (indovinati_counter < (i+1) ){
                    // non ho attivato la scala di ordine i
                    fprintf(vincite_f, " 0");
                    continue;
                }

                int comb_possibili = combinazioni(n_giocati_counter, i+1);
                int comb_realizzate = combinazioni(indovinati_counter, i+1);
                double guadagno = (double) importi[i] / 100.0; // gli importi sono in centesimi
                guadagno *= valore[i];
                guadagno *= comb_realizzate / (double) comb_possibili;
                guadagno /= (double) r_giocate_counter;

                fprintf(vincite_f, " %.2f", guadagno);
                vincite[i] += guadagno;
                break; // non importano le scala di ordine inferiore
            }

            fprintf(vincite_f, "\n");
        }
        if (vincita == 1){ // l'user ha riscontrato almeno una vincita
            fprintf(vincite_f, "*******************************************\n");
            aggiorna_consuntivo(vincite, user);
        }

        archivia_scommesse(scommesse_f, user);
        fclose(scommesse_f);
        // ri-apro il file delle scommesse per svuotarlo
        sprintf(filepath, "%s/%s/active_bets.txt", USERS_DIR, user);
        scommesse_f = fopen(filepath,"w");

        fclose(scommesse_f);
        fclose(vincite_f);
    }
    closedir(users_dir);
}

/*
Genera un'estrazione, gioca le scommesse attive degli utenti,
salva l'estrazione in ESTRAZ_FILE e aggiorna il file vincite.txt per ogni utente
*/

void estrazione(){

	// apro il file delle estrazioni
	FILE *estraz_f;
	estraz_f = fopen(ESTRAZ_FILE,"a"); // file in cui loggo le estrazioni

	int numeri_estratti[N_RUOTE][5]; // numeri estratti
	time_t now = time(NULL);
	char time_stamp[50];
    strftime(time_stamp, sizeof(time_stamp), "%d-%m-%Y ore %H:%M:%S", localtime(&now));
	fprintf(estraz_f, "Estrazione del %s\n", time_stamp);
	printf("Estrazione del %s\n", time_stamp);

	int numeri_possibili[90]; // array dei numeri che possono essere estratti
	for(int i = 0; i <  90; i++) numeri_possibili[i] = i + 1;

    // per ogni ruota estraggo 5 numeri
	for (int i=0; i<N_RUOTE; i++){
		int chars_written = 0;
		fprintf(estraz_f, " %-15s%n", ruota_str( (enum RUOTA) i), &chars_written); // stampo la ruota
		printf(" %-15s%n", ruota_str( (enum RUOTA) i), &chars_written);

		for (int j=0; j<5; j++){
			int index = rand() % (90 - j); // un index a caso per scegliere tra i 90-j numeri che posso ancora estrarre
			int estratto = numeri_possibili[index];
			numeri_estratti[i][j] = estratto;
			fprintf(estraz_f, "%-10d", estratto);
			printf("%-10d", estratto);

			// scambio il numero estratto con l'ultimo numero dell'array
			// in questo modo, all'interno di questo for non estratto gli stessi numeri
			numeri_possibili[index] = numeri_possibili[90 -1 -j];
			numeri_possibili[90 -1 -j] = estratto;
		}

        fprintf(estraz_f, "\n");
		printf("\n");
	}

	fclose(estraz_f);
	estraz_effetto(numeri_estratti, time_stamp);
}
