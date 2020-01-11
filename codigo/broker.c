#include "library.h"


/*
When a new sensor connects it's added to an array that stores it's
information.

This function will search if the sensor's ID is already in use or if
it's blocked.

If the sensor is accepted then a accepted message is sent to the
sensor. If not then a rejection message is sent.
*/
bool sensor_initialize(int new_client, struct sensor_arrays* arrays){
	char buffer[BUFF_SIZE];
	memset(buffer, '\0', BUFF_SIZE);

	// recieve message with sensor data
	recv(new_client, buffer, BUFF_SIZE, 0);

	// split message
	char split[4][SENSOR_CHAR_LIMIT];
	strsplit(buffer, ';',  4, split);

	// checks if sensor is blocked or already is in use
	struct sensor_node* temp1 = id_search(arrays->id,
		arrays->sensor_counter, split[0]);

	struct sensor_node* temp2 = id_search(arrays->blocked,
		arrays->blocked_counter, split[0]);

	char accepted = '\0';

	// sends rejection message
	if(temp1 != NULL || temp2 != NULL){
		accepted = 'F';
		printf("Rejected client on socket %d.\n", new_client);
		send(new_client, &accepted, 1, 0);
		return false;
	}
	
	// create new sensor and respective node
	struct sensor* new = sensor_new(split[0], split[1], split[2],
		split[3]);
	struct sensor_node* new_node = sensor_node_new(new, new_client);

	// insert node into arrays
	sensor_arrays_insert(new_node, arrays);
	

	// send accepted message
	accepted = 'T';
	send(new_client, &accepted, 1, 0);
	return true;
}


/*
When a new sensor connects it will need a new socket so that the sensor
can recieve updates.

In order to know what sensor this update socket corresponds the sensor
sends it's information and links the update socket to the already
existing sensor.
*/
void sensor_update_initialize(int new_socket,
	struct sensor_arrays* arrays){

	char buffer[BUFF_SIZE];
	memset(buffer, '\0', BUFF_SIZE);

	// recieve message with sensor data
	recv(new_socket, buffer, BUFF_SIZE, 0);

	// split message
	char split[4][SENSOR_CHAR_LIMIT];
	strsplit(buffer, ';',  4, split);

	// search for that sensor
	struct sensor_node* temp1 = id_search(arrays->id,
		arrays->sensor_counter, split[0]);

	// sensor not found
	if(temp1 == NULL){
		printf("Update sensor socket ERROR: sensor %s not found\n",
			split[0]);
		exit(1);
	}

	// link the sensor to the respective update socket
	temp1->update_socket = new_socket;
}


/*
Everytime a sensor sends a new message the message it's stored in the
sensor's node.

Then sends the message to all the public clients that are subscribed to
the sensor. 
*/
void sensor_message(int socket, struct sensor_arrays* arrays){
	char buffer[BUFF_SIZE];
	memset(buffer, '\0', BUFF_SIZE);

	// recieve message data
	recv(socket, buffer, BUFF_SIZE, 0);

	// split data
	char split[5][SENSOR_CHAR_LIMIT];
	strsplit(buffer, ';',  5, split);


	// create a message struct
	struct sensor_message* message = sensor_message_new(split[0],
		split[1], split[2], split[3], split[4]);

	// get respective node using the socket the data was sent from
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

		/*
		sends to the subscription socket of the public client that are
		subscribed
		*/
		if(subscriber != NULL){
			send(subscriber->subscribe_socket, buffer, BUFF_SIZE, 0);
		}
	}
}


/*-------------------------------------------------------------------*/


/*
When a new public client connects, the broker creates a public_cli
struct that will store the client socket and it's subscription socket.
*/
void public_cli_initialize(int socket, struct pub_clients* clients){
	struct public_cli* new = public_cli_new(socket);

	pub_clients_add(clients, new);

	// sends what socket the public client is using.(Like an unique ID).
	send(socket, &socket, sizeof(socket), 0);
}


/*
Links the subscription socket to the public client. So that later the
broker can send sensor data to the public client.
*/
void subscribe_initialize(int subscribe_socket,
	struct pub_clients* clients){
	
	int socket;

	// recieves the socket of the public client.
	// (Like an ID) so that the broker can link the two.
	recv(subscribe_socket, &socket, sizeof(socket), 0);

	// searches for the client
	struct public_cli* temp = pub_clients_get(clients, socket);

	if(temp == NULL){
		printf("Something strange happened: temp is NULL\n");
		exit(1);
	}

	// links the socket
	temp->subscribe_socket = subscribe_socket;
}


