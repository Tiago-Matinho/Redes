#include "header.h"


void sensor_initialize(int sensor_socket, int update_socket, struct sensor* this_sensor){
	char sensor_authentication = 'S';
	
	// type of connection is from sensor
	send(sensor_socket, &sensor_authentication, sizeof(sensor_authentication), 0);

	// build the message
	char buffer[BUFF_SIZE];
	memset(buffer, '\0', BUFF_SIZE);

	strcat(buffer, this_sensor->id);
	strcat(buffer, ";");
	strcat(buffer, this_sensor->type);
	strcat(buffer, ";");
	strcat(buffer, this_sensor->location);
	strcat(buffer, ";");
	strcat(buffer, this_sensor->version);


	send(sensor_socket, buffer, BUFF_SIZE, 0);

	char accepted ='\0';

	// check if sensor ID is valid
	recv(sensor_socket, &accepted, 1, 0);

	// ID is not valid
	if(accepted == 'F'){
		printf("ID: %s already in use.\n", this_sensor->id);
		free(this_sensor); // release sensor from memory
		close(sensor_socket);
		close(update_socket);
		exit(1);
	}

	// ID is valid
	printf("Registed with success.\n");
}



void update_initialize(int update_socket, struct sensor* this_sensor){
	char update_authentication = 'U';
	
	// type of connection is from update
	send(update_socket, &update_authentication, sizeof(update_authentication), 0);

	// build the message
	char buffer[BUFF_SIZE];
	memset(buffer, '\0', BUFF_SIZE);

	strcat(buffer, this_sensor->id);
	strcat(buffer, ";");
	strcat(buffer, this_sensor->type);
	strcat(buffer, ";");
	strcat(buffer, this_sensor->location);
	strcat(buffer, ";");
	strcat(buffer, this_sensor->version);


	send(update_socket, buffer, BUFF_SIZE, 0);

	printf("Update socket set up.\n\n");
}


void sensor_send(int socket, struct sensor* this_sensor){
	char buffer[BUFF_SIZE];
    char date[DATE_CHAR_LIMIT];
	char value_c[SENSOR_CHAR_LIMIT];
	memset(buffer, '\0', BUFF_SIZE);
    memset(date, '\0', DATE_CHAR_LIMIT);
    memset(value_c, '\0', SENSOR_CHAR_LIMIT);

    // get new values
    int value = rand() % 101; // random
	snprintf(value_c, SENSOR_CHAR_LIMIT, "%d", value);
	strdate(date, DATE_CHAR_LIMIT);

    // build the message
	strcat(buffer, this_sensor->id);
    strcat(buffer, ";");
    strcat(buffer, date);
    strcat(buffer, ";");
    strcat(buffer, value_c);
    strcat(buffer, ";");
    strcat(buffer, this_sensor->type);
    strcat(buffer, ";");
    strcat(buffer, this_sensor->version);


    send(socket, buffer, BUFF_SIZE, 0);
	printf("+ %s data sent\n", date);
}


void update(int socket, struct sensor* this_sensor){
	char new_version[SENSOR_CHAR_LIMIT];
	memset(new_version, '\0', SENSOR_CHAR_LIMIT);
	
	int n;

	n = recv(socket, new_version, SENSOR_CHAR_LIMIT, 0);

	if(n <= 0){
		if (n == 0){
			printf("+ Disconected\n");
			close(socket);
		}

	}
	// check if new version is the latest
	if(strcmp(new_version, this_sensor->version) > 0){
		strcpy(this_sensor->version, new_version);
		printf("+ New update: %s\n", this_sensor->version);
	}

	// refuse update
	else
		printf("+ Update: %s refused. Sensor is on a newer version\n", new_version);
	
}


int main(int argc, char *argv[]){


	srand(time(NULL)); // used in fuction rand()


	// sensor must have ID, type, location and version
    if(argc < 5){
        printf("- Too few arguments.\n");
        exit(1);
    }

	// creates a new sensor
	struct sensor* this_sensor = sensor_new(argv[1], argv[2],
		argv[3], argv[4]);

	int sensor_socket;
	int update_socket;
	struct sockaddr_in serv_addr;
	struct hostent *server;
   
   
// create sensor socket
	sensor_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (sensor_socket < 0){
		perror("ERROR opening sensor socket");
		exit(1);
   	}

// create update socket
	update_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (update_socket < 0){
		perror("ERROR opening update socket");
		exit(1);
   	}

// setting up the broker to connect
   	server = gethostbyname(HOME);

   	if(server == NULL){ 
		fprintf(stderr,"ERROR no such host\n");
		exit(1);
   	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(BROKER_PORT);

	
// connecting to broker
	if(connect(sensor_socket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
		perror("ERROR sensor connecting");
		exit(2);
   	}

	// initialize sensor on broker side
	sensor_initialize(sensor_socket, update_socket, this_sensor);
    

	if(connect(update_socket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
		perror("ERROR update connecting");
		exit(2);
   	}

   	// initialize update
	update_initialize(update_socket, this_sensor);
	

// connected

	bool flag = true;
	char sensor_authentication = 'S';

    printf("+ Connected to broker.\n");


	// file descriptors
	fd_set master, copy;
	FD_ZERO(&master);
	FD_ZERO(&copy);

	FD_SET(update_socket, &master);

	struct timeval time;
	time.tv_sec = SENSOR_INTREVAL;
	time.tv_usec = 0;

	// main loop
	while(flag){

		copy = master;

		if(select(update_socket+1, &copy, NULL, NULL, &time) == -1){
			perror("select:");
			exit(1);
		}

		// something new to read
		if(FD_ISSET(update_socket, &copy))
			update(update_socket, this_sensor); //TEST
		

		// send new data
		if(time.tv_sec == 0 && time.tv_usec == 0){
			// authenticates
			send(sensor_socket, &sensor_authentication, sizeof(sensor_authentication), 0);
			sensor_send(sensor_socket, this_sensor);

			time.tv_sec = SENSOR_INTREVAL;
		}
		
		
	}


	printf("+ Disconnected.\n");
   	close(sensor_socket);
   	close(update_socket);
	free(this_sensor);
   	return 0;
}
