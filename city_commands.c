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
        else if (strcmp(argv[i], "--remove_district") == 0 && i + 1 < argc) {
            strncpy(context->command, "remove_district", sizeof(context->command) - 1);
            strncpy(context->district, argv[i + 1], sizeof(context->district) - 1);
            i++;
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

    // fisier de log
    if (stat(log_path, &st) == -1) {
        int fd_init = open(log_path, O_CREAT | O_WRONLY, 0644);
        if (fd_init != -1) {
            close(fd_init);
            chmod(log_path, 0644);
            stat(log_path, &st);
        }
    }

    //doar manager poate scrie in log
    if (strcmp(context->role, "manager") == 0) {
        if (!(st.st_mode & S_IWUSR)) {
            fprintf(stderr, "Eroare: Fisierul log nu permite scrierea managerului.\n");
            return 0;
        }
    } else {
        //inspectorii nu au voie sa scrie in log
        return 1; 
    }

    int fd = open(log_path, O_WRONLY | O_APPEND);
    if (fd == -1) return 0;

    time_t timestamp = time(NULL);
    char entry[512];
    //logam actiuena
    int len = snprintf(entry, sizeof(entry), "[%ld]   %s   %s   %s\n", timestamp, context->user, context->role, context->command);
    
    write(fd, entry, len);
    close(fd);
    return 1;
}

void add_report(AppContext *context) {
    char dat_path[256];
    char cfg_path[256];
    char sym_path[256];
    char buffer[256];
    struct stat dat_st;
    struct stat cfg_st;
    struct stat sym_st;
    Report rep;

    snprintf(dat_path, sizeof(dat_path), "%s/reports.dat", context->district);
    snprintf(cfg_path, sizeof(cfg_path), "%s/district.cfg", context->district);
    snprintf(sym_path, sizeof(sym_path), "active_reports-%s", context->district);

    if (stat(dat_path, &dat_st) == -1) {
        int fd_init = open(dat_path, O_CREAT | O_WRONLY, 0664);
        if (fd_init != -1) {
            close(fd_init);
            chmod(dat_path, 0664);
            stat(dat_path, &dat_st);
        }
    }

    
    if (!has_permission(dat_st.st_mode, context->role, S_IWUSR, S_IWGRP)) {
        printf("Eroare de conformitate: Rolul '%s' nu are drept de scriere pe %s conform bitilor actuali.\n", context->role, dat_path);
        return;
    }

    if (stat(cfg_path, &cfg_st) == -1) {
        int fd_cfg = open(cfg_path, O_CREAT | O_WRONLY, 0640);
        if (fd_cfg != -1) {
            write(fd_cfg, "severity_level=1\n", 17);
            close(fd_cfg);
            chmod(cfg_path, 0640);
        }
    }

    if (lstat(sym_path, &sym_st) == -1) {
        symlink(dat_path, sym_path);
    }

    int id = (dat_st.st_size / sizeof(Report)) + 1;
    
    memset(&rep, 0, sizeof(Report));
    rep.id = id;
    strncpy(rep.inspector_name, context->user, sizeof(rep.inspector_name) - 1);
    rep.timestamp = time(NULL);

    printf("Latitude: ");
    if (fgets(buffer, sizeof(buffer), stdin)) rep.latitude = atof(buffer);

    printf("Longitude: ");
    if (fgets(buffer, sizeof(buffer), stdin)) rep.longitude = atof(buffer);

    printf("Severity: ");
    if (fgets(buffer, sizeof(buffer), stdin)) rep.severity = atoi(buffer);

    printf("Category: ");
    if (fgets(buffer, sizeof(buffer), stdin)) {
        buffer[strcspn(buffer, "\n")] = 0;
        strncpy(rep.category, buffer, sizeof(rep.category) - 1);
    }

    printf("Description: ");
    if (fgets(buffer, sizeof(buffer), stdin)) {
        buffer[strcspn(buffer, "\n")] = 0;
        strncpy(rep.description, buffer, sizeof(rep.description) - 1);
    }

    check_and_alert_severity(context->district, rep.severity);

    int fd = open(dat_path, O_WRONLY | O_APPEND);
    if (fd == -1) {
        fprintf(stderr, "Eroare: Nu s-a putut deschide %s pentru scriere\n", dat_path);
        return;
    }

    write(fd, &rep, sizeof(Report));
    close(fd);
    printf("Raport adaugat cu succes! ID: %d\n", id);
    // PHASE 2: trimitem semnal catre monitor pentru a anunta adaugarea unui nou raport
    int monitor_notified = 0;
    int pid_fd = open(".monitor_pid", O_RDONLY); // deschidem fisierul in care monitorul isi stocheaza PID-ul pentru a citi si a trimite semnalul
    if (pid_fd != -1) {
        char pid_buffer[32];
        ssize_t bytes_read = read(pid_fd, pid_buffer, sizeof(pid_buffer) - 1); // citim PID-ul monitorului din fisier
        if(bytes_read > 0) { 
            pid_buffer[bytes_read] = '\0';
            pid_t monitor_pid = (pid_t)atoi(pid_buffer); // convertim stringul citit intr-un PID
            if (kill(monitor_pid, SIGUSR1) == 0) { // trimitem semnalul SIGUSR1 folosit pentru a transmite notificarea 
                monitor_notified = 1;      // daca kill returneaza 0, inseamna ca semnalul a fost trimis cu succes
            }

            
        }
        close(pid_fd);
    }

    char log_path[256]; 
    snprintf(log_path, sizeof(log_path), "%s/logged_district", context->district);  
    int log_fd = open(log_path, O_WRONLY | O_CREAT | O_APPEND, 0644);  
    if (log_fd != -1) {
        char log_msg[256];
        if (monitor_notified) { // daca am reusit sa anuntam monitorul, logam acest lucru
            snprintf(log_msg, sizeof(log_msg), "Raport ID %d adaugat. Monitor notificat cu succes.\n", id);
        } else { //nu am reusit sa anuntam deci logam eroarea
            snprintf(log_msg, sizeof(log_msg), "Raport ID %d adaugat. Eroare: Monitorul nu a putut fi informat.\n", id);
        }
        write(log_fd, log_msg, strlen(log_msg)); 
        close(log_fd);
    }
}


