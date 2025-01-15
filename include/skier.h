#ifndef SKIER_H
#define SKIER_H

#include <time.h>
#include "ticket.h"
#include "shared_memory.h"

typedef struct {
    pid_t pid;
    int id;
    int age;
    time_t arrival_time;
    int is_vip;
    int num_children;
    Ticket ticket;
} Skier;

// Create new skier process
pid_t create_skier_process(int id, int queue1_id, int queue2_id, SharedData* shared_data);

// Create child processes
void create_children(int num_children, int parent_id, int is_vip, int queue1_id, int queue2_id, SharedData* shared_data);

// Initialize skier data
Skier init_skier_data(int id, int is_child, int parent_id, int inherit_vip);

// Buy ticket
int buy_ticket(Skier* skier, int queue_id);

#endif