#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <time.h>
#include "ticket.h"
#include "config.h"
#include "utils.h"

int init_ticket_queues() {
    int queue1_id = msgget(QUEUE_KEY_1, IPC_CREAT | 0666);
    int queue2_id = msgget(QUEUE_KEY_2, IPC_CREAT | 0666);

    if (queue1_id == -1 || queue2_id == -1) {
        perror("Failed to create message queues");
        return -1;
    }

    return 0;
}

void cleanup_ticket_queues(int queue1_id, int queue2_id) {
    msgctl(queue1_id, IPC_RMID, NULL);
    msgctl(queue2_id, IPC_RMID, NULL);
}

double calculate_ticket_price(int is_vip, int is_discounted, int duration) {
    double base_price;
    
    // Set base price depending on duration
    switch (duration) {
        case TICKET_TIME_2H:
            base_price = NORMAL_PRICE_2H;
            break;
        case TICKET_TIME_4H:
            base_price = NORMAL_PRICE_4H;
            break;
        case TICKET_TIME_8H:
            base_price = NORMAL_PRICE_8H;
            break;
        case TICKET_TIME_12H:
            base_price = NORMAL_PRICE_12H;
            break;
        default:
            base_price = NORMAL_PRICE_2H; // Default to 2h price
    }
    
    // Apply VIP multiplier if applicable
    if (is_vip) {
        base_price *= VIP_MULTIPLIER;
    }
    
    // Apply age discount if applicable
    if (is_discounted) {
        base_price *= DISCOUNT_MULTIPLIER;
    }

    return base_price;
}