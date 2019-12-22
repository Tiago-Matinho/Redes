#include "header.h"


void list_type(int socket){

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

	char buffer[BUFF_SIZE];
	bool flag = true;

    printf("+ Connected to broker.\n");

	// initialize sensor on broker side
	char authentication = 'C';
	send(server_socket, authentication, 1, 0);

    char command;

	// main loop
	while(flag){
        scanf(" %c %[^\n]s", &command, buffer);


        switch(command){

            case 'T':
            list_type(server_socket);
            break;

            case 'L':
            break;

            case 'P':
            break;

            default:
        }
	}


   	close(server_socket);
	printf("+ Disconnected.\n");
   	return 0;
}
