#ifndef CITY_CORE_H
#define CITY_CORE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_NAME_LEN 50
#define MAX_CAT_LEN 30
#define MAX_DESC_LEN 152



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

void mode_to_string(mode_t mode, char *str);
void parse_arguments(int argc, char *argv[], AppContext *context);
int setup_and_log_action(AppContext *context);
int has_permission(mode_t mode, const char *role, int manager_bit, int inspector_bit);
void add_report(AppContext *context);
void list_reports(AppContext *context);
void view_report(AppContext *context);
void remove_report(AppContext *context);
void check_and_alert_severity(const char *district, int severity);
int check_symlink(const char *filepath);
void verify_district_symlink(AppContext *context);
void update_threshold(AppContext *context);
void remove_district(AppContext *context);

//ai generated helper functions for filter command
int parse_condition(const char *input, char *field, char *op, char *value);
int match_condition(Report *r, const char *field, const char *op, const char *value);

void filter_reports(AppContext *context, int argc, char *argv[]);


#endif 