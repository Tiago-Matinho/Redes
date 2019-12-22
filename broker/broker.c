#include "hashtable.h"

#define PORT 2000
#define TIMEOUT 10

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

    printf("a registar\n");
    //cria um novo sensor e um node da hashtable
    struct sensor* new_s = new_sensor(id, tipo, local, versao);
    struct sensor_node* new_n = new_node(new_s, new_socket);

    char accepeted = 't';

    hash_insert(hashtable, new_n);
    //insere na hashtable e envia um valor booleano se inserido com sucesso
    /*
    if(!hash_insert(hashtable, new_n)){
        accepeted = 'f';
        send(new_socket, &accepeted, sizeof(accepeted), 0);
    }

    else
        send(new_socket, &accepeted, sizeof(accepeted), 0);

    */
    //verificar
    sensor_node_print(hash_get(hashtable, id));
}


/*
    Recebe as mensagens enviadas pelo sensor usando TCP.
*/
void recieve_data(int new_socket, struct hashtable* hashtable){
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
    /*recv(new_socket, &id, sizeof(id), 0);
    recv(new_socket, data, MAX_SIZE, 0);
    recv(new_socket, &valor, sizeof(valor), 0);
    recv(new_socket, tipo, MAX_SIZE, 0);
    recv(new_socket, versao, MAX_SIZE, 0);
    */

    char buffer[1000];
    memset(buffer, '\0', 1000);

    recv(new_socket, buffer, 1000, 0);

    printf("%s\n", buffer);
    //cria uma nova struct do tipo sensor_payload com as novas mensagens
    //struct sensor_payload* payload = new_sen_payload(id, data, valor, tipo, versao);
    
    //adiciona ao array de 10 ultimas mensagens do sensor
    //add_payload(hashtable, payload);

    //verificar
    //sensor_node_print(hash_get(hashtable, id));
}


void send_update(int sockfd, char versao[MAXCHAR], char buffer[MAX_UPDATE_SIZE]){
    send(sockfd, versao, MAX_SIZE, 0);
    send(sockfd, buffer, MAX_UPDATE_SIZE, 0);
}

int main(int argc, char* argv[]){

    //hashtable que guarda a informação sobre os sensores
    struct hashtable* hashtable = new_hashtable();

    fd_set master;
    fd_set read_fds;
    int fd_max = 0;

    int sockt_listener;
    int new_client;
    struct sockaddr_in address;
    int address_len;

    int port = PORT;            //porta default do broker
    int opt = 1;


    struct timeval time;
    time.tv_sec = 10;
    time.tv_usec = 0;

    //muda a porta default (PORT)
    if(argc >= 2)
        port = atoi(argv[1]);
    

    //Informação do servidor
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    
    //listening socket
    sockt_listener = socket(AF_INET, SOCK_STREAM, 0);

    if (sockt_listener < 0) {
        perror("ERROR opening socket");
        exit(1);
    }


    //setsockopt
    if(setsockopt(sockt_listener, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {

        perror("setsockopt");
        exit(EXIT_FAILURE);
    }



    //bind
    if(bind(sockt_listener, (struct sockaddr *) &address, sizeof(address)) < 0){
        printf("Socket not bound.\n");
        exit(EXIT_FAILURE);
    
    } else {
        printf("Socket bound.\n");
    
    }

    //listen
    //Setting the server to listen the client.
    if(listen(sockt_listener, 10) < 0) {
        printf("Listen failed.\n");
        exit(EXIT_FAILURE);
    }


    FD_ZERO(&master); //makes sure master is clean.
    FD_ZERO(&read_fds);
    FD_SET(sockt_listener, &master); //adds sockt_listener to the master.

    fd_max = sockt_listener;

    for(;;){

        read_fds = master; //copies the fd that have something to read.

        //Checks for sockets that have info and adds to read_fds.
        if(select(fd_max + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("Select error.");
            exit(4);
        }

        for (int i = 3; i <= fd_max; i++){
            if(FD_ISSET(i, &read_fds)){
                if(i == sockt_listener){

                    new_client = accept(sockt_listener, (struct sockaddr *) &address, (socklen_t * )&address_len);


                    //Check if the new socket is valid.
                    if(new_client == -1) {
                        perror("Accept of the nre socket failed.");
                    
                    }
                    else{
                        
                        //Adding the new socket to mater.
                        FD_SET(new_client, &master);
                        //Updating the fd_max.
                        if (fd_max < new_client){
                            fd_max = new_client;
                        }
                        
                        printf("New client on socket: %d\n", new_client);
                        iniciar_sensor(new_client, hashtable);


                    }
                }

                else{
                    recieve_data(i, hashtable);
                    /*
                    if(time.tv_sec == 0 && time.tv_usec == 0){
                        time.tv_sec = TIMEOUT;
                    }
                    */
                }

            }
        }
    }

    //bye!
    printf("Disconnected.\n");
    close(sockt_listener);
	return 0; 
}
