#include "header.h"


void list_locations(int socket, char type[BUFF_SIZE]){
	char buffer[BUFF_SIZE];
	memset(buffer, '\0', BUFF_SIZE);

	strcat(buffer, "T;");
	strcat(buffer, type);


	// sends request
	send(socket, buffer, BUFF_SIZE, 0);

	printf("%s\n", buffer);
	// recieves number of packages will have to recv
	int n = 0;
	//TODO
	recv(socket, &n, sizeof(n), 0);

	printf("pacotes a ler: %d\n", n);

	for(int i = 0; i < n; i++){
		recv(socket, buffer, BUFF_SIZE, 0);
		printf("%s\n", buffer);
	}
}


void last_reading(int socket, char location[SENSOR_CHAR_LIMIT]){
	char buffer[BUFF_SIZE];
	memset(buffer, '\0', BUFF_SIZE);

	strcat(buffer, "L;");
	strcat(buffer, location);

	// sends request
	send(socket, buffer, BUFF_SIZE, 0);

	recv(socket, buffer, BUFF_SIZE, 0);
	printf("%s\n", buffer);
}


int main(int argc, char *argv[]){


	int server_socket;
	struct sockaddr_in serv_addr;
	struct hostent *server;
   
   
   // create a TCP socket point
	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket < 0) {
		perror("ERROR opening socket");
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
	if (connect(server_socket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
		perror("ERROR connecting");
		exit(2);
   	}
    

	// connected

	char buffer[SENSOR_CHAR_LIMIT];
	bool flag = true;

    printf("+ Connected to broker.\n");

	// initialize sensor on broker side
	char authentication = 'C';
	send(server_socket, &authentication, sizeof(authentication), 0);

    char command;

	// main loop
	while(flag){
        scanf(" %c %[^\n]s", &command, buffer);


        switch(command){

            case 'T':
            list_locations(server_socket, buffer);
            break;

            case 'L':
			last_reading(server_socket, buffer);
            break;

            case 'P':
            break;

            default:
			break;
        }
	}


   	close(server_socket);
	printf("+ Disconnected.\n");
   	return 0;
}
