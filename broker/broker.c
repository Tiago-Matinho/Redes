#include "hashtable.h"

#define PORT 2000

void iniciar_sensor(int new_socket, struct hashtable* hashtable){
    int id;
    char tipo[MAXCHAR];
    char local[MAXCHAR];
    char versao[MAXCHAR];
    memset(tipo, '\0', MAXCHAR);
    memset(local, '\0', MAXCHAR);
    memset(versao, '\0', MAXCHAR);

    read(new_socket, &id, sizeof(id));
    read(new_socket, tipo, MAXCHAR);
    read(new_socket, local, MAXCHAR);
    read(new_socket, versao, MAXCHAR);

    struct sensor* new_s = new_sensor(id, tipo, local, versao);
    struct sensor_node* new_n = new_node(new_s);

    hash_insert(hashtable, new_n);

    //verificar
    sensor_node_print(hash_get(hashtable, id));
}

void recieve_data(int new_socket,struct hashtable* hashtable){
    int id;
    char data[MAXCHAR];
    int valor;
    char tipo[MAXCHAR];
    char versao[MAXCHAR];
    memset(data, '\0', MAXCHAR);
    memset(tipo, '\0', MAXCHAR);
    memset(versao, '\0', MAXCHAR);

    read(new_socket, id, sizeof(id));
    read(new_socket, data, MAX_SIZE);
    read(new_socket, valor, sizeof(valor));
    read(new_socket, tipo, MAX_SIZE);
    read(new_socket, versao, MAX_SIZE);
    
    
}

int main(int argc, char* argv[]){

    struct hashtable* hashtable = new_hashtable();

    int server_fd, new_socket; 
	struct sockaddr_in address;
	int port = PORT;
	
	int opt = 1;      // for setsockopt() SO_REUSEADDR, below
	int addrlen = sizeof(address);

    
    if(argc == 2)
        port = atoi(argv[1]);
    

	
    // Creating socket file descriptor 
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    { 
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    } 
       
    // Forcefully attaching socket to the port 1300 
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
                                                  &opt, sizeof(opt))) 
    { 
        perror("setsockopt failed"); 
        exit(EXIT_FAILURE); 
    } 

    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons( port ); 
       
    // Bind the socket to the network address and port
    if (  bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0  ) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    if (listen(server_fd, 3) < 0) 
    { 
        perror("listen failed"); 
        exit(EXIT_FAILURE); 
    }

    // Wait for a connection
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,  
                       (socklen_t*)&addrlen))<0) 
    { 
        perror("accept failed"); 
        exit(EXIT_FAILURE); 
    }
    
    
    printf("Client connected.\n");

    iniciar_sensor(new_socket, hashtable);

    printf("Disconnected.\n");
    close(server_fd);

	return 0; 
}