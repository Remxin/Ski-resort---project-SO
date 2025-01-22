#ifndef SKIER_H
#define SKIER_H

#include <time.h>
#include "ticket.h"
#include "shared_memory.h"
#include <pthread.h>

enum ChildState {
    CHILD_READY = 0,
    WAIT_FOR_TICKET = 1,
    GOT_TICKET = 2,
    IN_QUEUE = 3
};

typedef struct {
    long mtype; // typ wiadomości - id dziecka
    int new_state;
} StateMessage;

typedef struct {
    long mtype;     // typ wiadomości = ID rodzica
    int child_id;   // ID dziecka
    int state;      // stan dziecka
} ChildMessage;

typedef struct {
    int id;
    int age;
    int parent_id;
    int is_vip;
    pthread_mutex_t mutex;
    int state;
    int msg_queue_id;
    Ticket ticket;
    SharedData* shared_data;
} Child;

typedef struct {
    int id;
    int age;
    int is_vip;
    int num_children;
    Child children[MAX_CHILDREN];
    int msg_queue_id;
    Ticket ticket;
} Skier;



typedef struct {
    long mtype;
    char mtext[100];
} msgbuf;

// Create new skier process
pid_t create_skier_process(int id, int queue1_id, int queue2_id, SharedData* shared_data);

// Create child processes with parent's ticket duration
void create_children(Skier* parent, SharedData* shared_data);

// Initialize skier data
Skier init_skier_data(int id, int is_child, int parent_ticket_duration);

// Buy ticket
int buy_ticket(Skier* skier, int queue_id);

// Buy ticket for child
int buy_child_ticket(Child *child, int queue_id);

// Komunikacja z dzieckiem
void send_message_to_child(Child* child, int new_state);

#endif