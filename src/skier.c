#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>  // dla strerror
#include <errno.h>   // dla errno
#include "skier.h"
#include "utils.h"
#include "config.h"

// Usuwamy globalną zmienną keep_running, bo koliduje z main.c
static volatile sig_atomic_t skier_running = 1;

void handle_signal(int signo) {
    (void)signo;  // Uciszenie ostrzeżenia o nieużywanym parametrze
    skier_running = 0;
}

// Używamy extern aby uniknąć konfliktu z main.c
extern volatile sig_atomic_t keep_running;

pid_t create_skier_process(int id, int queue1_id, int queue2_id, SharedData* shared_data) {
    pid_t pid = fork();
    
    if (pid == 0) {  // Child process (skier)
        Skier skier = init_skier_data(id, 0, 0, 0, 0);
        
        // Choose shorter queue and buy ticket
        int queue_number = get_shorter_queue(shared_data);
        int chosen_queue = (queue_number == 1) ? queue1_id : queue2_id;
        printf("Skier %d choosing queue %d (ID: %d)\n", id, queue_number, chosen_queue);
        
        update_queue_length(shared_data, queue_number, 1);
        
        // Must buy ticket first
        if (buy_ticket(&skier, chosen_queue) != 0) {
            printf("Skier %d failed to buy ticket from queue %d\n", id, chosen_queue);
            update_queue_length(shared_data, queue_number, -1);
            exit(1);
        }
        
        // ... reszta kodu ...

        
        update_queue_length(shared_data, queue_number, -1);

        // Jeśli dorosły, twórz dzieci
        if (skier.age >= ADULT_MIN_AGE) {
            int num_children = randomInt(0, MAX_CHILDREN + 1);
            skier.num_children = num_children;
            
            if (num_children > 0) {
                create_children(num_children, id, skier.is_vip, skier.ticket.duration, 
                              queue1_id, queue2_id, shared_data);
            }
        }

        // Próba wejścia na platformę
        while (enter_lower_platform(&shared_data->platform, skier.id) != 0) {
            if (!keep_running) {
                printf("Skier %d leaving due to shutdown\n", skier.id);
                exit(0);
            }
            sleep(1);
        }

        sleep(1); // Krótkie oczekiwanie przed zakończeniem
        exit(0);
    }
    
    return pid;
}

void create_children(int num_children, int parent_id, int is_vip, int parent_ticket_duration,
                    int queue1_id, int queue2_id, SharedData* shared_data) {
    for (int i = 0; i < num_children; i++) {
        pid_t child_pid = fork();
        
        if (child_pid == 0) {  // Child process
            Skier child = init_skier_data(parent_id * 100 + i + 1, 1, parent_id, is_vip, parent_ticket_duration);
            
            // Kupno biletu
            int queue_number = get_shorter_queue(shared_data);
            int chosen_queue = (queue_number == 1) ? queue1_id : queue2_id;
            
            update_queue_length(shared_data, queue_number, 1);
            
            if (buy_ticket(&child, chosen_queue) != 0) {
                printf("Child %d couldn't buy ticket\n", child.id);
                update_queue_length(shared_data, queue_number, -1);
                exit(1);
            }
            
            update_queue_length(shared_data, queue_number, -1);

            while (enter_lower_platform(&shared_data->platform, child.id) != 0) {
                if (!keep_running) {
                    printf("Child %d leaving due to shutdown\n", child.id);
                    exit(0);
                }
                sleep(1);
            }

            sleep(1); // Krótkie oczekiwanie przed zakończeniem
            exit(0);
        }
    }
}


int buy_ticket(Skier* skier, int queue_id) {
    TicketRequest request;
    TicketResponse response;
    
    request.mtype = 1;  // Typ żądania zawsze 1
    request.skier_id = skier->id + 1;  // ID musi być > 0 dla odpowiedzi
    request.is_child = (skier->age < ADULT_MIN_AGE);
    request.age = skier->age;
    request.wants_vip = skier->is_vip;
    request.parent_ticket_duration = skier->parent_ticket_duration;
    
    printf("Skier %d requesting ticket from queue %d\n", skier->id, queue_id);
    
   if (msgsnd(queue_id, &request, sizeof(TicketRequest) - sizeof(long), 0) == -1) {
        printf("Skier %d: msgsnd error for queue %d: %s\n", skier->id, queue_id, strerror(errno));
        return 1;
    }
    
    if (msgrcv(queue_id, &response, sizeof(TicketResponse) - sizeof(long), request.skier_id, 0) == -1) {
        printf("Skier %d: msgrcv error for queue %d: %s\n", skier->id, queue_id, strerror(errno));
        return 1;
    }
    
    if (response.status != 0) {
        printf("Skier %d received error status from cashier\n", skier->id);
        return 1;
    }
    
    skier->ticket = response.ticket;
    printf("Skier %d bought a %s ticket for %d hours for %.2f PLN\n", 
           skier->id, 
           skier->ticket.type == 2 ? "VIP" : "normal",
           skier->ticket.duration,
           skier->ticket.price);
    
    return 0;
}

Skier init_skier_data(int id, int is_child, int parent_id, int inherit_vip, int parent_ticket_duration) {
    (void)parent_id;  // Uciszenie ostrzeżenia o nieużywanym parametrze
    
    Skier skier;
    skier.pid = getpid();
    skier.id = id;
    
    if (is_child) {
        skier.age = randomInt(CHILD_MIN_AGE, CHILD_MAX_AGE + 1);
        skier.num_children = 0;
        skier.is_vip = inherit_vip;
        skier.parent_ticket_duration = parent_ticket_duration;
    } else {
        skier.age = randomInt(ADULT_MIN_AGE, ADULT_MAX_AGE + 1);
        skier.num_children = 0;
        skier.is_vip = random_chance(VIP_PROBABILITY);
        skier.parent_ticket_duration = 0;
    }
    
    skier.arrival_time = time(NULL);
    
    return skier;
}