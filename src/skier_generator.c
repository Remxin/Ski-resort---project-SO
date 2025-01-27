#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/msg.h>
#include <unistd.h>
#include "skier_generator.h"


volatile sig_atomic_t keep_running = 1;

void handle_shutdown() {
    keep_running = 0;
}

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    if (argc != 2) {
        perror("Wrong argument cound at skier_generator");
        exit(1);
    }
    srand(time(NULL));
    // first skier id and char arr to convert int to chr
    int skier_id = 1;
    char skier_id_chr[20];

    // init shared data
    int shmid = atoi(argv[1]);

    SharedData* shared_data = attach_shared_memory(shmid);
    if (shared_data == (void*)-1) {
        perror("Failed to attach shared memory");
        exit(1);
    }
    
    while (keep_running) {
        if (shared_data->skier_count < MAX_SKIERS) {
            sprintf(skier_id_chr, "%d", skier_id);
            pid_t skier_pid = fork();
            if (skier_pid == 0) {
                execl("./bin/skier", "skier", skier_id_chr, argv[1], NULL);
                perror("Failed to create skier generator");
                exit(EXIT_FAILURE);
            }
            shared_data->skiers[shared_data->skier_count++] = skier_pid;
            skier_id++;
            
        }

        // Random delay between new skiers
        sleep(3);

        // Clean up finished processes
        pid_t finished_pid;
        while ((finished_pid = waitpid(-1, NULL, WNOHANG)) > 0) {
            for (int i = 0; i < shared_data->skier_count; i++) {
                if (shared_data->skiers[i] == finished_pid) {
                    shared_data->skiers[i] = shared_data->skiers[--shared_data->skier_count];
                    break;
                }
            }
        }
    }
    
    exit(0);
}