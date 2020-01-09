#include "header.h"


// DONE
void sensor_initialize(int new_client, struct sensor_arrays* arrays){
	char buffer[BUFF_SIZE];
	memset(buffer, '\0', BUFF_SIZE);

	// recieve message with sensor data
	recv(new_client, buffer, BUFF_SIZE, 0);

	// split message
	char split[4][SENSOR_CHAR_LIMIT];
	strsplit(buffer, ';',  4, split);

	// checks if sensor is blocked or exist
	struct sensor_node* temp1 = id_search(arrays->id, arrays->sensor_counter, split[0]);
	struct sensor_node* temp2 = id_search(arrays->blocked, arrays->blocked_counter, split[0]);

	char accepted = '\0';

	if(temp1 != NULL || temp2 != NULL){
		accepted = 'F';
		printf("Rejected client on socket %d.\n", new_client);
		send(new_client, &accepted, 1, 0);
		return;
	}
	
	// create new sensor and node
	struct sensor* new = sensor_new(split[0], split[1], split[2], split[3]);
	struct sensor_node* new_node = sensor_node_new(new, new_client);

	// insert into arrays
	sensor_arrays_insert(new_node, arrays);
	

	accepted = 'T';
	send(new_client, &accepted, 1, 0);
}


//DONE
void sensor_update_initialize(int new_socket, struct sensor_arrays* arrays){
	char buffer[BUFF_SIZE];
	memset(buffer, '\0', BUFF_SIZE);

	// recieve message with sensor data
	recv(new_socket, buffer, BUFF_SIZE, 0);

	// split message
	char split[4][SENSOR_CHAR_LIMIT];
	strsplit(buffer, ';',  4, split);

	struct sensor_node* temp1 = id_search(arrays->id, arrays->sensor_counter, split[0]);

	if(temp1 == NULL){
		printf("Update sensor socket ERROR: sensor %s not found\n", split[0]);
		exit(1);
	}

	temp1->update_socket = new_socket;
}


//DONE
void sensor_message(int socket, struct sensor_arrays* arrays){
	char buffer[BUFF_SIZE];
	memset(buffer, '\0', BUFF_SIZE);

	// recieve message data
	recv(socket, buffer, BUFF_SIZE, 0);

	// split data
	char split[5][SENSOR_CHAR_LIMIT];
	strsplit(buffer, ';',  5, split);


	// create a message struct
	struct sensor_message* message = sensor_message_new(split[0], split[1],
		split[2], split[3], split[4]);

	// get respective node
	struct sensor_node* node = arrays->socket[socket];

	// insert message into sensor message array
	insert_message(message, node);

	// prints
	/*
	printf("Sensor %d:\n", node->sensor_socket);
	
	for(int i = 0; i < node->log_counter; i++){
		printf("%s %s %s %s %s\n", node->log[i]->id, node->log[i]->date,
		node->log[i]->value, node->log[i]->type, node->log[i]->version);
	}
	*/

	// send to subscribed clients
	struct public_cli* subscriber = NULL;

	for(int i = 0; i < node->subs_counter; i++){
		subscriber = node->subs[i];

		if(subscriber != NULL){
			send(subscriber->subscribe_socket, buffer, BUFF_SIZE, 0);
		}
	}
}


/*---------------------------------------------------------------------------*/


// DONE
void public_cli_initialize(int socket, struct pub_clients* clients){
	struct public_cli* new = public_cli_new(socket);

	pub_clients_add(clients, new);
	send(socket, &socket, sizeof(socket), 0);
}


// DONE
void subscribe_initialize(int subscribe_socket, struct pub_clients* clients){
	int socket;

	recv(subscribe_socket, &socket, sizeof(socket), 0);

	struct public_cli* temp = pub_clients_get(clients, socket);

	if(temp == NULL){
		printf("Something strange happened: temp is NULL\n");
		exit(1);
	}

	temp->subscribe_socket = subscribe_socket;

	send(subscribe_socket, &subscribe_socket, sizeof(subscribe_socket), 0);
}


// DONE
void public_cli_event(int socket, struct sensor_arrays* arrays, struct pub_clients* clients){
	char buffer[BUFF_SIZE];
	memset(buffer, '\0', BUFF_SIZE);

	// receive client request
	recv(socket, buffer, BUFF_SIZE, 0);

	char split[2][SENSOR_CHAR_LIMIT];

	for(int i = 0; i < 2; i++)
		memset(split[i], '\0', SENSOR_CHAR_LIMIT);

	
	strsplit(buffer, ';',  2, split);


	int counter = 0, i = 0, start = 0;

	if(strcmp(split[0], "list") == 0){
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
				// sends it's location
				send(socket, arrays->type[i]->sensor->location, SENSOR_CHAR_LIMIT, 0);
			}
		}
		printf("Public client on socket %d type request fulfilled.\n", socket);
	}

	else if(strcmp(split[0], "last") == 0){
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
	}

	else if(strcmp(split[0], "subscribe") == 0){
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

		for(i = start; i < start + counter; i ++){
			// add to subscribe list
			if(arrays->location[i] != NULL)
				sub_sensor(arrays->location[i], pub_clients_get(clients, socket));	
		}
		printf("Public client on socket %d subscribe request fulfilled.\n", socket);
	}

	else
		printf("Something strange happened.\n");
}


/*---------------------------------------------------------------------------*/


