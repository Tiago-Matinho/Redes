#include "header.h"


void sensor_initialize(int socket, struct sensor* this_sensor){
	char buffer[BUFF_SIZE];
	memset(buffer, '\0', BUFF_SIZE);

	// build the message
	snprintf(buffer, BUFF_SIZE, "%d", this_sensor->id);
	strcat(buffer, ";");
	strcat(buffer, this_sensor->type);
	strcat(buffer, ";");
	strcat(buffer, this_sensor->location);
	strcat(buffer, ";");
	strcat(buffer, this_sensor->version);


	send(socket, buffer, strlen(buffer), 0);
}


void sensor_send(int socket, struct sensor* this_sensor){
	char buffer[BUFF_SIZE];
    char date[DATE_CHAR_LIMIT];
	char value_c[SENSOR_CHAR_LIMIT];
	memset(buffer, '\0', BUFF_SIZE);
    memset(date, '\0', DATE_CHAR_LIMIT);
    memset(value_c, '\0', SENSOR_CHAR_LIMIT);

    int value = rand() % 101; // random
	snprintf(value_c, SENSOR_CHAR_LIMIT, "%d", value);
	strdate(date, DATE_CHAR_LIMIT);

    // build the message
	snprintf(buffer, BUFF_SIZE, "%d", this_sensor->id);
    strcat(buffer, ";");
    strcat(buffer, date);
    strcat(buffer, ";");
    strcat(buffer, value_c);
    strcat(buffer, ";");
    strcat(buffer, this_sensor->type);
    strcat(buffer, ";");
    strcat(buffer, this_sensor->version);


    send(socket, buffer, BUFF_SIZE, 0);
	printf("%s data sent\n", date);
}

int main(int argc, char *argv[]){


	srand(time(NULL)); // used in fuction rand()


	// sensor must have ID, type, location and version
    if(argc < 5){
        printf("Too few arguments.\n");
        exit(1);
    }

	// creates a new sensor
	struct sensor* this_sensor = sensor_new(atoi(argv[1]), argv[2],
		argv[3], argv[4]);

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
	char authentication = 'S';
	send(server_socket, authentication, 1, 0);
	sensor_initialize(server_socket, this_sensor);

	int pid = fork();

	// main loop
	while(flag){
		// send new data
		if(pid == 0){
			sleep(SENSOR_INTREVAL);
			sensor_send(server_socket, this_sensor);
		}
		
		//FIXME
		else{
			recv(server_socket, buffer, BUFF_SIZE, 0);
			strcpy(this_sensor->version, buffer);
		}
	}


   	close(server_socket);
	free(this_sensor);
	printf("+ Disconnected.\n");
   	return 0;
}
