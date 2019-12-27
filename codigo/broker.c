#include "header.h"



void sensor_initialize(int new_client, struct sensor_arrays* arrays){
	char buffer[BUFF_SIZE];
	memset(buffer, '\0', BUFF_SIZE);

	// recieve message with sensor data
	recv(new_client, buffer, BUFF_SIZE, 0);

	// split message
	char split[4][SENSOR_CHAR_LIMIT];
	strsplit(buffer, 4, split);

	// create new sensor and node
	struct sensor* new = sensor_new(split[0], split[1], split[2], split[3]);
	struct sensor_node* new_node = sensor_node_new(new, new_client);

	// insert into arrays
	sensor_arrays_insert(new_node, arrays);
}


void sensor_message(int socket, struct sensor_arrays* arrays){
	char buffer[BUFF_SIZE];
	memset(buffer, '\0', BUFF_SIZE);

	// recieve message data
	recv(socket, buffer, BUFF_SIZE, 0);

	// split data
	char split[5][SENSOR_CHAR_LIMIT];
	strsplit(buffer, 5, split);


	// create a message struct
	struct sensor_message* message = sensor_message_new(split[0], split[1],
		split[2], split[3], split[4]);

	// get respective node
	struct sensor_node* node = arrays->socket[socket];

	// insert message into node array
	insert_message(message, node);

	// prints
	/*
	printf("Sensor %d:\n", node->socket);
	
	for(int i = 0; i < node->log_counter; i++){
		printf("%s %s %s %s %s\n", node->log[i]->id, node->log[i]->date,
		node->log[i]->value, node->log[i]->type, node->log[i]->version);
	}
	*/
}


/*---------------------------------------------------------------------------*/


void public_cli_event(int socket, struct sensor_arrays* arrays){
	char buffer[BUFF_SIZE];
	memset(buffer, '\0', BUFF_SIZE);
	//char value[SENSOR_CHAR_LIMIT];

	// receive client request
	recv(socket, buffer, BUFF_SIZE, 0);

	char split[2][SENSOR_CHAR_LIMIT];

	for(int i = 0; i < 2; i++)
		memset(split[i], '\0', SENSOR_CHAR_LIMIT);

	
	strsplit(buffer, 2, split);

	/*TODO visto que os arrays estão ordenados podemos procurar o primeiro elemento
	duma maneira mais inteligente. Tambem pode-se deixar de precorrer o array quando
	sabemos q não existe mais nada à frente.*/
	int counter = 0, i = 0, start = 0;


	switch(split[0][0]){
		// sends locations of sensor of a certain type
		case 'T':
		// walks sensor type array and counts
		while(arrays->type[i] != NULL){
			// if it's the last one
			if(counter != 0 && strcmp(split[1], arrays->type[i]->sensor->type) != 0)
				break;

			if(strcmp(split[1], arrays->type[i]->sensor->type) == 0)
				counter++;

			if(counter == 1)
				start = i;
			
			i++;
		}

		// send number of sensors found
		send(socket, &counter, sizeof(counter), 0);

		for(i = start; i < start + counter; i ++){
			if(arrays->type[i] != NULL){
				// if it's the type we are looking after
				// sends it's location
				send(socket, arrays->type[i]->sensor->location, SENSOR_CHAR_LIMIT, 0);
			}
		}
		printf("Public client on socket %d type request fulfilled.\n", socket);
		break;

		
		// sends last reading of a location
		case 'L':
		// walks sensor location array and counts
		while(arrays->location[i] != NULL){
			// if it's the last one
			if(counter != 0 && strcmp(split[1], arrays->location[i]->sensor->location) != 0)
				break;

			if(strcmp(split[1], arrays->location[i]->sensor->location) == 0)
				counter++;

			if(counter == 1)
				start = i;
			
			i++;
		}

		// send number of sensors found
		send(socket, &counter, sizeof(counter), 0);

		for(i = start; i < start + counter; i ++){
			if(arrays->location[i] != NULL){
				
				if(arrays->location[i]->log[0] != NULL){
					// build the message
					memset(buffer, '\0', BUFF_SIZE);

					strcat(buffer, arrays->location[i]->log[0]->date);
					strcat(buffer, ";");
					strcat(buffer, arrays->location[i]->log[0]->value);
					strcat(buffer, ";");
					strcat(buffer, arrays->location[i]->log[0]->type);

					// send message
					send(socket, buffer, BUFF_SIZE, 0);
				}
				// no new data
				else{
					strcat(buffer, "No new data;No new data;No new data");

					// send message
					send(socket, buffer, BUFF_SIZE, 0);
				}
			}
		}
		printf("Public client on socket %d locations request fulfilled.\n", socket);
		break;

		//TODO publish subscribe
		case 'P':
		break;

		default:
		break;
	}
	
}


