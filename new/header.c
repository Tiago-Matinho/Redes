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