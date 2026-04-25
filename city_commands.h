#ifndef CITY_CORE_H
#define CITY_CORE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_NAME_LEN 50
#define MAX_CAT_LEN 30
#define MAX_DESC_LEN 152

//50 + 30 + 152 + 24 = 256 bytes/report

typedef struct {
    int id;                              //4 bytes
    char inspector_name[MAX_NAME_LEN];   
    float latitude;                      //4 bytes
    float longitude;                     //4 bytes
    char category[MAX_CAT_LEN];
    int severity;                        //4 bytes
    time_t timestamp;                    //8 bytes
    char description[MAX_DESC_LEN];
} Report;

typedef struct {
    char role[20];
    char user[MAX_NAME_LEN];
    char command[30];
    char district[50];
    char extra_arg[256];      //argument extra pentru comenzi precum filter
    int target_id;
} AppContext;
//organizam datele date in linia de comanda

void parse_arguments(int argc, char *argv[], AppContext *context);

#endif 