//DONE
void admin_cli_event(int socket, struct sensor_arrays* arrays, fd_set* master){
	char buffer[BUFF_SIZE];
	memset(buffer, '\0', BUFF_SIZE);

	// receive client request
	recv(socket, buffer, BUFF_SIZE, 0);

	int spl = 2;
	
	// update sends 2 args
	if(buffer[0] == 'u')
		spl = 3;

	char split[spl][SENSOR_CHAR_LIMIT];

	for(int i = 0; i < spl; i++)
		memset(split[i], '\0', SENSOR_CHAR_LIMIT);

	
	strsplit(buffer, ';',  spl, split);
	int n, a;
	memset(buffer, '\0', BUFF_SIZE);
	struct sensor_node* temp = NULL;

	// last reading
	if(strcmp(split[0], "last") == 0){
		// search for the sensor
		temp = id_search(arrays->id, arrays->sensor_counter, split[1]);
		// sensor not found
		if(temp == NULL){
			strcat(buffer, "sensor:");
			strcat(buffer, ";");
			strcat(buffer, split[1]);
			strcat(buffer, ";");
			strcat(buffer, "not found");
			strcat(buffer, ";");
			strcat(buffer, ";");
		}

		else{
			if(temp->log[0] != NULL){
				strcat(buffer, temp->log[0]->id);
				strcat(buffer, ";");
				strcat(buffer, temp->log[0]->date);
				strcat(buffer, ";");
				strcat(buffer, temp->log[0]->type);
				strcat(buffer, ";");
				strcat(buffer, temp->log[0]->value);
				strcat(buffer, ";");
				strcat(buffer, temp->log[0]->version);
			}
			else{
				strcat(buffer, "No;mensages;;;");
			}
		}
		// send message
		send(socket, buffer, BUFF_SIZE, 0);
		printf("Admin client on socket %d sensor log request fulfilled.\n", socket);
	}

	else if(strcmp(split[0], "list") == 0){
		// send number of sensors
		n = arrays->sensor_counter;
		send(socket, &n, sizeof(n), 0);

		for(int i = 0; i < n; i++){
			memset(buffer, '\0', BUFF_SIZE);

			// build the message
			strcat(buffer, arrays->id[i]->sensor->id);
			strcat(buffer, ";");
			strcat(buffer, arrays->id[i]->sensor->type);
			strcat(buffer, ";");
			strcat(buffer, arrays->id[i]->sensor->location);
			strcat(buffer, ";");
			strcat(buffer, arrays->id[i]->sensor->version);

			// send message
			send(socket, buffer, BUFF_SIZE, 0);
		}
		printf("Admin client on socket %d sensor list request fulfilled.\n", socket);
	}

	else if(strcmp(split[0], "update") == 0){
		temp = id_search(arrays->id, arrays->sensor_counter, split[1]);

		if(temp == NULL)
			strcat(buffer, "Sensor not found.");
		

		else{
			if(strcmp(temp->sensor->version, split[2]) > 0)
				strcat(buffer, "sensor is on a newer version.");
			
			else{
				memset(temp->sensor->version, '\0', SENSOR_CHAR_LIMIT);
				strcpy(temp->sensor->version, split[2]);
				send(temp->update_socket, split[2], SENSOR_CHAR_LIMIT, 0);
				strcat(buffer, "update sent.");
			}
		}
		
		send(socket, buffer, BUFF_SIZE, 0);
		printf("Admin client on socket %d sensor update request fulfilled.\n", socket);
	}

	else if(strcmp(split[0], "desactivate") == 0){
		temp = id_search(arrays->id, arrays->sensor_counter, split[1]);

		if(temp == NULL)
			strcat(buffer, "sensor not found.");
		

		else{
			temp = sensor_arrays_remove(arrays, split[1]);
			n = temp->sensor_socket;
			a = temp->update_socket;
			FD_CLR(n, master);
			FD_CLR(a, master);
			strcat(buffer, "sensor removed from broker.");
		}

		send(socket, buffer, BUFF_SIZE, 0);
		printf("Sensor on socket %d removed.\n", socket);
		close(n);
		close(a);
	}

	else
		printf("Something strange happened.\n");

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
		printf("ERROR, socket creation failed.\n");
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

	if(bind(listening, (struct sockaddr *) &address, sizeof(address)) < 0){
		printf("Socket not bound.\n");
		exit(EXIT_FAILURE);
	}

//Setting the server to listen the client.
	if(listen(listening, 10) < 0){
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
	struct pub_clients* clients = pub_clients_new();


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
						sensor_initialize(new_client, arrays);
						printf("new sensor connected on %d\n", new_client);
						break;

						case 'U':
						sensor_update_initialize(new_client, arrays);
						printf("new update sensor connected on %d\n", new_client);
						break;

						case 'C':
						public_cli_initialize(new_client, clients);
						printf("new public client connected on %d\n", new_client);
						break;

						case 'P':
						subscribe_initialize(new_client, clients);
						printf("new public client subscribe channel connected on %d\n", new_client);
						break;

						case 'A':
						printf("new admin client connected on %d\n", new_client);
						break;

						default:
						break;
					}


					FD_SET(new_client, &master);

					// update counter
					if(sock_counter < new_client){
						sock_counter = new_client;
					}

				}

				// handle messages
				else{

					recv(current_sock, &authentication, 1, 0);
					
					switch(authentication){
						case 'S':
						sensor_message(current_sock, arrays);
						break;

						case 'C':
						public_cli_event(current_sock, arrays, clients);
						break;

						case 'A':
						admin_cli_event(current_sock, arrays, &master);
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