void list_reports(AppContext *context) {
    char dat_path[256];
    snprintf(dat_path, sizeof(dat_path), "%s/reports.dat", context->district);

    struct stat dat_st;
    if (stat(dat_path, &dat_st) == -1) {
        printf("Eroare: Fisierul de rapoarte %s nu exista\n", dat_path);
        return;
    }


    if (!has_permission(dat_st.st_mode, context->role, S_IRUSR, S_IRGRP)) {
        printf("Rolul '%s' nu are drept de citire pe %s\n", context->role, dat_path);
        return;
    }
    //verificam daca utilizatorul are permisiunea de citire
    char perm_str[10];
    mode_to_string(dat_st.st_mode, perm_str);

    char time_str[64];
    struct tm *tm_info = localtime(&dat_st.st_mtime);
    
    strftime(time_str, sizeof(time_str), "%d-%m-%Y %H:%M:%S", tm_info);
    //strftime formateaza data ultimei modificari a fisierului intr-un format lizibuil

    printf("\nInformatii Fisier District: %s\n", context->district);
    printf("Permisiuni: %s\n", perm_str);
    printf("Dimensiune: %ld bytes\n", (long)dat_st.st_size);
    printf("Ultima modificare: %s\n", time_str);
    printf("\n\n");

    verify_district_symlink(context);

    int fd = open(dat_path, O_RDONLY);
    if (fd == -1) {
        return;
    }

    printf("\nLista Rapoarte: %s\n", context->district);

    Report rep;
    int count = 0;
    while (read(fd, &rep, sizeof(Report)) == sizeof(Report)) {
    
        printf("[ID: %d] %s - Severitate: %d (Inspector: %s)\n", rep.id, rep.category, rep.severity, rep.inspector_name);
        count++;
        
    }

    if (count == 0) {
        printf("  (Niciun raport activ gasit)\n");
    }
    printf("\n\n");

    close(fd);
}