/*
Handles all the public client requests.

The client sends the requests following the structure: "command;arg".

Then the broker handles the requests.

It should be knowed that the struct sensor_arrays has an array for
every information the sensor stores. Furthermore the arrays are all in
order.

Example for the type information:
+-------+-------+-------+-------+
|   0   |   1   |   2   |   3   |
|  CH4  |  CO2  |  CO2  |  NULL |
+-------+-------+-------+-------+

*/
void public_cli_event(int socket, struct sensor_arrays* arrays,
	struct pub_clients* clients){

	char buffer[BUFF_SIZE];
	memset(buffer, '\0', BUFF_SIZE);

	// receive client request
	recv(socket, buffer, BUFF_SIZE, 0);

	char split[2][SENSOR_CHAR_LIMIT];

	for(int i = 0; i < 2; i++)
		memset(split[i], '\0', SENSOR_CHAR_LIMIT);

	
	// split the message so that split[0] = command and split[1] = arg
	strsplit(buffer, ';',  2, split);


	int counter = 0, i = 0, start = 0;

	// sends all the sensors that types correspond to the requested
	if(strcmp(split[0], "list") == 0){

		// walks sensor type array and counts
		while(arrays->type[i] != NULL){
			// if it's the last one
			if(counter != 0 && strcmp(split[1],
				arrays->type[i]->sensor->type) != 0)
				break;

			if(strcmp(split[1], arrays->type[i]->sensor->type) == 0)
				counter++;

			//found the first one
			if(counter == 1)
				start = i;
			
			i++;
		}

		// send number of sensors found
		send(socket, &counter, sizeof(counter), 0);

		// walks until the last sensor with the requested type
		for(i = start; i < start + counter; i ++){
			if(arrays->type[i] != NULL){
				// sends it's location
				send(socket, arrays->type[i]->sensor->location,
					SENSOR_CHAR_LIMIT, 0);
			}
		}
		printf("Public client on socket %d type request fulfilled.\n",
			socket);
	}

	/*
	sends the last messages recieved from sensors from the requested
	location
	*/
	else if(strcmp(split[0], "last") == 0){
		// walks sensor location array and counts
		while(arrays->location[i] != NULL){
			// if it's the last one
			if(counter != 0 && strcmp(split[1],
				arrays->location[i]->sensor->location) != 0)
				break;

			if(strcmp(split[1], arrays->location[i]->sensor->location)
				== 0)
				counter++;

			//found the first
			if(counter == 1)
				start = i;
			
			i++;
		}

		// send number of sensors found
		send(socket, &counter, sizeof(counter), 0);

		// walks until the last sensor with the requested location
		for(i = start; i < start + counter; i ++){
			if(arrays->location[i] != NULL){
				
				// if there's a message
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
					strcat(buffer, "No new data;;");

					// send message
					send(socket, buffer, BUFF_SIZE, 0);
				}
			}
		}
		printf("Public client on socket %d locations request fulfilled.\n",
			socket);
	}


	/*
	searches for the sensors on the requested location. Then adds the
	client to the subscription list on the sensors.
	*/
	else if(strcmp(split[0], "subscribe") == 0){

		// walks sensor location array and counts
		while(arrays->location[i] != NULL){
			// if it's the last one
			if(counter != 0 && strcmp(split[1],
				arrays->location[i]->sensor->location) != 0)
				break;

			if(strcmp(split[1], arrays->location[i]->sensor->location)
				== 0)
				counter++;

			// found the first
			if(counter == 1)
				start = i;
			
			i++;
		}

		// walks until the last sensor with the requested location
		for(i = start; i < start + counter; i ++){
			// add to subscribe list
			if(arrays->location[i] != NULL)
				sub_sensor(arrays->location[i],
					pub_clients_get(clients, socket));	
		}
		// sends number of sensors the public client subscribed to 
		send(socket, &counter, sizeof(counter), 0);
		printf("Public client on socket %d subscribe request
			fulfilled.\n", socket);
	}

	else
		printf("Something strange happened.\n");
}


/*-------------------------------------------------------------------*/


