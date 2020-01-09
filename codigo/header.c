#include "header.h"



struct sensor* sensor_new(char id[SENSOR_CHAR_LIMIT], char type[SENSOR_CHAR_LIMIT],
    char location[SENSOR_CHAR_LIMIT], char version[SENSOR_CHAR_LIMIT]){

    struct sensor* new = malloc(sizeof(struct sensor));

    if(new != NULL){
        memset(new->id, '\0', SENSOR_CHAR_LIMIT);
        memset(new->type, '\0', SENSOR_CHAR_LIMIT);
        memset(new->location, '\0', SENSOR_CHAR_LIMIT);
        memset(new->version, '\0', SENSOR_CHAR_LIMIT);

        strcpy(new->id, id);
        strcpy(new->type, type);
        strcpy(new->location, location);
        strcpy(new->version, version);
    }

    return new;
}


struct sensor_message* sensor_message_new(char id[SENSOR_CHAR_LIMIT],
    char date[DATE_CHAR_LIMIT], char value[VALUE_CHAR_LIMIT], char type[SENSOR_CHAR_LIMIT],
    char version[SENSOR_CHAR_LIMIT]){

    struct sensor_message* new = malloc(sizeof(struct sensor_message));

    if(new != NULL){
        memset(new->id, '\0', SENSOR_CHAR_LIMIT);
        memset(new->date, '\0', DATE_CHAR_LIMIT);
        memset(new->value, '\0', VALUE_CHAR_LIMIT);
        memset(new->type, '\0', SENSOR_CHAR_LIMIT);
        memset(new->version, '\0', SENSOR_CHAR_LIMIT);

        strcpy(new->id, id);
        strcpy(new->date, date);
        strcpy(new->value, value);
        strcpy(new->type, type);
        strcpy(new->version, version);
    }

    return new;
}


struct sensor_node* sensor_node_new(struct sensor* sensor, int sensor_socket){

    struct sensor_node* new = malloc(sizeof(struct sensor_node));

    if(new != NULL){
        new->sensor = sensor;
        new->sensor_socket = sensor_socket;
        new->log_counter = 0;
        new->subs_counter = 0;

        for(int i = 0; i < SENSOR_LOG_SIZE; i++)
            new->log[i] = NULL;

        for(int k = 0; k < MAX_PUB_CLI; k++)
            new->subs[k] = NULL;
    }

    return new;
}


struct sensor_arrays* sensor_arrays_new(){

    struct sensor_arrays* new = malloc(sizeof(struct sensor_arrays));