void view_report(AppContext *context) {
    if (context->target_id <= 0) {
        printf("Eroare: ID-ul raportului este invalid\n");
        return;
    }

    char dat_path[256];
    snprintf(dat_path, sizeof(dat_path), "%s/reports.dat", context->district);

    int fd = open(dat_path, O_RDONLY);
    if (fd == -1) {
        printf("Eroare: Nu s-a putut deschide %s. Verificati daca exista rapoarte\n", dat_path);
        return;
    }
    //lseek returneaza off_t
    off_t offset = (context->target_id - 1) * sizeof(Report);

    if (lseek(fd, offset, SEEK_SET) == -1) {
        printf("Eroare la navigarea in fisier.\n");
        close(fd);
        return;
    }

    Report rep;
    if (read(fd, &rep, sizeof(Report)) == sizeof(Report)) {
        if (rep.id == context->target_id) {
            
            char date_str[64];
            struct tm *tm_info = localtime(&rep.timestamp);
            //strftime formateaza data in format de string
            strftime(date_str, sizeof(date_str), "%d-%m-%Y %H:%M:%S", tm_info);

            printf("\nDetalii Raport ID: %d\n", rep.id);
            printf("Inspector:  %s\n", rep.inspector_name);
            printf("Data:       %s\n", date_str);
            printf("Locatie:    Lat %.6f, Lon %.6f\n", rep.latitude, rep.longitude);
            printf("Categorie:  %s\n", rep.category);
            printf("Severitate: %d\n", rep.severity);
            printf("Descriere:  %s\n", rep.description);
            printf("\n\n");
        } else {
            printf("Raportul cu ID-ul %d a fost sters\n", context->target_id);
        }
    } else {
        printf("Raportul cu ID-ul %d nu a fost gasit (ID prea mare)\n", context->target_id);
    }

    close(fd);
}

void remove_report(AppContext *context) {
    if (strcmp(context->role, "manager") != 0) {
        printf("Eroare: Doar rolul de manager poate sterge rapoarte.\n");
        return;
    }

    if (context->target_id <= 0) {
        printf("Eroare: ID invalid pentru stergere\n");
        return;
    }

    char dat_path[256];
    snprintf(dat_path, sizeof(dat_path), "%s/reports.dat", context->district);

    struct stat st_perm;

    if (stat(dat_path, &st_perm) == -1) {
        printf("Eroare: Nu s-au gasit rapoarte pentru districtul %s.\n", context->district);
        return;
    }

    if (!has_permission(st_perm.st_mode, context->role, S_IWUSR, 0)) { 
        printf("Eroare de conformitate: Permisiunile de pe disc interzic stergerea pentru rolul '%s'.\n", context->role);
        return;
    }

    int fd = open(dat_path, O_RDWR);

    Report rep;
    off_t target_offset = -1;

    while (read(fd, &rep, sizeof(Report)) == sizeof(Report)) {
        if (rep.id == context->target_id) {
            target_offset = lseek(fd, 0, SEEK_CUR) - sizeof(Report);
            //calculam offset-ul raportului gasit pentru a sti de unde sa incepem sa citim restul rapoartelor
            break;
        }
    }

    if (target_offset == -1) {
        printf("Eroare: Raportul cu ID %d nu a fost gasit.\n", context->target_id);
        close(fd);
        return;
    }

    off_t read_offset = target_offset + sizeof(Report);
    //pozitia raportului urmator
    off_t write_offset = target_offset;
    // pozitia raportului curent care trebuie suprascris

    while (1) {
        lseek(fd, read_offset, SEEK_SET);
        int bytes_read = read(fd, &rep, sizeof(Report));
        
        if (bytes_read <= 0) {
            //daca nu mai avem ce citi, iesim din loop
            break;
        }

        lseek(fd, write_offset, SEEK_SET);
        write(fd, &rep, sizeof(Report));
        //dupa ce am citit un raport, il suprascriem la pozitia raportului sters
        read_offset += sizeof(Report);
        write_offset += sizeof(Report);
        //incrementam ambele offset-uri
    }

    if (ftruncate(fd, write_offset) == -1) {
        //ftruncate reduce dimensiunea fisierului la write_offset, eliminand datele ramase dupa noua dimensiune
        printf("Eroare la trunchiere\n");
    } else {
        printf("Raportul %d a fost sters fizic cu succes!\n", context->target_id);
    }

    close(fd);
}
void check_and_alert_severity(const char *district, int severity) {
    char cfg_path[256];
    snprintf(cfg_path, sizeof(cfg_path), "%s/district.cfg", district);

    int fd = open(cfg_path, O_RDONLY);
    if (fd == -1) {
        return;
    }

    char buffer[128];
    memset(buffer, 0, sizeof(buffer));

    int bytes_read = read(fd, buffer, sizeof(buffer) - 1);
    close(fd);

    if (bytes_read > 0) {
        int threshold = 0;
        
        if (sscanf(buffer, "severity_level=%d", &threshold) == 1) {
            // am impus acest format pentru prag de securitate
            if (severity > threshold) {
                printf("\nSeveritatea (%d) depaseste pragul maxim (%d) setat pentru districtul %s!\n", severity, threshold, district);
            }
        }
    }
}

