#include "library.h"

/*
Initializes client on broker side.

To do so the public client send an authentication key ('C') then
receives wich socket was assingned to the public client. This will
then be sent to the broker so that the broker knows what client's
socket corresponds to wich client subscribe socket.
*/
void initialize_client(int* this_socket, int client_socket){
	// initialize sensor on broker side
	char authentication = 'C';

	// authenticates
	send(client_socket, &authentication, sizeof(authentication), 0);

	// receives socket on broker side. (Used as an ID). 
	recv(client_socket, this_socket, sizeof(*this_socket), 0);
}

/*
Initializes subscribe socket on broker side.

Firstly sends to the broker an authentication key ('P') then sends
the respective socket the public client is using on broker side so that
the broker can link this two.
*/
void initialize_subscribe(int* this_sub_socket, int this_socket,
	int subscribe_socket){

	// initialize sensor on broker side
	char authentication = 'P';

	// authenticates
	send(subscribe_socket, &authentication, sizeof(authentication), 0);

	// sends ID
	send(subscribe_socket, &this_socket, sizeof(this_socket), 0);
}

/*
Lists all the sensors location connected to the broker wich types match
the requested type.

Sends a message with the request following the format:
"list;TYPE"

the broker splits the string and searches the sensors, sends the number
of sensors found and sends their location.
*/
void list_locations(int socket, char type[SENSOR_CHAR_LIMIT]){
	char buffer[BUFF_SIZE];
	memset(buffer, '\0', BUFF_SIZE);

	// build request message
	strcat(buffer, "list;");
	strcat(buffer, type);

	// sends request
	send(socket, buffer, BUFF_SIZE, 0);

	// receives number of sensors that match
	int n = 0;
	recv(socket, &n, sizeof(n), 0);

	printf("\n+ Sensors found: %d\n", n);

	// receive all the locations
	for(int i = 0; i < n; i++){
		recv(socket, buffer, SENSOR_CHAR_LIMIT, 0);
		printf("+ %s\n", buffer);
	}
	printf("\n");
}

/*
Receives the last reading of all the sensors of a requested location.

Sends a message with the request following the format:
"list;LOCATION"

the broker splits the string and searches the sensors, sends the number
of sensors found and sends their last message.
*/
void last_reading(int socket, char location[SENSOR_CHAR_LIMIT]){
	char buffer[BUFF_SIZE];
	char split[3][SENSOR_CHAR_LIMIT];
	memset(buffer, '\0', BUFF_SIZE);
	
	// build request message
	strcat(buffer, "last;");
	strcat(buffer, location);

	// sends request
	send(socket, buffer, BUFF_SIZE, 0);

	// receives number of sensors that match
	int n = 0;
	recv(socket, &n, sizeof(n), 0);

	printf("\n+ Sensors found: %d\n", n);
	printf("+ %s:\n", location);

	// receive all the last readings
	for(int i = 0; i < n; i++){

		// clear array
		for(int j = 0; j < 3; j++)
			memset(split[j], '\0', SENSOR_CHAR_LIMIT);

		// receive message
		recv(socket, buffer, BUFF_SIZE, 0);

		strsplit(buffer, ';',  3, split);
		printf("- %s %s %s\n", split[0], split[1], split[2]);
	}
	printf("\n");
}

/*
Subscribes to a location. This means that all the sensors in the
requested location will send their readings to the public client.

Sends a message with the request following the format:
"subscribe;LOCATION"

This readings will be received on another socket than the used to do
the request.
*/
void subscribe(int client_socket, int this_socket,
	char location[SENSOR_CHAR_LIMIT]){
	
	char buffer[BUFF_SIZE];
	memset(buffer, '\0', BUFF_SIZE);
	
	// build request message
	strcat(buffer, "subscribe;");
	strcat(buffer, location);

	// send request
	send(client_socket, buffer, BUFF_SIZE, 0);

	int subs;
	recv(client_socket, &subs, sizeof(subs), 0);
	printf("\nSubscribed to %d sensors.\n", subs);
}


