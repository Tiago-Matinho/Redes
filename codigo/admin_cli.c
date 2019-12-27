#include "header.h"

void last_reading(int socket, char id[BUFF_SIZE]){
    char buffer[BUFF_SIZE];
    memset(buffer, '\0', BUFF_SIZE);

    // build message
    strcat(buffer, "R;");
    strcat(buffer, id);

    // send request
    send(socket, buffer, BUFF_SIZE, 0);

    memset(buffer, '\0', BUFF_SIZE);

    // recieves last reading
    recv(socket, buffer, BUFF_SIZE, 0);


    char split[5][SENSOR_CHAR_LIMIT];
    for(int i = 0; i < 5; i++)
        memset(split[i], '\0', SENSOR_CHAR_LIMIT);

    strsplit(buffer, 5, split);

    printf("ID\tDATE\t\t\t\tTYPE\tVALUE\tVERSION\n");
    printf("%s\t%s\t%s\t%s\t%s\n", split[0], split[1], split[2], split[3], split[4]);
    
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
	char authentication = 'A';
	send(server_socket, &authentication, sizeof(authentication), 0);

    char command;

	// main loop
	while(flag){
        scanf(" %c %[^\n]s", &command, buffer);

		// authenticates
		send(server_socket, &authentication, sizeof(authentication), 0);

        switch(command){

            case 'R':
			last_reading(server_socket, buffer);
            break;

            case 'L':
            //list_sensors(server_socket, buffer);
            break;

			//TODO Update firmware
            case 'U':
            break;

            //TODO desativar
            case 'D':
            break;

            default:
			break;
        }
	}


   	close(server_socket);
	printf("+ Disconnected.\n");
   	return 0;
}