int check_symlink(const char *filepath) {
    struct stat link_stat;
    struct stat target_stat;

    if (lstat(filepath, &link_stat) == 0) {
        if (S_ISLNK(link_stat.st_mode)) {
            if (stat(filepath, &target_stat) != 0) {
                if (errno == ENOENT) {
                    // ENOENT inseamna ca fisierul tinta nu exista, deci link-ul este dangling
                    fprintf(stderr, "Dangling link detected at '%s'. Target is missing\n", filepath);
                    return -1;
                } else {
                    perror("Stat failed for an unexpected reason");
                    return -1;
                }
            }
        }
    }

    return 0;
}

void verify_district_symlink(AppContext *context) {
    char sym_path[256];
    snprintf(sym_path, sizeof(sym_path), "active_reports-%s", context->district);
    check_symlink(sym_path);
}

void update_threshold(AppContext *context) { 
    if (strcmp(context->role, "manager") != 0) { 
        printf("Eroare de acces: Doar rolul de manager poate actualiza pragul de severitate.\n"); 
        return;
    }

    char cfg_path[256]; 
    snprintf(cfg_path, sizeof(cfg_path), "%s/district.cfg", context->district); 

    struct stat cfg_st; 
    if (stat(cfg_path, &cfg_st) == -1) { 
        printf("Eroare: Fisierul de configurare %s nu exista\n", cfg_path); 
        return; 
    }

    
    if ((cfg_st.st_mode & 0777) != 0640) { 
        printf("Permisiunile fisierului district.cfg (%o) nu sunt 640.\n", cfg_st.st_mode & 0777); 
        return; 
    }

    int fd = open(cfg_path, O_WRONLY | O_TRUNC); 
    if (fd == -1) { 
        printf("Eroare: Nu s-a putut deschide fisierul pentru scriere\n"); 
        return; 
    }

    char buffer[512]; 
    snprintf(buffer, sizeof(buffer), "severity_level=%s\n", context->extra_arg);
    
    
    write(fd, buffer, strlen(buffer)); 
    
    close(fd); 
    printf("Pragul de severitate pentru districtul %s a fost actualizat la nivelul %s\n", context->district, context->extra_arg); 
}


/* * Functie generata cu ajutorul AI pentru a sparge un sir de forma "camp:operator:valoare".
 */
int parse_condition(const char *input, char *field, char *op, char *value) {
    // Gasim prima aparitie a separatorului ':' pentru a delimita campul 
    const char *p1 = strchr(input, ':');
    if (!p1) return 0; // Format invalid daca lipseste primul separator

    // Extragem numele campului (ex: severity, category)
    size_t field_len = p1 - input;
    memcpy(field, input, field_len);
    field[field_len] = '\0';

    // Gasim al doilea separator ':' incepand de dupa primul
    const char *p2 = strchr(p1 + 1, ':');
    if (!p2) return 0; // Format invalid daca lipseste al doilea separator

    // Extragem operatorul (ex: ==, >=, <)
    size_t op_len = p2 - (p1 + 1);
    memcpy(op, p1 + 1, op_len);
    op[op_len] = '\0';

    // Restul sirului reprezinta valoarea de comparat
    strcpy(value, p2 + 1);

    return 1; // Parsare reusita
}

/*
 * Functie generata cu ajutorul AI pentru a verifica daca un record satisface o conditie.
 */