/*
Prints all the readings received from the sensors through the broker.
*/
void print_new(int socket){
	char buffer[BUFF_SIZE];
	memset(buffer, '\0', BUFF_SIZE);

	int nbytes;

	// receive message data
	nbytes = recv(socket, buffer, BUFF_SIZE, 0);

	// crash proof
	if(nbytes <= 0)
		exit(1);
	
	// split data
	char split[5][SENSOR_CHAR_LIMIT];
	strsplit(buffer, ';',  5, split);
	printf("\nSUBSCRIBED UPDATE:");
	printf("%s %s %s %s %s\n", split[0], split[1], split[2], split[3],
		split[4]);
}


/*
Prints help menu.
*/
void help(){
	printf("\nTo list all the sensors connected to the broker with the type X:\n");
	printf("list X\n");
	printf("\nTo get the last readings in location X:\n");
	printf("last X\n");
	printf("To subscribe to location X:\n");
	printf("subscribe X\n");
}


int main(int argc, char *argv[]){


	int client_socket;
	int subscribe_socket;
	int this_socket; // socket on broker side
	int this_sub_socket;
	struct sockaddr_in serv_addr;
	struct hostent *server;
   
   
// create client socket
	client_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(client_socket < 0){
		perror("ERROR opening client socket");
		exit(1);
   	}

// create publish socket
	subscribe_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(subscribe_socket < 0){
		perror("ERROR opening publish socket");
		exit(1);
   	}
	

// setting up the broker to connect
   	server = gethostbyname(HOME);

   	if(server == NULL){ 
		fprintf(stderr,"ERROR, no such host\n");
		exit(1);
   	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,
		server->h_length);
	serv_addr.sin_port = htons(BROKER_PORT);

	
// connecting to broker
	if(connect(client_socket, (struct sockaddr*)&serv_addr,
		sizeof(serv_addr)) < 0){
		perror("ERROR client connecting");
		exit(2);
   	}

// initialize client on broker side
	initialize_client(&this_socket, client_socket);

   	if(connect(subscribe_socket, (struct sockaddr*)&serv_addr,
   		sizeof(serv_addr)) < 0){
		perror("ERROR publish connecting");
		exit(2);
   	}

// initialize subscribe on the broker side
	initialize_subscribe(&this_sub_socket, this_socket, subscribe_socket);

// connected


	char buffer[SENSOR_CHAR_LIMIT];
	bool flag = true;
	char authentication = 'C';

    printf("+ Connected to broker.\n");

	// file descriptors
	fd_set master, copy;
	FD_ZERO(&master);
	FD_ZERO(&copy);

	FD_SET(0, &master);
	FD_SET(subscribe_socket, &master);

	int current_socket;

    char command[COMAND_LIMIT];

	bool subscribed = true;

	help();

	// main loop
	while(flag){

		copy = master;

		if(select(subscribe_socket+1, &copy, NULL, NULL, NULL) == -1){
			perror("select:");
			exit(1);
		}

		for(current_socket = 0; current_socket <= subscribe_socket;
			current_socket++){

			// there's something new to read
			if(FD_ISSET(current_socket, &copy)){

				// if it's from the subscribe socket
				if(current_socket == subscribe_socket)
					print_new(subscribe_socket);

				// from stardard input
				else{
					memset(command, '\0', COMAND_LIMIT);
					scanf("%s", command);

					// authenticates
					send(client_socket, &authentication,
						sizeof(authentication), 0);

					if(strcmp(command, "list") == 0){
						scanf(" %[^\n]s", buffer);
						list_locations(client_socket, buffer);
					}

					else if(strcmp(command, "last") == 0){
						scanf(" %[^\n]s", buffer);
						last_reading(client_socket, buffer);
					}

					else if(strcmp(command, "subscribe") == 0){
						scanf(" %[^\n]s", buffer);
						if(subscribed)
							subscribe(client_socket, this_socket,
								buffer);
						else
							printf("Already subscribed.\n");
						subscribed = false;
					}

					else if(strcmp(command, "exit") == 0){
						flag = false;
					}

					else if(strcmp(command, "help") == 0)
						printf("Help\n");

					else
						printf("Command not recgonized\n");
				}
			}
		}	
	}


   	close(client_socket);
   	close(subscribe_socket);
	printf("+ Disconnected.\n");
   	return 0;
}
