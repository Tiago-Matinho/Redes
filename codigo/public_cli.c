#include "header.h"


void initialize_client(int* this_socket, int client_socket){
	// initialize sensor on broker side
	char authentication = 'C';
	send(client_socket, &authentication, sizeof(authentication), 0);

	recv(client_socket, this_socket, sizeof(*this_socket), 0);
}


void initialize_subscribe(int* this_sub_socket, int this_socket, int publish_socket){
	// initialize sensor on broker side
	char authentication = 'P';
	send(publish_socket, &authentication, sizeof(authentication), 0);

	send(publish_socket, &this_socket, sizeof(this_socket), 0);

	recv(publish_socket, &this_sub_socket, sizeof(this_sub_socket), 0);
}


void list_locations(int socket, char type[SENSOR_CHAR_LIMIT]){
	char buffer[BUFF_SIZE];
	memset(buffer, '\0', BUFF_SIZE);

	// build message request
	strcat(buffer, "list;");
	strcat(buffer, type);

	// sends request
	send(socket, buffer, BUFF_SIZE, 0);

	// recieves number of packages that will have to recv
	int n = 0;
	recv(socket, &n, sizeof(n), 0);

	printf("\n\n+ Sensors found: %d\n", n);

	// recieve all the packages
	for(int i = 0; i < n; i++){
		recv(socket, buffer, SENSOR_CHAR_LIMIT, 0);
		printf("+ %s\n", buffer);
	}
	printf("\n");
}


void last_reading(int socket, char location[SENSOR_CHAR_LIMIT]){
	char buffer[BUFF_SIZE];
	char split[3][SENSOR_CHAR_LIMIT];
	memset(buffer, '\0', BUFF_SIZE);
	
	// build message request
	strcat(buffer, "last;");
	strcat(buffer, location);

	// sends request
	send(socket, buffer, BUFF_SIZE, 0);

	// recieves number of packages that will have to recv
	int n = 0;
	recv(socket, &n, sizeof(n), 0);

	printf("\n\n+ Sensors found: %d\n", n);
	printf("+ %s:\n", location);

	// recieve all the packages
	for(int i = 0; i < n; i++){

		// clear array
		for(int j = 0; j < 3; j++)
			memset(split[j], '\0', SENSOR_CHAR_LIMIT);

		// recieve message
		recv(socket, buffer, BUFF_SIZE, 0);

		strsplit(buffer, ';',  3, split);
		printf("- %s %s %s\n", split[0], split[1], split[2]);
	}
	printf("\n");
}


void subscribe(int client_socket, int this_socket, char location[SENSOR_CHAR_LIMIT]){
	char buffer[BUFF_SIZE];
	memset(buffer, '\0', BUFF_SIZE);
	
	// build message request
	strcat(buffer, "subscribe;");
	strcat(buffer, location);

	// send request
	send(client_socket, buffer, BUFF_SIZE, 0);
}


void print_new(int socket){
	char buffer[BUFF_SIZE];
	memset(buffer, '\0', BUFF_SIZE);

	// recieve message data
	recv(socket, buffer, BUFF_SIZE, 0);

	// split data
	char split[5][SENSOR_CHAR_LIMIT];
	strsplit(buffer, ';',  5, split);
	printf("\nSUBSCRIBED UPDATE:\n");
	printf("%s %s %s %s %s\n\n", split[0], split[1], split[2], split[3], split[4]);
}


int main(int argc, char *argv[]){


	int client_socket;
	int publish_socket;
	int this_socket; // socket on broker side
	int this_sub_socket;
	struct sockaddr_in serv_addr;
	struct hostent *server;
   
   
// create client socket
	client_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (client_socket < 0) {
		perror("ERROR opening client socket");
		exit(1);
   	}

// create publish socket
	publish_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (publish_socket < 0) {
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
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(BROKER_PORT);

	
// connecting to broker
	if(connect(client_socket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
		perror("ERROR client connecting");
		exit(2);
   	}

	initialize_client(&this_socket, client_socket);

   	if (connect(publish_socket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
		perror("ERROR publish connecting");
		exit(2);
   	}

	initialize_subscribe(&this_sub_socket, this_socket, publish_socket);

// connected


	char buffer[SENSOR_CHAR_LIMIT];
	bool flag = true;
	char authentication = 'C';

    printf("+ Connected to broker.\n");

	// file descriptors
	fd_set master, copy;
	FD_ZERO(&master);
	FD_ZERO(&copy);

	FD_SET(publish_socket, &master);

	struct timeval time;
	time.tv_sec = SENSOR_INTREVAL;
	time.tv_usec = 0;

    char command[COMAND_LIMIT];

	// main loop
	while(flag){

		copy = master;

		if(select(publish_socket+1, &copy, NULL, NULL, &time) == -1){
			perror("select:");
			exit(1);
		}

		// something new to read
		if(FD_ISSET(publish_socket, &copy))
			print_new(publish_socket);
		

		// send new data
		if(time.tv_sec == 0 && time.tv_usec == 0){	//FIXME
		
			memset(command, '\0', COMAND_LIMIT);
			scanf("%s", command);

			// authenticates
			send(client_socket, &authentication, sizeof(authentication), 0);

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
				subscribe(client_socket, this_socket, buffer);
			}

			else if(strcmp(command, "exit") == 0){
				flag = false;
			}

			else if(strcmp(command, "help") == 0)
				printf("Help\n");

			else
				printf("Command not recgonized\n");


			time.tv_sec = SENSOR_INTREVAL;
		}
	}


   	close(client_socket);
   	close(publish_socket);
	printf("+ Disconnected.\n");
   	return 0;
}
