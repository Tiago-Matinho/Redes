#include "header.h"



void sensor_initialize(int new_client, struct sensor_arrays* order){
	char buffer[BUFF_SIZE];
	memset(buffer, '\0', BUFF_SIZE);

	// recieve message with sensor data
	recv(new_client, buffer, BUFF_SIZE, 0);

	// split message
	char split[4][SENSOR_CHAR_LIMIT];
	strsplit(buffer, 4, split);

	// create new sensor and node
	struct sensor* new = sensor_new(atoi(split[0]), split[1], split[2], split[3]);
	struct sensor_node* new_node = sensor_node_new(new, new_client);

	// insert into arrays
	sensor_arrays_insert(new_node, order);
}


void sensor_message(int socket, struct sensor_arrays* order){
	char buffer[BUFF_SIZE];
	memset(buffer, '\0', BUFF_SIZE);

	// recieve message data
	recv(socket, buffer, BUFF_SIZE, 0);

	// split data
	char split[5][SENSOR_CHAR_LIMIT];
	strsplit(buffer, 5, split);

	// create a message struct
	struct sensor_message* message = sensor_message_new(atoi(split[0]), split[1],
		atoi(split[2]), split[3], split[4]);

	// get respective node
	struct sensor_node* node = order->socket[SOCK_TO_INDEX(socket)];

	// insert message into node array
	insert_message(message, node);

	printf("Cliente %d:\n", node->socket);
	
	for(int i = 0; i < node->log_counter; i++){
		printf("% 3d %s % 3d %s %s\n", node->log[i]->id, node->log[i]->date,
		node->log[i]->value, node->log[i]->type, node->log[i]->version);
	}
}


/*---------------------------------------------------------------------------*/


void public_cli_event(int socket, struct sensor_arrays* order){
	char buffer[BUFF_SIZE];
	char value[SENSOR_CHAR_LIMIT];
	memset(buffer, '\0', BUFF_SIZE);

	// receive client request
	recv(socket, buffer, BUFF_SIZE, 0);

	char split[2][SENSOR_CHAR_LIMIT];

	for(int i = 0; i < 2; i++)
		memset(split, '\0', SENSOR_CHAR_LIMIT);

	
	strsplit(buffer, 2, split);

	/*TODO visto que os arrays estão ordenados podemos procurar o primeiro elemento
	duma maneira mais inteligente. Tambem pode-se deixar de precorrer o array quando
	sabemos q não existe mais nada à frente.*/
	int counter = 0, i = 0, start = 0;

	printf("Chega aqui\n");
	
	switch(split[0][0]){
		// sends locations of sensor of a certain type
		case 'T':
		// walks sensor type array and counts
		while(order->type[i] != NULL){
			// if it's the last one
			if(counter != 0 && strcmp(split[1], order->type[i]->sensor->type) != 0)
				break;

			if(strcmp(split[1], order->type[i]->sensor->type) == 0)
				counter++;

			if(counter == 1)
				start = i;
			
			i++;
		}

		// send number of sensors found
		send(socket, &counter, sizeof(counter), 0);

		for(i = start; i < start + counter; i ++){
			if(order->type[i] != NULL){
				// if it's the type we are looking after
				// sends it's location
				send(socket, order->type[i]->sensor->location, SENSOR_CHAR_LIMIT, 0);	
			}
		}
		break;

		//FIXME
		// sends last reading of a location
		case 'L':
		// walks sensor location array
		for(i = 0; i < order->sensor_counter; i ++){
			if(order->location[i] != NULL){
				// if it's the right location sends it's last reading
				if(strcmp(split[1], order->location[i]->sensor->location) == 0){
					if(order->location[i]->log[0] != NULL){
						// build the message
						memset(buffer, '\0', BUFF_SIZE);
						memset(value, '\0', SENSOR_CHAR_LIMIT);
						snprintf(value, SENSOR_CHAR_LIMIT, "%d", order->location[i]->log[0]->value);
						
						strcat(buffer, order->location[i]->log[0]->date);
						strcat(buffer, ";");
						strcat(buffer, value);
						strcat(buffer, ";");
						strcat(buffer, order->location[i]->log[0]->type);

						// send message
						send(socket, buffer, strlen(buffer), 0);	
					}
				}
			}
		}
		break;

		default:
		break;
	}
	
}


/*---------------------------------------------------------------------------*/


int main(int argc, char *argv[]){

	struct sockaddr_in address;

	int listening, 
		new_client,
		address_len = sizeof(address), 
		opt = 1,
		sock_counter = 0;

	fd_set master, copy;

	/*struct timeval time;
	time.tv_sec = SENSOR_INTREVAL;
	time.tv_usec = 0;*/

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


// saves what kind of client it is (Sensor, public client, admin)
	char client_type[MAX_CLIENTS];
	memset(client_type, '\0', MAX_CLIENTS);

	char authentication;

	struct sensor_arrays* order = sensor_arrays_new();

//main loop
	while(true){

		// copy contrains the same sockets as master.
		copy = master;

		test = select(sock_counter +1 , &copy, NULL, NULL, NULL);

		if(test == -1) {
			perror("Select error.");
			exit(4);
		}

		for(current_sock = 0; current_sock <= sock_counter; current_sock++){

			if(FD_ISSET(current_sock, &copy)){
				if(current_sock == listening){


					//accept new connection
					new_client = accept(listening, (struct sockaddr *) &address, (socklen_t * )&address_len);

					if(new_client == -1){
						perror("Accept of the nre socket failed.");
						exit(1);
					}


					recv(new_client, &authentication, 1, 0);

					switch(authentication){
						case 'S':
						client_type[SOCK_TO_INDEX(current_sock)] = 'S';
						sensor_initialize(new_client,order);
						printf("new sensor connected on %d\n", new_client);
						break;

						case 'C':
						client_type[SOCK_TO_INDEX(current_sock)] = 'C';
						printf("new public client connected on %d\n", new_client);
						break;

						case 'A':
						client_type[SOCK_TO_INDEX(current_sock)] = 'A';
						break;

						default:
						break;
					}


					FD_SET(new_client, &master);

					if(sock_counter < new_client){
						sock_counter = new_client;
					}

				}

				else{
					//é um cliente a enviar mensagem

					switch(client_type[SOCK_TO_INDEX(current_sock)]){
						case 'S':
						sensor_message(current_sock, order);
						break;

						case 'C':
						public_cli_event(current_sock, order);
						break;

						case 'A':
						break;

						default:
						break;
					}
				}
			}
		}
	}
}