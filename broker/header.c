#include "header.h"


struct sensor* new_sensor(int id, char tipo[MAXCHAR],
    char local[MAXCHAR], char versao[MAXCHAR]){
    
    struct sensor* new = malloc(sizeof(struct sensor));

    if(new != NULL){
        new->id = id;
        memset(new->tipo, '\0', MAXCHAR);
        memset(new->local, '\0', MAXCHAR);
        memset(new->versao, '\0', MAXCHAR);
        strcpy(new->tipo, tipo);
        strcpy(new->local, local);
        strcpy(new->versao, versao);
    }
    return new;
}


struct sensor_payload* new_sen_payload(int id, char data[MAXCHAR],
    int valor, char tipo[MAXCHAR], char versao [MAXCHAR]){

    struct sensor_payload* new = malloc(sizeof(struct sensor_payload));

    if(new != NULL){
        new->id = id;
        new->valor = valor;
        memset(new->data, '\0', MAXCHAR);
        memset(new->tipo, '\0', MAXCHAR);
        memset(new->versao, '\0', MAXCHAR);
        strcpy(new->data, data);
        strcpy(new->tipo, tipo);
        strcpy(new->versao, versao);
    }
    return new;
}


void sensor_print(struct sensor* sensor){
    if(sensor == NULL)
        return;

    printf("%d: %s %s %s.\n", sensor->id, sensor->tipo,
        sensor->local, sensor->versao);
}


void sensor_payload_print(struct sensor_payload* sensor_payload){
    if(sensor_payload == NULL)
        return;

    printf("%s %d: %s %d %s.\n", sensor_payload->data, sensor_payload->id,
        sensor_payload->tipo, sensor_payload->valor, sensor_payload->versao);
}