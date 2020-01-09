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
#define SENSOR_CHAR_LIMIT 40
#define DATE_CHAR_LIMIT 50
#define VALUE_CHAR_LIMIT 12
#define SENSOR_LOG_SIZE 10
#define MAX_SENSORS 100
#define MAX_PUB_CLI 200
#define BUFF_SIZE 256
#define COMAND_LIMIT 20
#define BROKER_PORT 2000
#define HOME "localhost"


/*---------------------------------------------------------------------------*/


struct sensor{
    char id[SENSOR_CHAR_LIMIT];
    char type[SENSOR_CHAR_LIMIT];
    char location[SENSOR_CHAR_LIMIT];
    char version[SENSOR_CHAR_LIMIT];
};


struct sensor_message{
    char id[SENSOR_CHAR_LIMIT];
    char date[DATE_CHAR_LIMIT];
    char value[VALUE_CHAR_LIMIT];
    char type[SENSOR_CHAR_LIMIT];
    char version[SENSOR_CHAR_LIMIT];
};


struct sensor_node{
    struct sensor* sensor;
    struct sensor_message* log[SENSOR_LOG_SIZE];
    struct public_cli* subs[MAX_PUB_CLI];
    int log_counter;
    int sensor_socket;
    int update_socket;
    int subs_counter;
};


struct sensor_arrays{
    struct sensor_node* socket[MAX_SENSORS];
	struct sensor_node* id[MAX_SENSORS];
	struct sensor_node* type[MAX_SENSORS];
	struct sensor_node* location[MAX_SENSORS];
	struct sensor_node* version[MAX_SENSORS];
	struct sensor_node* blocked[MAX_SENSORS];
	int sensor_counter;
    int blocked_counter;
};

/*---------------------------------------------------------------------------*/


struct sensor* sensor_new(char id[SENSOR_CHAR_LIMIT], char type[SENSOR_CHAR_LIMIT],
    char location[SENSOR_CHAR_LIMIT], char version[SENSOR_CHAR_LIMIT]);


struct sensor_message* sensor_message_new(char id[SENSOR_CHAR_LIMIT],
    char date[DATE_CHAR_LIMIT], char value[VALUE_CHAR_LIMIT], char type[SENSOR_CHAR_LIMIT],
    char version[SENSOR_CHAR_LIMIT]);


struct sensor_node* sensor_node_new(struct sensor* sensor, int sensor_socket);


struct sensor_arrays* sensor_arrays_new();


/*---------------------------------------------------------------------------*/


void insert_message(struct sensor_message* new, struct sensor_node* node);


void sub_sensor(struct sensor_node* sensor, struct public_cli* client);


/*---------------------------------------------------------------------------*/


void sensor_arrays_insert(struct sensor_node* new, struct sensor_arrays* arrays);


struct sensor_node* sensor_arrays_remove(struct sensor_arrays* arrays, char id[SENSOR_CHAR_LIMIT]);


struct sensor_node* id_search(struct sensor_node* array[MAX_SENSORS], int last,
    char id[SENSOR_CHAR_LIMIT]);


/*---------------------------------------------------------------------------*/


struct public_cli{
    int socket;
    int subscribe_socket;
};


struct pub_clients{
    struct public_cli* array[MAX_PUB_CLI];
};


struct public_cli* public_cli_new(int socket);


struct pub_clients* pub_clients_new();


void pub_clients_add(struct pub_clients* array, struct public_cli* new);


void pub_clients_remove(struct pub_clients* array, int socket);


struct public_cli* pub_clients_get(struct pub_clients* array, int socket);


/*---------------------------------------------------------------------------*/


void strdate(char *buffer, int len);


void strsplit(char original[BUFF_SIZE], char c, int n, char result[n][SENSOR_CHAR_LIMIT]);