#include <stdlib.h> 
#include <unistd.h>
#include <stdio.h>
#include <string.h> 
#include <stdbool.h>
#include <time.h>
#include <fcntl.h>
#include <netdb.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <sys/select.h>
#include <sys/types.h> 

#define h_addr h_addr_list[0]

#define SENSOR_INTREVAL 5
#define SENSOR_CHAR_LIMIT 51
#define MAX_SENSORS 100
#define MAX_CLIENTS 200
#define BROKER_PORT 2000
#define BUFF_SIZE 256
#define DATE_CHAR_LIMIT 51
#define HOME "localhost"
#define SENSOR_LOG_SIZE 10

#define SOCK_TO_INDEX(i) ((i) - 3)
#define INT_TO_CHAR(c) ((c) + '0')


/*---------------------------------------------------------------------------*/


struct sensor{
    int id;
    char type[SENSOR_CHAR_LIMIT];
    char location[SENSOR_CHAR_LIMIT];
    char version[SENSOR_CHAR_LIMIT];
};


struct sensor_message{
    int id;
    char date[DATE_CHAR_LIMIT];
    int value;
    char type[SENSOR_CHAR_LIMIT];
    char version[SENSOR_CHAR_LIMIT];
};


struct sensor_node{
    struct sensor* sensor;
    struct sensor_message* log[SENSOR_LOG_SIZE];
    int log_counter;
    int socket;
};


struct sensor_arrays{
    struct sensor_node* socket[MAX_SENSORS];
	struct sensor_node* id[MAX_SENSORS];
	struct sensor_node* type[MAX_SENSORS];
	struct sensor_node* location[MAX_SENSORS];
	struct sensor_node* version[MAX_SENSORS];
	int sensor_counter;
};

/*---------------------------------------------------------------------------*/


struct sensor* sensor_new(int id, char type[SENSOR_CHAR_LIMIT],
    char location[SENSOR_CHAR_LIMIT], char version[SENSOR_CHAR_LIMIT]);


struct sensor_message* sensor_message_new(int id, char date[DATE_CHAR_LIMIT],
    int value, char type[SENSOR_CHAR_LIMIT], char version[SENSOR_CHAR_LIMIT]);


struct sensor_node* sensor_node_new(struct sensor* sensor, int socket);


struct sensor_arrays* sensor_arrays_new();


/*---------------------------------------------------------------------------*/


void insert_message(struct sensor_message* new, struct sensor_node* node);


/*---------------------------------------------------------------------------*/


void sensor_arrays_insert(struct sensor_node* new, struct sensor_arrays* order);


/*---------------------------------------------------------------------------*/


struct public_cli{
    
};


/*---------------------------------------------------------------------------*/


void strdate(char *buffer, int len);


void strsplit(char original[BUFF_SIZE], int n, char result[n][SENSOR_CHAR_LIMIT]);