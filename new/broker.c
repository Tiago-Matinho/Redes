#include "header.h"



void sensor_initialize(int new_client, struct sensor* array[100]){
	char buffer[BUFF_SIZE];
	memset(buffer, '\0', BUFF_SIZE);

	recv(new_client, buffer, BUFF_SIZE, 0);

	char split[4][SENSOR_CHAR_LIMIT];
	strsplit(buffer, 4, split);

	struct sensor* new = sensor_new(atoi(split[0]), split[1], split[2], split[3]);

	array[new_client] = new;
}


void sensor_message(int socket, struct sensor* array[100]){
	char buffer[BUFF_SIZE];
	memset(buffer, '\0', BUFF_SIZE);

	recv(socket, buffer, BUFF_SIZE, 0);

	char split[5][SENSOR_CHAR_LIMIT];
	strsplit(buffer, 5, split);

	printf("%d @ %s: %d %s version: %s\n", atoi(split[0]), split[1],
		atoi(split[2]), split[3], split[4]);
}

int main(int argc, char *argv[]){

	struct sockaddr_in address;

	struct sensor* array[100];

	for(int i = 0; i < 100; i++)
		array[i] = NULL;


	int listening, 
		new_client,
		address_len = sizeof(address), 
		opt = 1,
		sock_counter = 0;

	fd_set master, copy;

	struct timeval time;
	time.tv_sec = SENSOR_INTREVAL;
	time.tv_usec = 0;

//creates a listening socket
	listening = socket(AF_INET, SOCK_STREAM, 0);

//Test if the socket was successfully created.
	if(listening < 0) {
		printf("ERROR, socket creation failed.\n Aborting.\n");
		exit(0);
	
	} else {
		printf("Socket successfully created.\n");
	}

//Make sure that socket doesnt reserve the port.
	if(setsockopt(listening, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {

		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

//Assign IP and port to the socket.
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(BROKER_PORT);

	if(bind(listening, (struct sockaddr *) &address, sizeof(address)) < 0) {
		printf("Socket not bound.\n");
		exit(EXIT_FAILURE);
	
	} else {
		printf("Socket bound.\n");
	
	}

//Setting the server to listen the client.
	if(listen(listening, 10) < 0) {
		printf("Listen failed.\n");
		exit(EXIT_FAILURE);
	}

//Clearing the master queue and adding the server socket to it.
	FD_ZERO(&master);
	FD_SET(listening, &master);

	sock_counter = listening;

	int current_sock, test;

	char buffer[BUFF_SIZE];

//main loop
	while(true){

		//copy contrains the same sockets as master.
		copy = master;

		test = select(sock_counter +1 , &copy, NULL, NULL, NULL);

		if(test == -1) {
			perror("Select error.");
			exit(4);
		}

		for(current_sock = 0; current_sock <= sock_counter; current_sock++){

			if(FD_ISSET(current_sock, &copy)){
				if(current_sock == listening){

					printf("Aceitando novas conecções\n");

					//accept new connection
					new_client = accept(listening, (struct sockaddr *) &address, (socklen_t * )&address_len);

					if(new_client == -1){
						perror("Accept of the nre socket failed.");
						exit(1);
					}

					// initialize sensor
					sensor_initialize(new_client, array);

					FD_SET(new_client, &master);

					if(sock_counter < new_client){
						sock_counter = new_client;
					}

				}


				else{
					//é um cliente a enviar mensagem

					printf("Client @ %d says:\n", current_sock);
					sensor_message(current_sock, array);
				}
			}
		}
	}
}