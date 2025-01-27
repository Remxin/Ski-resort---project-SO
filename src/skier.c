#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>
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

int main(int argc, char *argv[]) {
    srand(time(NULL));
    if (argc != 3) {
        printf("Wrong usage of skier process\n");
        return 1;
    }
    int id = atoi(argv[1]);
    int shmid = atoi(argv[2]);

    SharedData* shared_data = attach_shared_memory(shmid);
    if (shared_data == (void*)-1) {
        perror("Failed to attach shared memory");
        exit(1);
    }
    // Initialize message queues
    struct CheckoutQueues queues = get_queues();
    int queue1_id = queues.queue1_id, queue2_id = queues.queue2_id;
    
    if (queue1_id == -1 || queue2_id == -1) {
        perror("Failed to create message queues");
        exit(1);
    }
    // Child process (skier)
    Skier skier = init_skier_data(id, 0, 0, 0, 0);
    
    // Choose shorter queue and buy ticket
    int queue_number = get_shorter_queue(shared_data);
    int chosen_queue = (queue_number == 1) ? queue1_id : queue2_id;
    printf("Skier %d choosing queue %d (ID: %d)\n", id, queue_number, chosen_queue);

    // Jeśli dorosły, twórz dzieci
    for (int i = 0; i < skier.num_children; i++) {
        pthread_create(&skier.children[i], NULL, child_thread, NULL);
        skier.child_ids[i] = skier.id * 1000 + i + 1;
    }

    
    update_queue_length(shared_data, queue_number, 1);
    
    // Must buy ticket first
    if (buy_ticket(&skier, chosen_queue) != 0) {
        printf("Skier %d failed to buy ticket from queue %d\n", id, chosen_queue);
        update_queue_length(shared_data, queue_number, -1);
        exit(1);
    }


    for (int i = 0; i < skier.num_children; i++) {
        if (buy_child_ticket(chosen_queue, skier.child_ids[i], randomInt(CHILD_MIN_AGE, CHILD_MAX_AGE), skier.is_vip, skier.ticket.duration) != 0) {
            printf("Skier %d failed to buy ticket for child %d from queue %d\n", id, skier.child_ids[i], chosen_queue);
            update_queue_length(shared_data, queue_number, -1);
            exit(1);
        }
    }

    update_queue_length(shared_data, queue_number, -1);
    time_t time_now = time(NULL);

    
    while (skier.ticket.valid_until >= time_now) {
        enter_lower_platform(&shared_data->platform, skier.id, skier.is_vip, skier.num_children);
        sleep(5);
        exit_lower_platform(&shared_data->platform, skier.num_children);
    }

    return 0;
}

void *child_thread(void *arg) {
    (void)arg;
    return NULL;
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

int buy_child_ticket(int queue_id, int skier_id, int skier_age, int is_vip, int ticket_duration) {
    TicketRequest request;
    TicketResponse response;
    request.mtype = 1;  // Typ żądania zawsze 1
    request.skier_id = skier_id + 1;  // ID musi być > 0 dla odpowiedzi
    request.is_child = 1;
    request.age = skier_age;
    request.wants_vip = is_vip;
    request.parent_ticket_duration = ticket_duration;
    
    printf("Child %d requesting ticket from queue %d\n", skier_id, queue_id);
    
   if (msgsnd(queue_id, &request, sizeof(TicketRequest) - sizeof(long), 0) == -1) {
        printf("Child %d: msgsnd error for queue %d: %s\n", skier_id, queue_id, strerror(errno));
        return 1;
    }
    
    if (msgrcv(queue_id, &response, sizeof(TicketResponse) - sizeof(long), request.skier_id, 0) == -1) {
        printf("Child %d: msgrcv error for queue %d: %s\n", skier_id, queue_id, strerror(errno));
        return 1;
    }
    
    if (response.status != 0) {
        printf("Child %d received error status from cashier\n", skier_id);
        return 1;
    }
    
    printf("Child %d bought a %s ticket for %d hours for %.2f PLN\n", 
           skier_id, 
           response.ticket.type == 2 ? "VIP" : "normal",
           response.ticket.duration,
           response.ticket.price);
    
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
        skier.num_children = randomInt(0, MAX_CHILDREN + 1);
        skier.is_vip = random_chance(VIP_PROBABILITY);
        skier.parent_ticket_duration = 0;
    }
    
    skier.arrival_time = time(NULL);

    printf("Skier age %d, is_vip %d children %d\n", skier.age, skier.is_vip, skier.num_children);
    
    return skier;
}

void end_skier(Skier* skier, int times_used) {
    if (skier->num_children > 0) {
        for (int i = 0; i < skier->num_children; i++) {
            pthread_join(skier->children[i], NULL);
        }
    }
    report_ticket(skier, times_used);
}

void report_ticket(Skier* skier, int times_used) {
    FILE *file = fopen(TICKET_REPORT_FILE, "a");
    if (file == NULL) {
        perror("Cannot open file for adding ticket reports");
        return;
    }
    char type[6];
    char reduced[3];
    strcpy(type, skier->is_vip ? "VIP" : "normal");
    strcpy(reduced, skier->age < ADULT_MIN_AGE ? "YES" : "NO");
    fprintf(file, "Ticket ID: %d, type: %s, reduced: %s,\t\tused %d times\n", skier->id, type, reduced, times_used);
    for (int i = 0; i < skier->num_children; i++) {
        fprintf(file, "Ticket ID: %d, type: %s, reduced: %s,\t\tused %d times\n", skier->child_ids[i], type, reduced, times_used);
    }
    fclose(file);
}