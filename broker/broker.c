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
    struct sensor_node* new_n = new_node(new_s);

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

int main(int argc, char* argv[]){

    //hashtable que guarda a informação sobre os sensores
    struct hashtable* hashtable = new_hashtable();

    //server_fd -> socket usada para fazer o listening
    //new_socket -> porta onde se vai comunicar com o sensor
    int server_fd, new_socket; 
	struct sockaddr_in address; //endereço do broker
	int port = PORT;            //porta default do broker
	
	int opt = 1;      // for setsockopt() SO_REUSEADDR, below
	int addrlen = sizeof(address);

    //muda a porta default (PORT)
    if(argc == 2)
        port = atoi(argv[1]);
    

//1º Passo!
    // Creating socket file descriptor
    //cria a socket... AF_INET = TCP
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    { 
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    } 


//2º Passo!     
    // Forcefully attaching socket to port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
                                                  &opt, sizeof(opt))) 
    { 
        perror("setsockopt failed"); 
        exit(EXIT_FAILURE); 
    } 

    //informação sobre o endereço do broker
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons( port ); 


//3º Passo!
    // Bind the socket to the network address and port
    if (  bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0  ) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    }


//4º Passo!
    if (listen(server_fd, 20) < 0) 
    { 
        perror("listen failed"); 
        exit(EXIT_FAILURE); 
    }


//5º Passo!
    // Wait for a connection
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,  
                       (socklen_t*)&addrlen))<0) 
    { 
        perror("accept failed"); 
        exit(EXIT_FAILURE); 
    }
    
//TODO: usar select aqui para responder a todos os clientes!

    //new_socket = sensor ligado    
    printf("Client connected.\n");

    //incia esse sensor
    iniciar_sensor(new_socket, hashtable);

    bool flag = true;

    //loop infinito
    while(flag){
        recieve_data(new_socket, hashtable);
    }


    //bye!
    printf("Disconnected.\n");
    close(server_fd);
	return 0; 
}