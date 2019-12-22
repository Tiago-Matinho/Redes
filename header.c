#include "header.h"



struct sensor* sensor_new(int id, char type[SENSOR_CHAR_LIMIT],
    char location[SENSOR_CHAR_LIMIT], char version[SENSOR_CHAR_LIMIT]){
    
    struct sensor* new = malloc(sizeof(struct sensor));

    if(new != NULL){
        memset(new->type, '\0', SENSOR_CHAR_LIMIT);
        memset(new->location, '\0', SENSOR_CHAR_LIMIT);
        memset(new->version, '\0', SENSOR_CHAR_LIMIT);

        new->id = id;
        strcpy(new->type, type);
        strcpy(new->location, location);
        strcpy(new->version, version);
    }

    return new;
}


struct sensor_message* sensor_message_new(int id, char date[DATE_CHAR_LIMIT],
    int value, char type[SENSOR_CHAR_LIMIT], char version[SENSOR_CHAR_LIMIT]){
    
    struct sensor_message* new = malloc(sizeof(struct sensor_message));

    if(new != NULL){
        memset(new->date, '\0', DATE_CHAR_LIMIT);
        memset(new->type, '\0', SENSOR_CHAR_LIMIT);
        memset(new->version, '\0', SENSOR_CHAR_LIMIT);

        new->id = id;
        strcpy(new->date, date);
        new->value = value;
        strcpy(new->type, type);
        strcpy(new->version, version);
    }

    return new;
}


struct sensor_node* sensor_node_new(struct sensor* sensor, int socket){

    struct sensor_node* new = malloc(sizeof(struct sensor_node));

    if(new != NULL){
        new->sensor = sensor;
        new->socket = socket;
        new->log_counter = 0;

        for(int i = 0; i < SENSOR_LOG_SIZE; i++)
            new->log[i] = NULL;
    }

    return new;
}


struct sensor_order_by* sensor_order_by_new(){
    
    struct sensor_order_by* new = malloc(sizeof(struct sensor_order_by));

    if(new != NULL){
        for(int i = 0; i < MAX_SENSORS; i++){
		new->socket[i] = NULL;
		new->id[i] = NULL;
		new->type[i] = NULL;
		new->location[i] = NULL;
		new->version[i] = NULL;
        }

        new->sensor_counter = 0;
    }

    return new;
}

/*---------------------------------------------------------------------------*/


void insert_message(struct sensor_message* new, struct sensor_node* node){
    if(new == NULL || node == NULL)
        return;

    free(node->log[SENSOR_LOG_SIZE - 1]);

    for(int i = SENSOR_LOG_SIZE - 1; i > 0; i--)
        node->log[i] = node->log[i - 1];

    node->log[0] = new;

    node->log_counter++;
}


/*---------------------------------------------------------------------------*/


void sensor_order_insert(struct sensor_node* new, struct sensor_order_by* order){
    order->sensor_counter++;

    int counter = order->sensor_counter;

    if(counter == MAX_SENSORS){
        printf("Too many sensors.\n");
        exit(1);
    }

    if(new != NULL){
        order->socket[SOCK_TO_INDEX(new->socket)];
        order->id[counter] = new;
        order->type[counter] = new;
        order->location[counter] = new;
        order->version[counter] = new;

        //TODO sort
    }
}


/*---------------------------------------------------------------------------*/


//passa a data atual para string (Retirado das aulas)
void strdate(char *buffer, int len){
    time_t now = time(NULL);
    struct tm *ptm = localtime(&now);
    
    if (ptm == NULL){  
        puts("The localtime() function failed");
        return;
    }

    strftime(buffer, len, "%c", ptm);
}


void strsplit(char original[BUFF_SIZE], int n, char result[n][SENSOR_CHAR_LIMIT]){
    char c;
    int split_counter = 0;
    int counter = 0;

    for(int k = 0; k < n; k++)
        memset(result[k], '\0', SENSOR_CHAR_LIMIT);
    

    for(int i = 0; i < BUFF_SIZE; i++){
        c = original[i];

        if(c == '\0'){
            return;
        }

        else if(c == ';'){
            split_counter++;
            counter = 0;
        }

        else{
            result[split_counter][counter] = c;
            counter++;
        }
    }
}