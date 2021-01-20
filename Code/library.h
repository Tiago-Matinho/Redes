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

#define SENSOR_INTREVAL 10
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


/*-------------------------------------------------------------------*/


// holds all the sensors information
struct sensor{
    char id[SENSOR_CHAR_LIMIT];
    char type[SENSOR_CHAR_LIMIT];
    char location[SENSOR_CHAR_LIMIT];
    char version[SENSOR_CHAR_LIMIT];
};


// holds all the reading information sent from the sensors
struct sensor_message{
    char id[SENSOR_CHAR_LIMIT];
    char date[DATE_CHAR_LIMIT];
    char value[VALUE_CHAR_LIMIT];
    char type[SENSOR_CHAR_LIMIT];
    char version[SENSOR_CHAR_LIMIT];
};


/*
saves all the information from the sensors, theirs messages
and their subscribed public clients
*/
struct sensor_node{
    struct sensor* sensor;
    struct sensor_message* log[SENSOR_LOG_SIZE];
    struct public_cli* subs[MAX_PUB_CLI];
    int log_counter;
    int sensor_socket;
    int update_socket;
    int subs_counter;
};


// holds all the sensor nodes in order. Also holds the blocked sensors.
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

/*-------------------------------------------------------------------*/


// creates a new sensor
struct sensor* sensor_new(char id[SENSOR_CHAR_LIMIT],
    char type[SENSOR_CHAR_LIMIT], char location[SENSOR_CHAR_LIMIT],
    char version[SENSOR_CHAR_LIMIT]);


// creates a new reading message sent from the sensors
struct sensor_message* sensor_message_new(char id[SENSOR_CHAR_LIMIT],
    char date[DATE_CHAR_LIMIT], char value[VALUE_CHAR_LIMIT],
    char type[SENSOR_CHAR_LIMIT], char version[SENSOR_CHAR_LIMIT]);

// creates a new sensor node
struct sensor_node* sensor_node_new(struct sensor* sensor,
    int sensor_socket);

// creates the sensor arrays
struct sensor_arrays* sensor_arrays_new();


/*-------------------------------------------------------------------*/

// insert the latest reading into the sensor node
void insert_message(struct sensor_message* new,
    struct sensor_node* node);

// adds the new subscriber to the sensor node
void sub_sensor(struct sensor_node* sensor, struct public_cli* client);


/*-------------------------------------------------------------------*/

// adds new sensor nodes to the sensor arrays
void sensor_arrays_insert(struct sensor_node* new,
    struct sensor_arrays* arrays);

// removes an already existing sensor and adds it to the blocked list
struct sensor_node* sensor_arrays_remove(struct sensor_arrays* arrays,
    char id[SENSOR_CHAR_LIMIT]);

// returns the sensor wich id is the requested. Returns NULL if not found
struct sensor_node* id_search(struct sensor_node* array[MAX_SENSORS],
    int last, char id[SENSOR_CHAR_LIMIT]);


/*-------------------------------------------------------------------*/

// links the public client to it's subscribe socket
struct public_cli{
    int socket;
    int subscribe_socket;
};

// holds the clients with their information
struct pub_clients{
    struct public_cli* array[MAX_PUB_CLI];
};


// creates a new client 
struct public_cli* public_cli_new(int socket);

// creates a new client array
struct pub_clients* pub_clients_new();

// adds a new client to the array
void pub_clients_add(struct pub_clients* array, struct public_cli* new);

// removes a client from the array
void pub_clients_remove(struct pub_clients* array, int socket);

// returns a client from the array (uses socket as an index)
struct public_cli* pub_clients_get(struct pub_clients* array,
    int socket);


/*-------------------------------------------------------------------*/

// returns a date in string format
void strdate(char *buffer, int len);

// splits a string delimited by c
void strsplit(char original[BUFF_SIZE], char c, int n,
    char result[n][SENSOR_CHAR_LIMIT]);


/*-------------------------------------------------------------------*/

// frees all the sensors from memory
void free_sensor_arrays(struct sensor_arrays* arrays);