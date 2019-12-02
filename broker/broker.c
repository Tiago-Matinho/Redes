#include "hashtable.h"

#define PORT 2000

/*
    Inicia o sensor registando-o na hashtable do broker.
    Recebe toda a informação necessária para criar uma struct
    sensor e adiciona à hashtable.
*/
void iniciar_sensor(int new_socket, struct hashtable* hashtable){
    //variáveis onde vai guardar os valores recebidos
    int id;
    char tipo[MAXCHAR];
    char local[MAXCHAR];
    char versao[MAXCHAR];
    memset(tipo, '\0', MAXCHAR);
    memset(local, '\0', MAXCHAR);
    memset(versao, '\0', MAXCHAR);

    //lê o que foi enviado pelo sensor para o new_socket
    read(new_socket, &id, sizeof(id));
    read(new_socket, tipo, MAXCHAR);
    read(new_socket, local, MAXCHAR);
    read(new_socket, versao, MAXCHAR);

    //cria um novo sensor e um node da hashtable
    struct sensor* new_s = new_sensor(id, tipo, local, versao);
    struct sensor_node* new_n = new_node(new_s, new_socket);

    bool accepeted = true;

    //insere na hashtable e envia um valor booleano se inserido com sucesso
    if(!hash_insert(hashtable, new_n)){
        accepeted = false;
        send(new_socket, &accepeted, sizeof(accepeted), 0);
    }

    else
        send(new_socket, &accepeted, sizeof(accepeted), 0);
}


/*
    Recebe as mensagens enviadas pelo sensor usando TCP.
*/
void recieve_data(int new_socket,struct hashtable* hashtable){
    //variáveis onde vai guardar os valores recebidos
    int id;
    char data[MAXCHAR];
    int valor;
    char tipo[MAXCHAR];
    char versao[MAXCHAR];
    memset(data, '\0', MAXCHAR);
    memset(tipo, '\0', MAXCHAR);
    memset(versao, '\0', MAXCHAR);

    //lê o que foi enviado pelo sensor para o new_socket
    read(new_socket, &id, sizeof(id));
    read(new_socket, data, MAX_SIZE);
    read(new_socket, &valor, sizeof(valor));
    read(new_socket, tipo, MAX_SIZE);
    read(new_socket, versao, MAX_SIZE);
    
    //cria uma nova struct do tipo sensor_payload com as novas mensagens
    struct sensor_payload* payload = new_sen_payload(id, data, valor, tipo, versao);
    
    //adiciona ao array de 10 ultimas mensagens do sensor
    add_payload(hashtable, payload);

    //verificar
    sensor_node_print(hash_get(hashtable, id));
}


void send_update(int sockfd, char versao[MAXCHAR], char buffer[MAX_UPDATE_SIZE]){
    send(sockfd, versao, MAX_SIZE, 0);
    send(sockfd, buffer, MAX_UPDATE_SIZE, 0);
}

int main(int argc, char* argv[]){

    //hashtable que guarda a informação sobre os sensores
    struct hashtable* hashtable = new_hashtable();

    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int listening_socket, new_socket, fd_max, opt = 1;
    int port = PORT;            //porta default do broker

    fd_set master, read_fds;

    //muda a porta default (PORT)
    if(argc >= 2)
        port = atoi(argv[1]);
    

//1º Passo!

	listening_socket = socket(AF_INET, SOCK_STREAM, 0);

    if(listening_socket < 0) {
        printf("ERROR, socket creation failed.\n Aborting.\n");
        exit(0);
    
    } else {
        printf("Socket successfully created.\n");
    }


//2º Passo!
    if(setsockopt(listening_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt failed"); 
        exit(EXIT_FAILURE); 
    }


    //informação sobre o endereço do broker
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons(port); 


//3º Passo!
    // Bind the socket to the network address and port
    if(bind(listening_socket, (struct sockaddr *)&address, sizeof(address)) < 0){ 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    }


//4º Passo!
    if(listen(listening_socket, 3) < 0){
        perror("listen failed"); 
        exit(EXIT_FAILURE); 
    }


//5º Passo!
    FD_ZERO(&master);
    FD_SET(listening_socket, &master);

    fd_max = listening_socket;
    bool flag = true;

    while(flag){
        read_fds = master;

        if(select(fd_max + 1, &read_fds, NULL, NULL, NULL) == -1){
            perror("Select error.");
            exit(4);
        }
        //lets go through all the fds.
        for (int i = 0; i <= fd_max; i++) {
            printf("teste\n");

            //checks if the file descriptor i is in read_fds (meaning that it has info).
            if (FD_ISSET(i, &read_fds)){

                //new conenction incoming.
                if(i == listening_socket) {

                    //accepts the new client.
                    if((new_socket = accept(new_socket, (struct sockaddr *) &address,
                    (socklen_t *) &addrlen)) == -1){
                        perror("Accept failed.\n");
                    }
                    
                    else {
                        //updating master by adding the new socket.
                        FD_SET(new_socket, &master);
                        printf("Client %d connected.\n", new_socket);

                        iniciar_sensor(new_socket, hashtable);
                        //updating max fd.
                        fd_max = (fd_max < new_socket ? new_socket : fd_max);
                    } 
                }
                else {
                    /*
                    //lê e guarda o output dado pelo servidor
                    if(read(i, msg, BUFFERSIZE) <= 0) {
                        printf("%d has disconected.\n", i);
                        FD_CLR(i, &master);
                        break;
                    }
                    */
                    recieve_data(i, hashtable);
                }

            }
        }
    }

    //bye!
    printf("Disconnected.\n");
    close(listening_socket);
	return 0; 
}