int match_condition(Report *r, const char *field, const char *op, const char *value) {
    // Comparatii pentru campul "severity" (int)
    if (strcmp(field, "severity") == 0) {
        int v = atoi(value);
        if (strcmp(op, "==") == 0) return r->severity == v;
        if (strcmp(op, "!=") == 0) return r->severity != v;
        if (strcmp(op, "<") == 0)  return r->severity < v;
        if (strcmp(op, "<=") == 0) return r->severity <= v;
        if (strcmp(op, ">") == 0)  return r->severity > v;
        if (strcmp(op, ">=") == 0) return r->severity >= v;
    } 
    // Comparatii pentru campul "category" (string)
    else if (strcmp(field, "category") == 0) {
        int res = strcmp(r->category, value);
        if (strcmp(op, "==") == 0) return res == 0;
        if (strcmp(op, "!=") == 0) return res != 0;
    } 
    // Comparatii pentru campul "inspector" (string)
    else if (strcmp(field, "inspector") == 0) {
        int res = strcmp(r->inspector_name, value);
        if (strcmp(op, "==") == 0) return res == 0;
        if (strcmp(op, "!=") == 0) return res != 0;
    } 
    // Comparatii pentru campul "id" (int)
    else if (strcmp(field, "id") == 0) {
        int v = atoi(value);
        if (strcmp(op, "==") == 0) return r->id == v;
        if (strcmp(op, "!=") == 0) return r->id != v;
        if (strcmp(op, "<") == 0)  return r->id < v;
        if (strcmp(op, "<=") == 0) return r->id <= v;
        if (strcmp(op, ">") == 0)  return r->id > v;
        if (strcmp(op, ">=") == 0) return r->id >= v;
    }
    // Comparatii pentru campul "timestamp" (time_t)
    else if (strcmp(field, "timestamp") == 0) {
        time_t v = (time_t)atoll(value);
        if (strcmp(op, "==") == 0) return r->timestamp == v;
        if (strcmp(op, "!=") == 0) return r->timestamp != v;
        if (strcmp(op, "<") == 0)  return r->timestamp < v;
        if (strcmp(op, "<=") == 0) return r->timestamp <= v;
        if (strcmp(op, ">") == 0)  return r->timestamp > v;
        if (strcmp(op, ">=") == 0) return r->timestamp >= v;
    }

    return 0;
}

void filter_reports(AppContext *context, int argc, char *argv[]) {
    char dat_path[256];
    snprintf(dat_path, sizeof(dat_path), "%s/reports.dat", context->district);

    int fd = open(dat_path, O_RDONLY);
    if (fd == -1) {
        printf("Eroare: Nu exista rapoarte in districtul %s.\n", context->district);
        return;
    }

    printf("\nRezultate Filtrare %s \n", context->district);
    Report rep;
    int found_count = 0;

    while (read(fd, &rep, sizeof(Report)) == sizeof(Report)) {
        int matches_all = 1;
        for (int i = 5; i < argc; i++) {
            char f[32], o[8], v[128];
            if (parse_condition(argv[i], f, o, v)) {
                if (!match_condition(&rep, f, o, v)) {
                    matches_all = 0;
                    break;
                }
            }
        }

        if (matches_all) {
            printf("[ID: %d] %s | Sev: %d | Insp: %s\n", rep.id, rep.category, rep.severity, rep.inspector_name);
            found_count++;
        }
    }

    if (found_count == 0) printf("  (Niciun raport nu indeplineste toate conditiile)\n");
    printf("\n\n");
    close(fd);
}

void remove_district(AppContext *context) {
    if(strcmp(context->role, "manager") != 0) {
        printf("Eroare: Doar managerii pot sterge un district\n");
        return;
    }
    char sym_path[256];
    snprintf(sym_path, sizeof(sym_path), "active_reports-%s", context->district);

    unlink(sym_path); 
    pid_t pid = fork();
    if (pid == -1) {
        printf("Eroare: Nu s-a putut crea procesul copil\n");
        return;
    }
    else if( pid == 0) {
        execlp("rm", "rm", "-rf", context->district, NULL);
        printf("Eroare la executie\n");
        return;
    }
    else {
        int status;
        wait(&status);
        if(WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            printf("Districtul %s a fost sters cu succes\n", context->district);
        } else {
            printf("Eroare la stergerea districtului %s.\n", context->district);
        }
    }
    
}