    if(new != NULL){
        for(int i = 0; i < MAX_SENSORS; i++){
            new->socket[i] = NULL;
            new->id[i] = NULL;
            new->type[i] = NULL;
            new->location[i] = NULL;
            new->version[i] = NULL;
            new->blocked[i] = NULL;
        }

        new->sensor_counter = 0;
        new->blocked_counter = 0;
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


void sub_sensor(struct sensor_node* sensor, struct public_cli* client){
    sensor->subs[sensor->subs_counter] = client;
    sensor->subs_counter++;
}


/*---------------------------------------------------------------------------*/


bool compare_merge(struct sensor_node* node1, struct sensor_node* node2, char c){
    switch(c){
        case 'I':
            return strcmp(node1->sensor->id, node2->sensor->id) <= 0;


        case 'T':
            return strcmp(node1->sensor->type, node2->sensor->type) <= 0;


        case 'L':
            return strcmp(node1->sensor->location, node2->sensor->location) <= 0;


        case 'V':
            return strcmp(node1->sensor->version, node2->sensor->version) <= 0;

        default:
            break;
    }
    return false;
}

//https://www.geeksforgeeks.org/c-program-for-merge-sort/
// Merges two subarrays of arr[].
// First subarray is arr[l..m]
// Second subarray is arr[m+1..r]
void merge(struct sensor_node* arr[], int l, int m, int r, char c){
    int i, j, k;
    int n1 = m - l + 1;
    int n2 =  r - m;

    /* create temp arrays */
    struct sensor_node* L[n1];
    struct sensor_node* R[n2];

    /* Copy data to temp arrays L[] and R[] */
    for (i = 0; i < n1; i++)
        L[i] = arr[l + i];
    for (j = 0; j < n2; j++)
        R[j] = arr[m + 1+ j];

    /* Merge the temp arrays back into arr[l..r]*/
    i = 0; // Initial index of first subarray
    j = 0; // Initial index of second subarray
    k = l; // Initial index of merged subarray
    while (i < n1 && j < n2)
    {
        if (compare_merge(L[i], R[j], c))
        {
            arr[k] = L[i];
            i++;
        }
        else
        {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    /* Copy the remaining elements of L[], if there
       are any */
    while (i < n1)
    {
        arr[k] = L[i];
        i++;
        k++;
    }

    /* Copy the remaining elements of R[], if there
       are any */
    while (j < n2)
    {
        arr[k] = R[j];
        j++;
        k++;
    }
}


/* l is for left index and r is right index of the
   sub-array of arr to be sorted */
void mergeSort(struct sensor_node* arr[], int l, int r, char c){
    if (l < r)
    {
        // Same as (l+r)/2, but avoids overflow for
        // large l and h
        int m = l+(r-l)/2;

        // Sort first and second halves
        mergeSort(arr, l, m, c);
        mergeSort(arr, m+1, r, c);

        merge(arr, l, m, r, c);
    }
}


void sensor_arrays_insert(struct sensor_node* new, struct sensor_arrays* arrays){

    int counter = arrays->sensor_counter;

    if(counter + 1 == MAX_SENSORS){
        printf("Too many sensors.\n");
        exit(1);
    }

    if(new != NULL){
        arrays->socket[new->sensor_socket] = new;
        arrays->id[counter] = new;
        arrays->type[counter] = new;
        arrays->location[counter] = new;
        arrays->version[counter] = new;
    }


    mergeSort(arrays->id, 0, counter, 'I');
    mergeSort(arrays->type, 0, counter, 'T');
    mergeSort(arrays->location, 0, counter, 'L');
    mergeSort(arrays->version, 0, counter, 'V');
    arrays->sensor_counter++;
}


struct sensor_node* sensor_arrays_remove(struct sensor_arrays* arrays, char id[SENSOR_CHAR_LIMIT]){
    struct sensor_node* temp = NULL;
    int counter = arrays->sensor_counter;


    for(int i = 0; i < counter; i++){

        if(strcmp(id, arrays->id[i]->sensor->id) == 0){
            temp = arrays->id[i];
            arrays->id[i] = arrays->id[counter - 1];
        }

        if(strcmp(id, arrays->location[i]->sensor->id) == 0)
            arrays->location[i] = arrays->location[counter - 1];
        

        if(strcmp(id, arrays->type[i]->sensor->id) == 0)
            arrays->type[i] = arrays->type[counter - 1];
        

        if(strcmp(id, arrays->version[i]->sensor->id) == 0)
            arrays->version[i] = arrays->version[counter - 1];
        
    }

    arrays->blocked[arrays->blocked_counter] = temp;
    arrays->sensor_counter--;
    counter = arrays->sensor_counter;

    mergeSort(arrays->id, 0, counter, 'I');
    mergeSort(arrays->type, 0, counter, 'T');
    mergeSort(arrays->location, 0, counter, 'L');
    mergeSort(arrays->version, 0, counter, 'V');

    for(int j = 0; j < SENSOR_LOG_SIZE; j++)
        free(temp->log[j]);

    mergeSort(arrays->blocked, 0, arrays->blocked_counter, 'I');
    arrays->blocked_counter++;

    return temp;
}


struct sensor_node* id_search(struct sensor_node* array[MAX_SENSORS], int last,
    char id[SENSOR_CHAR_LIMIT]){
    
    for(int i = 0; i < last; i++){
        if(array[i] != NULL){
            if(strcmp(id, array[i]->sensor->id) == 0)
                return array[i];
        }
    }
    return NULL;

    //binary search FIXME
    /*
    int l, r, m;

    if(last == 0)
        return NULL;

    l = 0;
    r = last - 1;

    while(l <= r){
        m = l + (r - l) / 2;

        if(array[m] != NULL){
            if(strcmp(array[m]->sensor->id, id) == 0)
                return array[m];

            if(strcmp(array[m]->sensor->id, id) < 0)
                l = m + 1;
            
            else
                r = m - 1;
        }
    }
    return NULL;
    */
}


/*---------------------------------------------------------------------------*/


struct public_cli* public_cli_new(int socket){
    struct public_cli* new = malloc(sizeof(struct public_cli));

    if(new != NULL){
        new->socket = socket;
        new->subscribe_socket = -1;
    }
    return new;
}


struct pub_clients* pub_clients_new(){
    struct pub_clients* new = malloc(sizeof(struct pub_clients));

    if(new != NULL)
        for(int i = 0; i < MAX_PUB_CLI; i++)
            new->array[i] = NULL;
    
    return new;
}


void pub_clients_add(struct pub_clients* array, struct public_cli* new){
    if(new == NULL || array == NULL)
        return;

    // saves client on respectives socket
    array->array[new->socket] = new;
}


void pub_clients_remove(struct pub_clients* array, int socket){
    if(array == NULL)
        return;

    if(array->array[socket] != NULL)
        free(array->array[socket]);
}


struct public_cli* pub_clients_get(struct pub_clients* array, int socket){
    if(array == NULL)
        return NULL;

    return array->array[socket];
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


void strsplit(char original[BUFF_SIZE], char c, int n, char result[n][SENSOR_CHAR_LIMIT]){
    char cur;
    int split_counter = 0;
    int counter = 0;

    for(int k = 0; k < n; k++)
        memset(result[k], '\0', SENSOR_CHAR_LIMIT);


    for(int i = 0; i < BUFF_SIZE; i++){
        cur = original[i];

        if(cur == '\0'){
            return;
        }

        else if(cur == c){
            split_counter++;
            counter = 0;
        }

        else{
            result[split_counter][counter] = cur;
            counter++;
        }
    }
}
