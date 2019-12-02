#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <netdb.h>
#include <netinet/in.h>
#include <time.h>
#include <pthread.h>

#define MAXCHAR 255
#define MAX_UPDATE_SIZE 3000

#define h_addr h_addr_list[0]

struct sensor{
    int id;
    char tipo [MAXCHAR];
    char local [MAXCHAR];
    char versao [MAXCHAR];
};


struct sensor_payload{
    int id;
    char data[MAXCHAR];
    int valor;
    char tipo[MAXCHAR];
    char versao [MAXCHAR];
};


struct sensor* new_sensor(int id, char tipo[MAXCHAR],
    char local[MAXCHAR], char versao[MAXCHAR]);


struct sensor_payload* new_sen_payload(int id, char data[MAXCHAR],
    int valor, char tipo[MAXCHAR], char versao [MAXCHAR]);


void sensor_print(struct sensor* sensor);
void sensor_payload_print(struct sensor_payload* sensor_payload);