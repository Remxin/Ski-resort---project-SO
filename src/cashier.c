#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/msg.h>
#include <time.h>
#include "cashier.h"
#include "ticket.h"
#include "config.h"
#include "utils.h"

void run_cashier(int cashier_id, int queue_id, SharedData* shared_data) {
    printf("Cashier %d started working\n", cashier_id);
    
    while (1) {
        TicketRequest request;
        TicketResponse response;
        
        // Receive ticket request
        if (msgrcv(queue_id, &request, sizeof(TicketRequest) - sizeof(long), 0, 0) == -1) {
            perror("msgrcv");
            continue;
        }

        // Simulate service time
        int service_time = randomInt(MIN_SERVICE_TIME, MAX_SERVICE_TIME + 1);
        sleep(service_time);

        // Prepare response
        response.mtype = request.skier_id;
        response.status = 0;

        // Create ticket
        Ticket ticket;
        ticket.type = request.wants_vip ? 2 : 1;
        
        // Random ticket duration
        int durations[] = {TICKET_TIME_2H, TICKET_TIME_4H, TICKET_TIME_8H, TICKET_TIME_12H};
        ticket.duration = durations[randomInt(0, 4)];

        // Calculate price with duration and discounts
        int is_discounted = (request.age < CHILD_DISCOUNT_AGE || request.age > SENIOR_AGE);
        ticket.price = calculate_ticket_price(request.wants_vip, is_discounted, ticket.duration);

        // Set validity time
        time_t current_time;
        time(&current_time);
        ticket.valid_until = current_time + (ticket.duration * 3600);

        response.ticket = ticket;

        // Send response
        if (msgsnd(queue_id, &response, sizeof(TicketResponse) - sizeof(long), 0) == -1) {
            perror("msgsnd");
            continue;
        }

        printf("Cashier %d: Sold %s ticket for %dh to customer %d (age: %d) for %.2f PLN\n",
               cashier_id,
               ticket.type == 2 ? "VIP" : "Normal",
               ticket.duration,
               request.skier_id,
               request.age,
               ticket.price);
    }
}