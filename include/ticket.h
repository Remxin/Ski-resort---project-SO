#ifndef TICKET_H
#define TICKET_H

#include <time.h>

typedef struct {
    int type;           // 1 - normal, 2 - VIP
    int duration;       // 2, 4, 8 or 12 hours
    double price;
    time_t valid_until;
} Ticket;

typedef struct {
    long mtype;
    int skier_id;
    int is_child;
    int age;
    int wants_vip;
    int parent_ticket_duration;  // Nowe pole dla czasu biletu rodzica
} TicketRequest;

typedef struct {
    long mtype;
    Ticket ticket;
    int status;         // 0 - success, 1 - error
} TicketResponse;

// Initialize ticket queues
int init_ticket_queues();

// Clean up ticket queues
void cleanup_ticket_queues(int queue1_id, int queue2_id);

// Calculate ticket price
double calculate_ticket_price(int is_vip, int is_discounted, int duration);

#endif