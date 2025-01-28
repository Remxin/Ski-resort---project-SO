#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/msg.h>
#include "ticket.h"
#include "shared_memory.h"
#include "config.h"
#include "utils.h"
#include "worker1.h"
#include "worker2.h"

volatile sig_atomic_t keep_running = 1;

void handle_shutdown() {
    keep_running = 0;
}


int main() {
    srand(time(NULL));
    // Czyszczenie pliku raportu
    FILE *file = fopen(TICKET_REPORT_FILE, "w");
    if (file == NULL) {
        perror("Cannot open report file for clearing");
    } else {
        fclose(file);
    }
    
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
    printf("Shared memory initialized with platform capacity: %d\n", MAX_PLATFORM_CAPACITY);
    printf("Main process shared memory: %p\n", (void*)shared_data);
    struct CheckoutQueues queues = init_queues();
    
    if (queues.queue1_id == -1 || queues.queue2_id == -1) {
        perror("Failed to create message queues");
        exit(1);
    }

    printf("Created queues with IDs: queue1=%d, queue2=%d\n", queues.queue1_id, queues.queue2_id);

    // Initialize platform
    if (init_platform() == NULL) {
        printf("Failed to initialize platform\n");
        // Cleanup
        msgctl(queues.queue1_id, IPC_RMID, NULL);
        msgctl(queues.queue2_id, IPC_RMID, NULL);
        detach_shared_memory(shared_data);
        remove_shared_memory(shmid);
        exit(1);
    }

    char cashier_id[10], queue_id[10], shmid_str[10];
    sprintf(shmid_str, "%d", shmid);
    // Create cashier processes
    pid_t cashier1_pid = fork();
    if (cashier1_pid == 0) {
        sprintf(cashier_id, "%d", 1);
        sprintf(queue_id, "%d", queues.queue1_id);
        execl("./bin/cashier", "cashier", cashier_id, queue_id, NULL);
        perror("Failed to create cashier 1");
        exit(EXIT_FAILURE);
    }

    pid_t cashier2_pid = fork();
    if (cashier2_pid == 0) {
        sprintf(cashier_id, "%d", 2);
        sprintf(queue_id, "%d", queues.queue2_id);
        execl("./bin/cashier", "cashier", cashier_id, queue_id, NULL);
        perror("Failed to create cashier 2");
        exit(EXIT_FAILURE);
    }

    pid_t worker1_pid = fork();
    if (worker1_pid == 0) {
        execl("./bin/worker1", "worker1", NULL);
        perror("Failed to create worker 1");
        exit(EXIT_FAILURE);
    }

     pid_t worker2_pid = fork();
    if (worker2_pid == 0) {
        execl("./bin/worker2", "worker2", NULL);
        perror("Failed to create worker 2");
        exit(EXIT_FAILURE);
    }

    // Main loop - create skiers
    pid_t skier_generator = fork();
    if (skier_generator == 0) {
        execl("./bin/skier_generator", "skier_generator", shmid_str, NULL);
        perror("Failed to create skier generator");
        exit(EXIT_FAILURE);
    }

    while (keep_running) {
        sleep(1);
    }

    printf("Sending terminate signal to all processes...\n");
    
    // Terminate cashier processes
    kill(cashier1_pid, SIGTERM);
    kill(cashier2_pid, SIGTERM);
    kill(worker1_pid, SIGTERM);
    kill(worker2_pid, SIGTERM);

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
    cleanup_ticket_queues(queues.queue1_id, queues.queue2_id);
    
    printf("Cleanup complete\n");
    return 0;
}