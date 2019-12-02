#include "header.h"

#define INTREVALO 10
#define BROKER_PORT 2000
#define VALOR 4
#define HOME "localhost"


/*
    Inicia o sensor registando-se com o broker.
    Envia toda a informação necessária para recriar uma struct
    sensor no lado do broker.
*/
void iniciar_sensor(int sockfd,
    struct sensor* this_sens){

    //variáveis temporárias para enviar os valores do sensor
    int id = this_sens->id;
    char tipo[MAXCHAR];
    char local[MAXCHAR];
    char versao[MAXCHAR];
    memset(tipo, '\0', MAXCHAR);
    memset(local, '\0', MAXCHAR);
    memset(versao, '\0', MAXCHAR);

    strcpy(tipo, this_sens->tipo);
    strcpy(local, this_sens->local);
    strcpy(versao, this_sens->versao);

    //envia os dados para o broker
    send(sockfd, &id, sizeof(id), 0);
    send(sockfd, tipo, strlen(tipo), 0);
    send(sockfd, local, strlen(local), 0);
    send(sockfd, versao, strlen(versao), 0);

    bool accepeted = false;

    //recebe se foi registado no broker ou não
    read(sockfd, &accepeted, sizeof(accepeted));

    if(accepeted)
        printf("Registado com sucesso.\n");

    else{
        printf("Erro: Sensor com mesmo ID já registado.\n");
        exit(2);
    }
}

//passa a data atual para string (Retirado das aulas)
void strdate(char *buffer, int len){
	time_t now = time(NULL);
	struct tm *ptm = localtime(&now);
	
	if (ptm == NULL) {	
		puts("The localtime() function failed");
		return;
	}

	strftime(buffer, len, "%c", ptm);
}


void send_data(int sockfd, struct sensor* this_sens){
    //variáveis temporárias para enviar os valores da mensagem
    int id = this_sens->id;
    char data[MAXCHAR];
    int valor;
    char tipo[MAXCHAR];
    char versao[MAXCHAR];
    memset(data, '\0', MAXCHAR);
    memset(tipo, '\0', MAXCHAR);
    memset(versao, '\0', MAXCHAR);

    valor = rand() % 101; //valor random
    strcpy(tipo, this_sens->tipo);
    strcpy(versao, this_sens->versao);
    strdate(data, MAXCHAR);

    //envia os dados para o broker criar uma struct do tipo
    //sensor_payload
    send(sockfd, &id, sizeof(id), 0);
    send(sockfd, data, strlen(data), 0);
    send(sockfd, &valor, sizeof(valor), 0);
    send(sockfd, tipo, strlen(tipo), 0);
    send(sockfd, versao, strlen(versao), 0);
}


void update_firmare(int sockfd, struct sensor* this_sens){
    FILE* fp = NULL;
    char versao[MAXCHAR];
    char buffer[MAX_UPDATE_SIZE];
    char filename[MAXCHAR + 5];
    memset(versao, '\0', MAXCHAR);
    memset(buffer, '\0', MAX_UPDATE_SIZE);
    memset(filename, '\0', MAXCHAR + 5);

    while(true){
        read(sockfd, versao, MAXCHAR);
        read(sockfd, buffer, MAX_UPDATE_SIZE);

        if(strcmp(versao, this_sens->versao) > 1){
            strcpy(filename, versao);
            strcat(versao, ".upd");

            fp = fopen(filename, "w+");
            fprintf(fp, "%s", buffer);
            fclose(fp);
            strcpy(this_sens->versao, versao);
        }
    }
}

int main(int argc, char *argv[]){
    srand(time(NULL)); //usado para a função rand()

    int intervalo = INTREVALO;
    int portno = BROKER_PORT;

    //o sensor tem de ter um ID, tipo, local e versão
    if(argc < 5){
        printf("Too few arguments.\n");
        exit(1);
    }

    //cria o sensor
    struct sensor* this_sens = new_sensor(atoi(argv[1]), argv[2], argv[3], argv[4]);

    
    //sockfd -> socket
    int sockfd;
    struct hostent *server = gethostbyname(HOME); //host entety
    struct sockaddr_in serv_addr;   //server address


    if(argc >= 5)
        server = gethostbyname(argv[4]);

    if(argc >= 6)
        portno = atoi(argv[5]);
    
    if(argc == 7)
        intervalo = atoi(argv[6]);


//1º Passo!
    /* Create a socket point */
    //cria a socket... AF_INET = TCP
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }
	 
    //caso o host não seja válido
    if (server == NULL) { 
        fprintf(stderr,"ERROR, no such host\n");
        exit(1);
    }

    //informação sobre o endereço do broker
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);
   
//2º Passo!
    /* Now connect to the server */
    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR connecting");
        exit(2);
    }
   
    printf("Connected.\n");
    

    //inicia o sensor no broker
    iniciar_sensor(sockfd, this_sens);
    
    bool flag = true;
    pthread_t thread_id;

    pthread_create(&thread_id, NULL, update_firmare, NULL);

    while(flag){
        //usando threads para saber se envia a próxima mensagem
        send_data(sockfd, this_sens);
        sleep(intervalo);
    }


    pthread_join(thread_id, NULL);
    //Bye!
    printf("Disconnected.\n");
    close(sockfd);
    return 0;
}