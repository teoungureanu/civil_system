#include "city_commands.h"

void parse_arguments(int argc, char *argv[], AppContext *context) {
    memset(context, 0, sizeof(AppContext));

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--role") == 0 && i + 1 < argc) {
            //making sure we have the next argument (i + 1)
            strncpy(context->role, argv[i + 1], sizeof(context->role) - 1);
            i++;
            //am procesat 2 argumente, deci incrementam i 
        } 

        else if (strcmp(argv[i], "--user") == 0 && i + 1 < argc) {
            strncpy(context->user, argv[i + 1], sizeof(context->user) - 1);
            i++;
        } 

        else if (strcmp(argv[i], "--add") == 0 && i + 1 < argc) {
            strncpy(context->command, "add", sizeof(context->command) - 1);
            strncpy(context->district, argv[i + 1], sizeof(context->district) - 1);
            i++;
        } 

        else if (strcmp(argv[i], "--list") == 0 && i + 1 < argc) {
            strncpy(context->command, "list", sizeof(context->command) - 1);
            strncpy(context->district, argv[i + 1], sizeof(context->district) - 1);
            i++;
        }
        
        else if (strcmp(argv[i], "--view") == 0 && i + 2 < argc) {
            strncpy(context->command, "view", sizeof(context->command) - 1);
            strncpy(context->district, argv[i + 1], sizeof(context->district) - 1);
            context->target_id = atoi(argv[i + 2]);
            i+=2;
        } 

        else if (strcmp(argv[i], "--remove_report") == 0 && i + 2 < argc) {
            strncpy(context->command, "remove_report", sizeof(context->command) - 1);
            strncpy(context->district, argv[i + 1], sizeof(context->district) - 1);
            context->target_id = atoi(argv[i + 2]);
            i+=2;
        } 

        else if (strcmp(argv[i], "--update_threshold") == 0 && i + 2 < argc) {
            strncpy(context->command, "update_threshold", sizeof(context->command) - 1);
            strncpy(context->district, argv[i + 1], sizeof(context->district) - 1);
            strncpy(context->extra_arg, argv[i + 2], sizeof(context->extra_arg) - 1);
            i+=2;
        } 

        else if (strcmp(argv[i], "--filter") == 0 && i + 2 < argc) {
            strncpy(context->command, "filter", sizeof(context->command) - 1);
            strncpy(context->district, argv[i + 1], sizeof(context->district) - 1);
            strncpy(context->extra_arg, argv[i + 2], sizeof(context->extra_arg) - 1);
            i+=2;
        }

    }
}


void mode_to_string(mode_t mode, char *str) {
    strcpy(str, "---------");
    
    if (mode & S_IRUSR) str[0] = 'r';
    if (mode & S_IWUSR) str[1] = 'w';
    if (mode & S_IXUSR) str[2] = 'x';
    
    if (mode & S_IRGRP) str[3] = 'r';
    if (mode & S_IWGRP) str[4] = 'w';
    if (mode & S_IXGRP) str[5] = 'x';
    
    if (mode & S_IROTH) str[6] = 'r';
    if (mode & S_IWOTH) str[7] = 'w';
    if (mode & S_IXOTH) str[8] = 'x';
}

int has_permission(mode_t mode, const char *role, int manager_bit, int inspector_bit) {
    if (strcmp(role, "manager") == 0) {
        return (mode & manager_bit);
    } 
    if (strcmp(role, "inspector") == 0) {
        return (mode & inspector_bit);
    }
    return 0;
}


int setup_and_log_action(AppContext *context) {
    struct stat st;
    char log_path[256];
    snprintf(log_path, sizeof(log_path), "%s/logged_district", context->district);

    if (stat(context->district, &st) == -1) {
        mkdir(context->district, 0750);
        chmod(context->district, 0750);
    }

    if (stat(log_path, &st) == -1) {
        int fd = open(log_path, O_CREAT | O_WRONLY, 0644);
        if (fd != -1) {
            close(fd);
            chmod(log_path, 0644);
            stat(log_path, &st);
        }
    }

    if (!has_permission(st.st_mode, context->role, S_IWUSR, S_IWGRP)) {
        fprintf(stderr, "Acces interzis: Rolul '%s' nu are drept de scriere in log.\n", context->role);
        return 0;
    }

    int fd = open(log_path, O_WRONLY | O_APPEND);
    if (fd == -1) return 0;

    time_t now = time(NULL);
    char entry[256];
    int len = snprintf(entry, sizeof(entry), "%ld %s %s %s\n", now, context->user, context->role, context->command);
    
    write(fd, entry, len);
    close(fd);
    return 1;
}

