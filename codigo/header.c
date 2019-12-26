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


struct sensor_arrays* sensor_arrays_new(){

    struct sensor_arrays* new = malloc(sizeof(struct sensor_arrays));

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


bool compare_merge(struct sensor_node* node1, struct sensor_node* node2, char c){
    switch(c){
        case 'I':
            return node1->sensor->id <= node2->sensor->id;


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
void merge(struct sensor_node* arr[], int l, int m, int r, char c)
{
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
void mergeSort(struct sensor_node* arr[], int l, int r, char c)
{
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



void sensor_arrays_insert(struct sensor_node* new, struct sensor_arrays* order){

    int counter = order->sensor_counter;

    if(counter + 1 == MAX_SENSORS){
        printf("Too many sensors.\n");
        exit(1);
    }

    if(new != NULL){
        order->socket[new->socket] = new;
        order->id[counter] = new;
        order->type[counter] = new;
        order->location[counter] = new;
        order->version[counter] = new;
    }


    mergeSort(order->id, 0, counter, 'I');
    mergeSort(order->type, 0, counter, 'T');
    mergeSort(order->location, 0, counter, 'L');
    mergeSort(order->version, 0, counter, 'V');
    order->sensor_counter++;
}


/*---------------------------------------------------------------------------*/




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
