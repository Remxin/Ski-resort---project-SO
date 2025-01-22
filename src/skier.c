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

int create_message_queue(int skier_id) {
    key_t key = ftok("/tmp", skier_id);
    int msg_queue_id = msgget(key, IPC_CREAT | 0666);
    if (msg_queue_id == -1) {
        perror("Błąd tworzenia kolejki komunikatów");
        exit(1);
    }
    return msg_queue_id;
}

pid_t create_skier_process(int id, int queue1_id, int queue2_id, SharedData* shared_data) {
    pid_t pid = fork();
    
    if (pid != 0) {
            return pid;
    }
    // Child process (skier)
    Skier skier = init_skier_data(id, 0, 0, 0, 0);
    skier.msg_queue_id = create_message_queue(skier.id);
    

    // Jeśli dorosły, twórz dzieci
    if (skier.age >= ADULT_MIN_AGE) {
        int num_children = randomInt(0, MAX_CHILDREN + 1);
        skier.num_children = num_children;
        
        if (num_children > 0) {
            create_children(&skier, shared_data);
        }
    }
        // Choose shorter queue and buy ticket
    int queue_number = get_shorter_queue(shared_data);
    int chosen_queue = (queue_number == 1) ? queue1_id : queue2_id;

    printf("Skier %d (age %d) choosing queue %d (ID: %d) (children %d)\n", id, skier.age, queue_number, chosen_queue, skier.num_children);
    
    printf("Parent %d: waiting for %d children to be ready\n", skier.id, skier.num_children);
    for (int i = 0; i < skier.num_children; i++) {
        ChildMessage ready_msg;
        msgrcv(skier.msg_queue_id, &ready_msg, sizeof(ChildMessage) - sizeof(long), skier.id, 0);
        printf("Parent %d: received ready message from child %d\n",skier.id, ready_msg.child_id);
    }
    printf("Parent %d: all children are ready\n", skier.id);
    
    update_queue_length(shared_data, queue_number, 1 + skier.num_children);
    
    // Must buy ticket first
    if (buy_ticket(&skier, chosen_queue) != 0) {
        printf("Skier %d failed to buy ticket from queue %d\n", id, chosen_queue);
        update_queue_length(shared_data, queue_number, -1);
        exit(1);
    }
    
    // buy ticket for children
    for (int i = 0; i < skier.num_children; i++) {
        printf("Skier %d wants to buy ticket for child %d\n\n", skier.id, skier.children[i].id);
        if (buy_child_ticket(&skier.children[i], chosen_queue) != 0) {
            printf("Skier %d faild to buy ticket for child %d from queue %d\n", skier.id, skier.children[i].id, chosen_queue);
            update_queue_length(shared_data, queue_number, -1);
            exit(1);
        }
        send_message_to_child(&skier.children[i], GOT_TICKET);
    }
    

    update_queue_length(shared_data, queue_number, -(1 + skier.num_children));

    printf("Parent %d: shared_data address: %p\n", skier.id, (void*)shared_data);
    // Próba wejścia na platformę
    while (enter_lower_platform(&shared_data->platform, skier.id) != 0) {
        if (!keep_running) {
            printf("Skier %d leaving due to shutdown\n", skier.id);
            exit(0);
        }
        sleep(1);
    }

    sleep(10); // Krótkie oczekiwanie przed zakończeniem
    exit(0);
    
    

}

void send_message_to_child(Child* child, int new_state) {
    StateMessage msg;
    msg.mtype = child->id;          // ID dziecka jako typ wiadomości
    msg.new_state = new_state;      // Nowy stan
    
    if (msgsnd(child->msg_queue_id, &msg, sizeof(StateMessage) - sizeof(long), 0) == -1) {
        perror("Błąd wysyłania wiadomości");
    } 
}

void create_children(Skier* parent, SharedData* shared_data) {
    pid_t children_pids[MAX_CHILDREN];
    
    for (int i = 0; i < parent->num_children; i++) {
        parent->children[i].age = randomInt(CHILD_MIN_AGE, CHILD_MAX_AGE);
        parent->children[i].is_vip = parent->is_vip;
        parent->children[i].id = 1000 + (parent->id * 10) + i;
        parent->children[i].state = 0;
        parent->children[i].msg_queue_id = parent->msg_queue_id;
        parent->children[i].shared_data = shared_data;
        pthread_mutex_init(&parent->children[i].mutex, NULL);

        pid_t child_pid = fork();
        if (child_pid == 0) { // Proces dziecka
            Child* child = &parent->children[i];



            ChildMessage ready_message;
            ready_message.mtype = parent->id;
            ready_message.child_id = child->id;
            ready_message.state = CHILD_READY;
            msgsnd(child->msg_queue_id, &ready_message, sizeof(ChildMessage) - sizeof(long), 0);
            StateMessage msg;

            printf("Child %d with parent %d created\n", child->id, parent->id);
            while (1) {
                if (msgrcv(child->msg_queue_id, &msg, sizeof(StateMessage) - sizeof(long), child->id, 0) > 0) {
                    pthread_mutex_lock(&child->mutex);
                    child->state = msg.new_state;
                    switch(child->state) {
                    case GOT_TICKET:
                        printf("Child %d (parent %d): got ticket. Child state: %dhalooo\n", 
                            child->id, parent->id, child->state);
                        // Próba wejścia na platformę
                        while (enter_lower_platform(&shared_data->platform, child->id) != 0) {
                            if (!keep_running) {
                                printf("Child %d leaving due to shutdown\n", child->id);
                                exit(0);
                            }
                            sleep(1);
                        }
                        sleep(10);
                        break;
                    }
                    pthread_mutex_unlock(&child->mutex);
                }

            
            }
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

int buy_child_ticket(Child *child, int queue_id) {
     TicketRequest request;
    TicketResponse response;
    
    request.mtype = 1;  // Typ żądania zawsze 1
    request.skier_id = child->id + 1;  // ID musi być > 0 dla odpowiedzi
    request.is_child = 1;
    request.age = child->age;
    request.wants_vip = child->is_vip;
    printf("Child %d requesting ticket from queue %d\n", child->id, queue_id);
    
   if (msgsnd(queue_id, &request, sizeof(TicketRequest) - sizeof(long), 0) == -1) {
        printf("Skier %d: msgsnd error for queue %d: %s\n", child->id, queue_id, strerror(errno));
        return 1;
    }
    
    if (msgrcv(queue_id, &response, sizeof(TicketResponse) - sizeof(long), request.skier_id, 0) == -1) {
        printf("Skier %d: msgrcv error for queue %d: %s\n", child->id, queue_id, strerror(errno));
        return 1;
    }
    
    if (response.status != 0) {
        printf("Skier %d received error status from cashier\n", child->id);
        return 1;
    }
    
    child->ticket = response.ticket;
    printf("Child %d bought a %s ticket for %d hours for %.2f PLN\n", 
           child->id, 
           child->ticket.type == 2 ? "VIP" : "normal",
           child->ticket.duration,
           child->ticket.price);
    
    return 0;
}

Skier init_skier_data(int id, int is_child, int parent_id, int is_vip, int parent_ticket_duration) {
    Skier skier;
    skier.id = id;
    skier.is_vip = is_vip;
    skier.num_children = 0;
    
    if (is_child) {
        skier.age = randomInt(5, 18);
        skier.ticket.duration = parent_ticket_duration;
    } else {
        skier.age = randomInt(18, 60);
        skier.ticket.duration = randomInt(1, 5);
    }
    
    return skier;
}