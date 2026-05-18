#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

void cmd_calculate_scores(char *args) {
    char *districts[50];
    int num_districts = 0;

    //parsam argumentele din lista de districte 
    char *token = strtok(args, " \n");
    while (token != NULL && num_districts < 50) {
        districts[num_districts++] = token;
        token = strtok(NULL, " \n");
    }

    if (num_districts == 0) {
        printf("Eroare: Trebuie specificat cel putin un district.\n");
        return;
    }

    int pipes[50][2];
    // un pipe pentru fiecare district, pipes[i][0] pentru citire, pipes[i][1] pentru scriere
    pid_t pids[50];
    //cream procesele copil si pipes pentru fiecare district

    printf("\nCombined Workload Report\n");

    for (int i = 0; i < num_districts; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("Eroare la crearea pipeului.");
            return;
        }

        pids[i] = fork();
        
        if (pids[i] == -1) {
            perror("Eroare la fork");
        } else if (pids[i] == 0) {
            // PROCESUL COPIL (Scorer)
            
            close(pipes[i][0]); // 1. Inchidem capatul de citire, copilul doar scrie

            // 2. MAGIA dup2: Redirectionam STDOUT (ecranul) catre capatul de scriere al pipe-ului
            dup2(pipes[i][1], STDOUT_FILENO);
            
            close(pipes[i][1]); // 3. Inchidem descriptorul original, am facut deja clona cu dup2

            // 4. Inlocuim copilul cu executabilul 'scorer' creat la Pasul 2
            execlp("./scorer", "scorer", districts[i], NULL);
            
            // Daca codul ajunge aici, execlp a esuat (ex: scorer nu exista)
            perror("Eroare la exec scorer");
            exit(1);
        } else {
            // PROCESUL PARINTE (Hub-ul)
            close(pipes[i][1]); // Parintele doar citeste, deci trebuie sa inchida capatul de scriere
        }
    }

    // Faza 2: Parintele colecteaza datele de la toti copiii
    for (int i = 0; i < num_districts; i++) {
        char buffer[1024];
        ssize_t bytes_read;
        
        // Citim din pipe pana cand copilul termina de executat (trimite EOF cand se inchide)
        while ((bytes_read = read(pipes[i][0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytes_read] = '\0';
            printf("%s", buffer); // Afisam textul venit de la scorer in hub
        }
        
        close(pipes[i][0]); // Am terminat de citit, inchidem capatul de citire
        waitpid(pids[i], NULL, 0); // Asteptam sa moara procesul copil curat (evitam procesele Zombie)
    }
    
    printf("================================\n\n");
}

int main() {
    char input[256];

    printf("City Hub pornit.\n");
    printf("Comenzi disponibile:\n");
    printf(" - calculate_scores <d1> <d2> ...\n");
    printf(" - start_monitor (urmeaza)\n");
    printf(" - exit\n\n");

    while (1) {
        printf("hub> ");
        if (!fgets(input, sizeof(input), stdin)) {
            break; // Daca apesi Ctrl+D iese elegant
        }

        input[strcspn(input, "\n")] = 0; // Curatam enter-ul (\n) de la final

        if (strlen(input) == 0) continue;

        if (strcmp(input, "exit") == 0 || strcmp(input, "quit") == 0) {
            break;
        }

        // Extragem prima parte (comanda) si a doua parte (argumentele)
        char *cmd = strtok(input, " ");
        char *args = strtok(NULL, ""); 

        if (strcmp(cmd, "calculate_scores") == 0) {
            if (args == NULL) {
                printf("Eroare: Lipsesc districtele. Utilizare: calculate_scores downtown uptown\n");
            } else {
                cmd_calculate_scores(args);
            }
        } else if (strcmp(cmd, "start_monitor") == 0) {
            printf("Functia start_monitor urmeaza sa fie implementata in pasul urmator.\n");
        } else {
            printf("Comanda necunoscuta: %s\n", cmd);
        }
    }

    return 0;
}
  