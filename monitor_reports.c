#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>

volatile sig_atomic_t keep_running = 1;
// prinde SIGINT


void handle_sigint(int sig) {
    const char *msg = "\nSIGINT received. Shutting down.\n";
    // file descriptorul STDOUT_FILENO reprezinta stdout
    write(STDOUT_FILENO, msg, strlen(msg));
    keep_running = 0;
}

void handle_sigusr1(int sig) {
    const char *msg = "A new report has been added.\n";
    write(STDOUT_FILENO, msg, strlen(msg));
}

int main() {
    struct sigaction sa_int;  
    memset(&sa_int, 0, sizeof(sa_int)); 
    sa_int.sa_handler = handle_sigint; // setam handlerul pentru SIGINT
    sigemptyset(&sa_int.sa_mask);     // asiguram ca nu blocam alte semnale in timp ce procesam SIGINT
    sigaction(SIGINT, &sa_int, NULL); 

    struct sigaction sa_usr1;
    memset(&sa_usr1, 0, sizeof(sa_usr1));
    sa_usr1.sa_handler = handle_sigusr1; 
    sigemptyset(&sa_usr1.sa_mask);     
    sa_usr1.sa_flags = SA_RESTART;  //apelurile de sistem intrerupte de acest semnal vor fi reluate automat
    sigaction(SIGUSR1, &sa_usr1, NULL);

    pid_t pid = getpid();   // obtinem PID-ul procesului curent
    int fd = open(".monitor_pid", O_CREAT | O_WRONLY | O_TRUNC, 0644); // cream fisierul pentru a stoca PID-ul, cu permisiuni 644
    if (fd != -1) { 
        char pid_str[32];
        int len = snprintf(pid_str, sizeof(pid_str), "%d\n", pid);
        write(fd, pid_str, len);
        close(fd);
    }

    const char *start_msg = "Monitor running. Waiting for signals..\n";
    write(STDOUT_FILENO, start_msg, strlen(start_msg));
    // asteptam semnale pana cand primim SIGINT, moment in care setam keep_running la 0 si iesim din loop
    while (keep_running) {
        pause();
    }
    remove(".monitor_pid"); // stergem fisierul cu PID-ul la iesire
    return 0;

   


}