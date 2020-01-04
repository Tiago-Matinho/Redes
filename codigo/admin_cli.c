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

    strsplit(buffer, ';', 5, split);

    printf("ID\tDATE\t\t\t\tTYPE\tVALUE\tVERSION\n");
    printf("%s\t%s\t%s\t%s\t%s\n", split[0], split[1], split[2], split[3], split[4]);
    
}


void list_all_sensors(int socket){
	char buffer[BUFF_SIZE];
	char split[4][SENSOR_CHAR_LIMIT];
    memset(buffer, '\0', BUFF_SIZE);

    // build message
    strcat(buffer, "L;N");

    // send request
    send(socket, buffer, BUFF_SIZE, 0);

	int counter = 0;
    // recieves number of sensors
    recv(socket, &counter, sizeof(counter), 0);

	printf("ID\tTYPE\tLOCATION\tVERSION\n");

	for(int i = 0; i < counter; i++){
    	memset(buffer, '\0', BUFF_SIZE);
		recv(socket, buffer, BUFF_SIZE, 0);

		for(int k = 0; k < 4; k++)
			memset(split[k], '\0', SENSOR_CHAR_LIMIT);

		strsplit(buffer, ';', 4, split);

		printf("%s\t%s\t%s\t\t%s\n", split[0], split[1], split[2], split[3]);
	}
}


void update_sensor(int socket, char buffer[BUFF_SIZE]){
	char split[2][SENSOR_CHAR_LIMIT];
	for(int i = 0; i < 2; i++)
		memset(split[i], '\0', SENSOR_CHAR_LIMIT);

	strsplit(buffer, ' ', 2, split);

	// build the message
	memset(buffer, '\0', BUFF_SIZE);

	strcat(buffer, "U;");
	strcat(buffer, split[0]);
	strcat(buffer, ";");
	strcat(buffer, split[1]);

	// send update
	send(socket, buffer, BUFF_SIZE, 0);

	printf("Update sent.\n");
}


void desactivate(int socket, char id[SENSOR_CHAR_LIMIT]){
	char buffer[BUFF_SIZE];
    memset(buffer, '\0', BUFF_SIZE);

    // build message
    strcat(buffer, "D;");
    strcat(buffer, id);

	// send request
    send(socket, buffer, BUFF_SIZE, 0);
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
		//FIXME
        scanf(" %c %[^\n]s", &command, buffer);

		// authenticates
		send(server_socket, &authentication, sizeof(authentication), 0);

        switch(command){

            case 'R':
			last_reading(server_socket, buffer);
            break;

            case 'L':
            list_all_sensors(server_socket);
            break;

            case 'U':
			update_sensor(server_socket, buffer);
            break;

            case 'D':
			desactivate(server_socket, buffer);
            break;
			
			case 'E':
			flag = false;
			break;

            default:
			break;
        }
	}


   	close(server_socket);
	printf("+ Disconnected.\n");
   	return 0;
}
