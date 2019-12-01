#include "header.h"

#define INTREVALO 10
#define PORT 2000
#define VALOR 4
#define HOME "localhost"


void iniciar_sensor(int sockfd,
    struct sensor* this_sens){
    
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

    send(sockfd, &id, sizeof(id), 0);
    send(sockfd, tipo, strlen(tipo), 0);
    send(sockfd, local, strlen(local), 0);
    send(sockfd, versao, strlen(versao), 0);

    bool accepeted = false;

    read(sockfd, &accepeted, sizeof(accepeted));

    if(accepeted)
        printf("Registado com sucesso.\n");

    else{
        printf("Erro: Sensor com mesmo ID já registado.\n");
        exit(2);
    }
}

//das aulas
void strdate(char *buffer, int len){
	time_t now = time(NULL);
	struct tm *ptm = localtime(&now);
	
	if (ptm == NULL) {	
		puts("The localtime() function failed");
		return;
	}

	strftime(buffer, len, "%c", ptm);
}


int get_new_value(){
    return rand() % 101;
}


void send_data(int sockfd, struct sensor* this_sens){
    int id = this_sens->id;
    char data[MAXCHAR];
    int valor;
    char tipo[MAXCHAR];
    char versao[MAXCHAR];
    memset(data, '\0', MAXCHAR);
    memset(tipo, '\0', MAXCHAR);
    memset(versao, '\0', MAXCHAR);

    valor = get_new_value();
    strcpy(tipo, this_sens->tipo);
    strcpy(versao, this_sens->versao);
    strdate(data, MAXCHAR);

    send(sockfd, &id, sizeof(id), 0);
    send(sockfd, data, strlen(data), 0);
    send(sockfd, &valor, sizeof(valor), 0);
    send(sockfd, tipo, strlen(tipo), 0);
    send(sockfd, versao, strlen(versao), 0);
}


int main(int argc, char *argv[]){
    srand(time(NULL));   // Initialization, should only be called once.

    int intervalo = INTREVALO;
    int portno = PORT;

    if(argc < 5){
        printf("Too few arguments.\n");
        exit(1);
    }

    struct sensor* this_sens = new_sensor(atoi(argv[1]), argv[2], argv[3], argv[4]);

    //retirado das aulas
    int sockfd;
    struct hostent *server = gethostbyname(HOME);
    struct sockaddr_in serv_addr;


    if(argc >= 5)
        server = gethostbyname(argv[4]);

    if(argc >= 6)
        portno = atoi(argv[5]);
    
    if(argc == 7)
        intervalo = atoi(argv[6]);



    /* Create a socket point */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }
	 
    if (server == NULL) { 
        fprintf(stderr,"ERROR, no such host\n");
        exit(1);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);
   
    /* Now connect to the server */
    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR connecting");
        exit(2);
    }
   
    printf("Connected.\n");
    

    iniciar_sensor(sockfd, this_sens);
    
    bool flag = true;

    while(flag){
        //novo update de firmare????
        send_data(sockfd, this_sens);
        sleep(intervalo);
    }

    printf("Disconnected.\n");
    close(sockfd);
    return 0;
}