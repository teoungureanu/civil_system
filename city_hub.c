#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

void cmd_calculate_scores(char *args) { //functie pt calculator scoruri per district
    char *districts[50]; 
    int num_districts = 0; 
    char *token = strtok(args, " \n");
    while (token != NULL && num_districts < 50) { 
        districts[num_districts++] = token; // 
        token = strtok(NULL, " \n"); 
    } 
    if (num_districts == 0) { 
        printf("Eroare: Trebuie specificat cel putin un district.\n"); 
        return; 
    } 
    int pipes[50][2]; // matrice pentru a stoca capetele de citire si scriere ale canalelor
    pid_t pids[50]; // vector pentru id proceselor clonate
    printf("\nCombined Workload Report\n"); 
    for (int i = 0; i < num_districts; i++) { 
        if (pipe(pipes[i]) == -1) { //canal de comunicare
            perror("Eroare la crearea pipe-ului"); 
            continue; 
        } 
        pids[i] = fork(); // clonam procesul si salvam id
        if (pids[i] == -1) { 
            perror("Eroare la fork"); 
        } else if (pids[i] == 0) { // id 0 inseamna ca suntem in procesul copil
            close(pipes[i][0]); // copilul nu citeste, deci inchidem capatul de receptie
            dup2(pipes[i][1], STDOUT_FILENO); // fortam iesirea standard sa mearga in canalul de scriere al copilului
            close(pipes[i][1]); // inchidem capatul de scriere original dupa redirectionare

            execlp("./scorer", "scorer", districts[i], NULL); // suprascriem copilul cu executabilul scorer
            perror("Eroare la exec scorer"); //daca exec esueaza, afisam eroarea
            exit(1); 
        } else { 
            close(pipes[i][1]); // parintele nu emite, deci inchidem capatul de scriere al parintelui
        } 
    } 
    for (int i = 0; i < num_districts; i++) { // parcurgem copii pentru rezultatele
        char buffer[1024]; 
        ssize_t bytes_read; 
        while ((bytes_read = read(pipes[i][0], buffer, sizeof(buffer) - 1)) > 0) { 
            buffer[bytes_read] = '\0';
            printf("%s", buffer); 
        } 
        close(pipes[i][0]); 
        waitpid(pids[i], NULL, 0); // asteptam ca fiecare copil sa isi termine executia
    } 
    printf("\n\n"); 
}

void cmd_start_monitor() {
    pid_t hub_mon_pid = fork(); //hub_mon va supraveghea monitorul

    if (hub_mon_pid == -1) {
        perror("Eroare la crearea hub_mon");
        return;
    } else if (hub_mon_pid == 0) {

        int pipefd[2]; // 0 citire, 1 scriere
        if (pipe(pipefd) == -1) {
            perror("Eroare la crearea pipe-ului in hub_mon");
            exit(1);
        }

        pid_t monitor_pid = fork(); //monitorul propriu zis

        if (monitor_pid == -1) {
            perror("Eroare la crearea procesului monitor");
            exit(1);
        } else if (monitor_pid == 0) {
            close(pipefd[0]); //inchidem capatul de citire

            
            dup2(pipefd[1], STDOUT_FILENO); 
            close(pipefd[1]);

            execlp("./monitor_reports", "monitor_reports", NULL); //suprascriem cu monitorul
            
            perror("Eroare la pornirea monitor_reports");
            exit(1);
        } else { 
            close(pipefd[1]); 
            
            FILE *stream = fdopen(pipefd[0], "r");
            char buffer[256];

            if (stream) {
                while (fgets(buffer, sizeof(buffer), stream)) {
                    
                    if (strncmp(buffer, "ERR_PID:", 8) == 0) { //daca avem deja un monitor activ
                        printf("\n[Hub Monitor] Eroare: Monitorul ruleaza deja cu PID %s", buffer + 8);
                        break; 
                    } else if (strncmp(buffer, "EVT_NOTIFY:", 11) == 0) { //notificare
                        printf("\n[Hub Monitor] ALERTA: %s", buffer + 11);
                        printf("hub> ");
                        fflush(stdout);
                    } else if (strncmp(buffer, "INF_START:", 10) == 0) { //monitorul a pornit cu succes
                        printf("\n[Hub Monitor] Procesul a fost pornit cu succes in fundal.\n");
                    } else if (strncmp(buffer, "INF_STOP:", 9) == 0) { //monitorul s-a oprit cu succes                       
                        printf("\n[Hub Monitor] Procesul s-a inchis cu succes.\n");
                        printf("hub> ");
                        fflush(stdout);
                        break;
                    }

                } 
                fclose(stream);
            } 
            
            waitpid(monitor_pid, NULL, 0); //asteptam sa se termine monitorul
            exit(0); //hub_mon se va inchide dupa ce monitorul se inchide
        }
    }


    printf("Procesul de supraveghere (hub_mon) a fost lansat cu PID %d.\n", hub_mon_pid);

    usleep(50000); //mic delay ca sa inceapa hub_mon inainte de a afisa promptul iar
}

int main() { 
    char input[256]; 
    printf("City Hub pornit.\n"); 
    printf("Comenzi disponibile:\n"); 
    printf(" - calculate_scores <d1> <d2> ...\n"); 
    printf(" - start_monitor\n"); 
    printf(" - exit\n\n"); 
    while (1) { 
        printf("hub> ");
        if (!fgets(input, sizeof(input), stdin)) { 
            break; 
        } 
        input[strcspn(input, "\n")] = 0; 
        if (strlen(input) == 0) continue; // evitam procesarea daca e doar enter fara text
        if (strcmp(input, "exit") == 0 || strcmp(input, "quit") == 0) { 
            break; 
        } 
        char *cmd = strtok(input, " "); 
        char *args = strtok(NULL, ""); 
        if (strcmp(cmd, "calculate_scores") == 0) { 
            if (args == NULL) {
                printf("Eroare: Lipsesc districtele. Utilizare: calculate_scores downtown uptown\n"); 
            } else { 
                cmd_calculate_scores(args); 
            } 
        } else if (strcmp(cmd, "start_monitor") == 0) {
            cmd_start_monitor();
        } else { 
            printf("Comanda necunoscuta\n"); 
        }
    } 
    return 0;
} 