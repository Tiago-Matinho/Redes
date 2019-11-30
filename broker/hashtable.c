#include "hashtable.h"


struct sensor_node* new_node(struct sensor* sensor){
    
    struct sensor_node* new = malloc(sizeof(struct sensor_node));

    if(new != NULL){
        new->sensor = sensor;
        for(int i = 0; i < LOG_SIZE; i++)
            new->log[i] = NULL;
    }
    return new;
}

struct hashtable* new_hashtable(){

    struct hashtable* new = malloc(sizeof(struct hashtable));

    if(new != NULL){
        new->load = 0;

        for(int i = 0; i < MAX_SIZE; i++)
            new->array[i] = NULL;
    }
    return new;
}


void sensor_node_print(struct sensor_node* sensor){
    if(sensor == NULL)
        return;

    sensor_print(sensor->sensor);
    
    for(int i = 0; i < LOG_SIZE; i++)
        sensor_payload_print(sensor->log[i]);

    printf("\n");
}


void hash_insert(struct hashtable* table, struct sensor_node* new_node){
    int key = new_node->sensor->id;

    if(key < 0)
        exit(1);

    if(table->load + 1 == MAX_SIZE)
        exit(1);

    key %= MAX_SIZE;

    while(table->array[key] != NULL){
        if(table->array[key]->sensor->id == new_node->sensor->id){
            printf("Already saved.\n");
            exit(1);
        }

        key++;
        key %= MAX_SIZE;
    }

    table->array[key] = new_node;    
}


struct sensor_node* hash_get(struct hashtable* table, int id){
    int key = id;

    if(key < 0)
        exit(1);

    key %= MAX_SIZE;

    while(table->array[key] != NULL){
        if(table->array[key]->sensor->id == id)
            return table->array[key];
        

        key++;
        key %= MAX_SIZE;
    }

    return NULL;
}


void hash_destroy(struct hashtable* hashtable){
    if(hashtable == NULL)
        return;

    for(int i = 0; i < MAX_SIZE; i++){
        if(hashtable->array[i] != NULL){
            free(hashtable->array[i]->sensor);

            for(int j = 0; j < LOG_SIZE; j++){
                if(hashtable->array[i]->log[j] != NULL)
                    free(hashtable->array[i]->log[j]);
            }
            
            free(hashtable->array[i]);
        }
    }
    free(hashtable);
}