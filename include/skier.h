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
    int child_ids[MAX_CHILDREN];
    pthread_t children[MAX_CHILDREN];
    int parent_ticket_duration;  // Nowe pole dla dziedziczenia czasu biletu
    Ticket ticket;
} Skier;

// Create new skier process
int main(int argc, char *argv[]);

void *child_thread(void *arg);

// Create child processes with parent's ticket duration
void create_children(int num_children, int parent_id, int is_vip, int parent_ticket_duration, int queue1_id, int queue2_id, SharedData* shared_data);

// Initialize skier data
Skier init_skier_data(int id, int is_child, int parent_id, int inherit_vip, int parent_ticket_duration);

// Buy ticket
int buy_ticket(Skier* skier, int queue_id);
int buy_child_ticket(int queue_id, int skier_id, int skier_age, int is_vip, int ticket_duration);

void end_skier(Skier* skier, int times_used);
void report_ticket(Skier* skier, int times_used);

void *child_thread(void *arg);
#endif