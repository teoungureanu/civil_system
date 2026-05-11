#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "city_commands.h"


#define MAX_INSPECTORS 100 
//presupunem max 100 inspectori pt simplitate


typedef struct {
    char name[64];
    int score;
} InspectorScore;


int main(int argc, char *argv[]) {
    if (argc < 2) {
        return 1;
    }

    char dat_path[256];
    snprintf(dat_path, sizeof(dat_path), "%s/reports.dat", argv[1]);

    int fd = open(dat_path, O_RDONLY);
    if (fd == -1) {
        return 0;
    }
    //

    InspectorScore scores[MAX_INSPECTORS];
    int num_inspectors = 0;
    Report rep;

    while(read(fd, &rep, sizeof(Report)) == sizeof(Report)) {
        int found = 0;
        for(int i = 0; i < num_inspectors; i++) {    
            if (strcmp(scores[i].name, rep.inspector_name) == 0) {
                scores[i].score += rep.severity;        
                //adaugam severitatea raportului la scorul inspectorului   
                found = 1;
                break;
            }
        }
        if (!found && num_inspectors < MAX_INSPECTORS) {
            strncpy(scores[num_inspectors].name, rep.inspector_name, 63);         
            //ne asiguram ca numele nu depaseste 63 de caractere + null terminator
            //scores[num_inspectors].name[63] = '\0'; //asiguram null termination
            scores[num_inspectors].score = rep.severity;
            num_inspectors++;
        }

    }
    close(fd);

    printf("Scor Inspectori pentru disctrictul %s:\n", argv[1]);

    for(int i = 0; i < num_inspectors; i++) {
        printf("Inspector %s: %d points\n", scores[i].name, scores[i].score);
    }

    return 0;
}

//FIX MAKEFILE TO IMPLEMENT SCORER !