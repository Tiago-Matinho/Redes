#include "header.h"

#define MAX_SIZE 101
#define LOG_SIZE 10

struct sensor_node{
    struct sensor* sensor;
    struct sensor_payload* log[LOG_SIZE];
};

struct hashtable{
    int load;
    struct sensor_node* array[MAX_SIZE];
};


struct sensor_node* new_node(struct sensor* sensor);
void sensor_node_print(struct sensor_node* sensor);

struct hashtable* new_hashtable();
bool hash_insert(struct hashtable* table, struct sensor_node* new_node);
struct sensor_node* hash_get(struct hashtable* table, int id);
void add_payload(struct hashtable* table, struct sensor_payload* payload);

void hash_destroy(struct hashtable* hashtable);