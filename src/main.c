#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/msg.h>
#include "skier.h"
#include "ticket.h"
#include "cashier.h"
#include "shared_memory.h"
#include "config.h"
#include "utils.h"
#include "worker.h"
#include "lift.h"

volatile sig_atomic_t keep_running = 1;

void handle_shutdown() {
    keep_running = 0;
}


void run_skier_generator(int queue1_id, int queue2_id, SharedData* shared_data) {
    int skier_id = 1;
    
    while (keep_running) {
        if (shared_data->skier_count < MAX_SKIERS) {
            pid_t skier_pid = create_skier_process(skier_id++, queue1_id, queue2_id, shared_data);
            if (skier_pid > 0) {
                shared_data->skiers[shared_data->skier_count++] = skier_pid;
            }
        }

        // Random delay between new skiers
        sleep(randomInt(MIN_ARRIVAL_TIME, MAX_ARRIVAL_TIME + 1));

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

int main() {
    srand(time(NULL));
    
    // Set up signal handling
    signal(SIGINT, handle_shutdown);
    signal(SIGTERM, handle_shutdown);
    
    // Initialize shared memory
    int shmid = init_shared_memory();
    if (shmid == -1) {
        perror("Failed to initialize shared memory");
        exit(1);
    }

    SharedData* shared_data = attach_shared_memory(shmid);
    if (shared_data == (void*)-1) {
        perror("Failed to attach shared memory");
        exit(1);
    }

    // Initialize message queues
    int queue1_id = msgget(QUEUE_KEY_1, IPC_CREAT | 0666);
    int queue2_id = msgget(QUEUE_KEY_2, IPC_CREAT | 0666);
    
    if (queue1_id == -1 || queue2_id == -1) {
        perror("Failed to create message queues");
        exit(1);
    }

    printf("Created queues with IDs: queue1=%d, queue2=%d\n", queue1_id, queue2_id);


    if (init_lift() == NULL) {
        printf("Failed to initialize lift\n");
        // Cleanup
        msgctl(queue1_id, IPC_RMID, NULL);
        msgctl(queue2_id, IPC_RMID, NULL);
        detach_shared_memory(shared_data);
        remove_shared_memory(shmid);
        exit(1);
    }

    // Create cashier processes
    pid_t cashier1_pid = fork();
    if (cashier1_pid == 0) {
        run_cashier(1, queue1_id, shared_data);
        exit(0);
    }

    pid_t cashier2_pid = fork();
    if (cashier2_pid == 0) {
        run_cashier(2, queue2_id, shared_data);
        exit(0);
    }

    // Main loop - create skiers
    pid_t skier_generator = fork();
    if (skier_generator == 0) {
        run_skier_generator(queue1_id, queue2_id, shared_data);
        exit(0);
    }

    while (keep_running) {
        sleep(0);
    }

    printf("Sending terminate signal to all processes...\n");
    
    // Terminate cashier processes
    kill(cashier1_pid, SIGTERM);
    kill(cashier2_pid, SIGTERM);
    kill(skier_generator, SIGTERM);

    // Terminate all skier processes
    for (int i = 0; i < shared_data->skier_count; i++) {
        kill(shared_data->skiers[i], SIGTERM);
    }
    
    // Wait for all processes to finish
    printf("Waiting for processes to finish...\n");
    while (wait(NULL) > 0);
    
    // Cleanup resources
    detach_shared_memory(shared_data);
    remove_shared_memory(shmid);
    cleanup_ticket_queues(queue1_id, queue2_id);
    
    printf("Cleanup complete\n");
    return 0;
}