/*---------------------------------------------------------------------------*/


void admin_cli_event(int socket, struct sensor_arrays* arrays){
	char buffer[BUFF_SIZE];
	memset(buffer, '\0', BUFF_SIZE);

	// receive client request
	recv(socket, buffer, BUFF_SIZE, 0);

	char split[2][SENSOR_CHAR_LIMIT];

	for(int i = 0; i < 2; i++)
		memset(split[i], '\0', SENSOR_CHAR_LIMIT);

	
	strsplit(buffer, 2, split);
	int comp;
	memset(buffer, '\0', BUFF_SIZE);

	switch(split[0][0]){
		case 'R':
		//TODO pesquisa binária

		for(int i = 0; i < arrays->sensor_counter; i++){
			if(arrays->id[i] != NULL){
				comp = strcmp(arrays->id[i]->sensor->id, split[1]);

				// found it
				if(comp == 0){
					// build the message
					if(arrays->id[i]->log[0] != NULL){
						strcat(buffer, arrays->id[i]->log[0]->id);
						strcat(buffer, ";");
						strcat(buffer, arrays->id[i]->log[0]->date);
						strcat(buffer, ";");
						strcat(buffer, arrays->id[i]->log[0]->type);
						strcat(buffer, ";");
						strcat(buffer, arrays->id[i]->log[0]->value);
						strcat(buffer, ";");
						strcat(buffer, arrays->id[i]->log[0]->version);
					}
					else{
						strcat(buffer, "No;mensages;;;");
					}

					break;
				}

				// doesn't exist
				if(comp > 0){
					strcat(buffer, "Sensor;not;found;;");
					break;
				}
			}
		}
		// send message
		send(socket, buffer, BUFF_SIZE, 0);
		printf("Admin client on socket %d sensor log request fulfilled.\n", socket);
		break;

		case 'L':
		break;

		case 'U':
		break;

		case 'D':
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


//creates a listening socket
	listening = socket(AF_INET, SOCK_STREAM, 0);

//Test if the socket was successfully created.
	if(listening < 0) {
		printf("ERROR, socket creation failed.\n Aborting.\n");
		exit(0);
	
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
	char authentication;

	struct sensor_arrays* arrays = sensor_arrays_new();

//TODO enviar firmwares para os sensores


//main loop
	while(true){

		// copy contrains the same sockets as master.
		copy = master;

		test = select(sock_counter + 1, &copy, NULL, NULL, NULL);

		if(test == -1){
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
						sensor_initialize(new_client,arrays);
						printf("new sensor connected on %d\n", new_client);
						break;

						case 'C':
						printf("new public client connected on %d\n", new_client);
						break;

						case 'A':
						printf("new admin client connected on %d\n", new_client);
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

					recv(current_sock, &authentication, 1, 0);
					
					switch(authentication){
						case 'S':
						sensor_message(current_sock, arrays);
						break;

						case 'C':
						public_cli_event(current_sock, arrays);
						break;

						case 'A':
						admin_cli_event(current_sock, arrays);
						break;

						default:
						break;
					}
				}
			}
		}
	}

	return 0;
}