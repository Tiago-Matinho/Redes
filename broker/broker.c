#include "hashtable.h"

#define PORT 2000
#define TIMEOUT 1

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
    int socket_server, 
        socket_client,
        address_len = sizeof(address), 
        opt = 1,
        fd_max = 0;

    fd_set master, 
           read_fds;

    struct timeval time;
    time.tv_sec = TIMEOUT;
    time.tv_usec = 0;

    int port = PORT;            //porta default do broker

    char buffer[MAX_SIZE];

    //muda a porta default (PORT)
    if(argc >= 2)
        port = atoi(argv[1]);
    
    
    socket_server = socket(AF_INET, SOCK_STREAM, 0);

    //Test if the socket was successfully created.
    if (socket_server < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    //Make sure that socket doesnt reserve the port.
    if(setsockopt(socket_server, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {

        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    //Informação do servidor
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if(bind(socket_server, (struct sockaddr *) &address, sizeof(address)) < 0) {
        printf("Socket not bound.\n");
        exit(EXIT_FAILURE);
    
    } else {
        printf("Socket bound.\n");
    
    }

    //Setting the server to listen the client.
    if(listen(socket_server, 3) < 0) {
        printf("Listen failed.\n");
        exit(EXIT_FAILURE);
    }

    FD_ZERO(&master); //makes sure master is clean.
    FD_SET(socket_server, &master); //adds socket_server to the master.

    fd_max = socket_server;

    for(;;) {

        read_fds = master; //copies the fd that have something to read.

        //Checks for sockets that have info and adds to read_fds.
        if(select(fd_max + 1, &read_fds, NULL, NULL, &time) == -1) {
            perror("Select error.");
            exit(4);
        }

        if(time.tv_sec == 0 && time.tv_usec == 0) {
            
            for (int i = 4; i <= fd_max; i++){

                recieve_data(i, hashtable);
            }

            time.tv_sec = TIMEOUT;
        }

        for (int i = 0; i <= fd_max; i++) {

            if(FD_ISSET(i, &read_fds)) {
                printf("teste\n");
                if(i == socket_server) {

                    socket_client = accept(socket_server, (struct sockaddr *) &address, (socklen_t * )&address_len);

                    //Check if the new socket is valid.
                    if(socket_client == -1) {
                        perror("Accept of the nre socket failed.");
                    
                    } else {

                        //Adding the new socket to mater.
                        FD_SET(socket_client, &master);
                        //printf("novo cliente\n");

                        //Updating the fd_max.
                        if (fd_max < socket_client) {
                            fd_max = socket_client;
                        }

                    }

                } else {

                    FD_CLR(i, &master);
                    close(i);
                }
            }
        }
    }

    //bye!
    printf("Disconnected.\n");
    close(socket_server);
	return 0; 
}