void add_report(AppContext *context) {
    char dat_path[256];
    snprintf(dat_path, sizeof(dat_path), "%s/reports.dat", context->district);

    struct stat st;
    if (stat(dat_path, &st) == -1) {
        int fd = open(dat_path, O_CREAT | O_WRONLY, 0664);
        if (fd != -1) {
            close(fd);
            chmod(dat_path, 0664);
            stat(dat_path, &st);
        }
    }

    char sym_path[256];
    snprintf(sym_path, sizeof(sym_path), "active_reports-%s", context->district);
    
    struct stat sym_st;
    if (lstat(sym_path, &sym_st) == -1) {
        symlink(dat_path, sym_path);
        
    }

    int id = (st.st_size / sizeof(Report)) + 1;
    //id-ul raportului = dimensiunea fisierului / dimensiunea unui raport

    Report rep;
    memset(&rep, 0, sizeof(Report));

    rep.id = id;
    strncpy(rep.inspector_name, context->user, sizeof(rep.inspector_name) - 1);
    rep.timestamp = time(NULL);

    char buffer[256];

    printf("Latitude: ");
    if (fgets(buffer, sizeof(buffer), stdin)) rep.latitude = atof(buffer);
    //atof converteste stringul introdus de utilizator intr-un double

    printf("Longitude: ");
    if (fgets(buffer, sizeof(buffer), stdin)) rep.longitude = atof(buffer);

    printf("Severity: ");
    if (fgets(buffer, sizeof(buffer), stdin)) rep.severity = atoi(buffer);

    printf("Category: ");
    if (fgets(buffer, sizeof(buffer), stdin)) {
        buffer[strcspn(buffer, "\n")] = 0;
        //cautam newline si il inlocuim cu \0 
        strncpy(rep.category, buffer, sizeof(rep.category) - 1);
    }

    printf("Description: ");
    if (fgets(buffer, sizeof(buffer), stdin)) {
        buffer[strcspn(buffer, "\n")] = 0;
        strncpy(rep.description, buffer, sizeof(rep.description) - 1);
    }

    int fd = open(dat_path, O_WRONLY | O_APPEND);
    if (fd == -1) {
        fprintf(stderr, "Eroare: Nu s-a putut deschide %s pentru scriere\n", dat_path);
        return;
    }

    write(fd, &rep, sizeof(Report));
    close(fd);

    printf("Raport adaugat cu succes! ID: %d\n", id);
}


void list_reports(AppContext *context) {
    char sym_path[256];
    snprintf(sym_path, sizeof(sym_path), "active_reports-%s", context->district);

    struct stat sym_st;
    if (lstat(sym_path, &sym_st) == -1) {
        printf("Avertisment: Link-ul simbolic %s nu exista sau este corupt.\n", sym_path);
    } else {
        char perm_str[10];
        mode_to_string(sym_st.st_mode, perm_str);
        printf("Permisiuni symlink (%s): %s\n", sym_path, perm_str);
    }

    char dat_path[256];
    snprintf(dat_path, sizeof(dat_path), "%s/reports.dat", context->district);

    int fd = open(dat_path, O_RDONLY);
    if (fd == -1) {
        printf("Nu exista rapoarte inregistrate pentru districtul %s.\n", context->district);
        return;
    }

    printf("\nLista Rapoarte: %s \n", context->district);

    Report rep;
    int count = 0;

    while (read(fd, &rep, sizeof(Report)) == sizeof(Report)) {
        if (rep.id != 0) {
            printf("[ID: %d] %s - Severitate: %d (Inspector: %s)\n", rep.id, rep.category, rep.severity, rep.inspector_name);
            count++;
        }
    }

    if (count == 0) {
        printf("  (Niciun raport activ gasit)\n");
    }
    
    printf("\n\n");

    close(fd);
}