/*
Handles all the admin client requests.

The admin sends the requests following the structure:
- for last reading and desactivate: "command;arg".
- for list all the sensors: "command".
- for update: "command;arg1;arg2".

*/
void admin_cli_event(int socket, struct sensor_arrays* arrays,
	fd_set* master){

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

	
	// split the message
	strsplit(buffer, ';',  spl, split);

	int counter, start, n, i, a = 0;
	memset(buffer, '\0', BUFF_SIZE);
	struct sensor_node* temp = NULL;

	// sends the last reading of the sensor with the requested ID
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

		// sensor was found
		else{
			// there's a reading
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
				strcat(buffer, "No;readings;yet;;");
			}
		}
		// sends message
		send(socket, buffer, BUFF_SIZE, 0);
		printf("Admin client on socket %d sensor log request
			fulfilled.\n", socket);
	}

	// sends all the information about all the sensors connected
	else if(strcmp(split[0], "list") == 0){
		// send number of sensors
		n = arrays->sensor_counter;
		send(socket, &n, sizeof(n), 0);

		// walks all the sensors
		for(int i = 0; i < n; i++){
			memset(buffer, '\0', BUFF_SIZE);

			if(arrays->id[i] == NULL)
				exit(1);

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
		printf("Admin client on socket %d sensor list request
			fulfilled.\n", socket);
	}

	// sends update to the requested sensor
	else if(strcmp(split[0], "update") == 0){

		// walks sensor type array and counts
		while(arrays->type[i] != NULL){
			// if it's the last one
			if(counter != 0 && strcmp(split[1],
				arrays->type[i]->sensor->type) != 0)
				break;

			if(strcmp(split[1], arrays->type[i]->sensor->type) == 0)
				counter++;

			//found the first one
			if(counter == 1)
				start = i;
			
			i++;
		}

		a = 0;

		// walks until the last sensor with the requested type
		for(i = start; i < start + counter; i ++){
			if(arrays->type[i] != NULL){
				temp = arrays->type[i];

				// if the new version is the latest

				if(strcmp(temp->sensor->version, split[2]) < 0){
					a++;
					memset(temp->sensor->version, '\0',
						SENSOR_CHAR_LIMIT);
					strcpy(temp->sensor->version, split[2]);
					// send the update
					send(temp->update_socket, split[2],
						SENSOR_CHAR_LIMIT, 0);
				}
				
			}
		}
		// sends number of sensors updated
		send(socket, &a, sizeof(a), 0);
		printf("Admin client on socket %d sensor update
			request fulfilled.\n", socket);
	}

	// closes the sensor socket and add's to the blocked list
	else if(strcmp(split[0], "desactivate") == 0){
		temp = id_search(arrays->id, arrays->sensor_counter,
			split[1]);

		if(temp == NULL)
			strcat(buffer, "sensor not found.");
		

		else{
			// adds to the blocked list
			temp = sensor_arrays_remove(arrays, split[1]);
			n = temp->sensor_socket;
			a = temp->update_socket;
			FD_CLR(n, master); //removes sensor from master
			FD_CLR(a, master); //also removes the updated socket
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


/*-------------------------------------------------------------------*/


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
	if(listening < 0){
		printf("ERROR, socket creation failed.\n");
		exit(0);
	}

//Make sure that socket doesnt reserve the port.
	if(setsockopt(listening, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
		&opt, sizeof(opt))){

		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

//Assign IP and port to the socket.
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(BROKER_PORT);

	if(bind(listening, (struct sockaddr *) &address,
		sizeof(address)) < 0){

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

	int current_sock;


/*
authenticates what kind of client it is (Sensor, Update,
public client, subscribe, admin)
*/
	char authentication;

	struct sensor_arrays* arrays = sensor_arrays_new();
	struct pub_clients* clients = pub_clients_new();

	bool test;


//main loop
	while(true){

		// copy the master fd_set
		copy = master;


		if(select(sock_counter + 1, &copy, NULL, NULL, NULL) == -1){
			perror("Select error.");
			exit(4);
		}

		// walks all the sockets
		for(current_sock = 0; current_sock <= sock_counter;
			current_sock++){

			// something new to read
			if(FD_ISSET(current_sock, &copy)){

				/*
				if the listening socket has somethin new
				(i.e. a new connection)
				*/
				if(current_sock == listening){
					//accept new connection
					new_client = accept(listening, (struct sockaddr *)
						&address, (socklen_t * )&address_len);

					test = true;

					if(new_client == -1){
						perror("Accept of the nre socket failed.");
						exit(1);
					}


					recv(new_client, &authentication, 1, 0);

					switch(authentication){
						case 'S':
						/*
						test to see if the sensor is'nt blocked or
						ID is in use
						*/
						test = sensor_initialize(new_client, arrays);

						if(test)
							printf("new sensor connected on %d\n",
								new_client);						
						break;

						case 'U':
						// update socket that will be linked to a sensor
						sensor_update_initialize(new_client, arrays);
						printf("new update sensor connected on %d\n",
							new_client);
						break;

						case 'C':
						public_cli_initialize(new_client, clients);
						printf("new public client connected on %d\n",
							new_client);
						break;

						case 'P':
						/*
						subscribe socket that will be linked to a
						public client
						*/
						subscribe_initialize(new_client, clients);
						printf("new public client subscribe channel
							connected on %d\n", new_client);
						break;

						case 'A':
						printf("new admin client connected on %d\n",
							new_client);
						break;

						default:
						break;
					}

					// adds to master queue
					if(test)
						FD_SET(new_client, &master);

					// update counter
					if(sock_counter < new_client){
						sock_counter = new_client;
					}

				}

				// handle messages and requests
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
	free_sensor_arrays(arrays);
	return 0;
}