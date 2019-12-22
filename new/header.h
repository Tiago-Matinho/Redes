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
#define SENSOR_CHAR_LIMIT 11
#define BROKER_PORT 2000
#define BUFF_SIZE 256
#define DATE_CHAR_LIMIT 21
#define HOME "localhost"



struct sensor{
    int id;
    char type[SENSOR_CHAR_LIMIT];
    char location[SENSOR_CHAR_LIMIT];
    char version[SENSOR_CHAR_LIMIT];
};


struct sensor* sensor_new(int id, char type[SENSOR_CHAR_LIMIT],
    char location[SENSOR_CHAR_LIMIT], char version[SENSOR_CHAR_LIMIT]);



void strdate(char *buffer, int len);


void strsplit(char original[BUFF_SIZE], int n, char result[n][SENSOR_CHAR_LIMIT]);