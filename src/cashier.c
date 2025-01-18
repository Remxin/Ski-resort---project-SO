#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/msg.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <string.h>  // dla strerror

#include "cashier.h"
#include "ticket.h"
#include "config.h"
#include "utils.h"

volatile sig_atomic_t cashier_running = 1;

void handle_cashier_shutdown(int sig) {
    (void)sig;  // Uciszenie ostrzeżenia o nieużywanym parametrze
    cashier_running = 0;
}

void run_cashier(int cashier_id, int queue_id, SharedData* shared_data) {
    (void)shared_data;
    signal(SIGTERM, handle_cashier_shutdown);
    signal(SIGINT, handle_cashier_shutdown);
    
    printf("Cashier %d started working\n", cashier_id);
    
    while (cashier_running) {
        TicketRequest request;
        TicketResponse response;
        
        ssize_t recv_size = msgrcv(queue_id, &request, sizeof(TicketRequest) - sizeof(long), 0, IPC_NOWAIT);
        
        if (recv_size == -1) {
            if (errno == ENOMSG) {
                usleep(100000);
                continue;
            }
            if (errno == EINTR) {
                if (!cashier_running) break;
                continue;
            }
            perror("msgrcv error");
            continue;
        }

        // Prepare response
        response.mtype = request.skier_id;
        response.status = 0;

        // Set duration
        int durations[] = {TICKET_TIME_2H, TICKET_TIME_4H, TICKET_TIME_8H, TICKET_TIME_12H};
        if (request.parent_ticket_duration > 0) {
            response.ticket.duration = request.parent_ticket_duration;
        } else {
            response.ticket.duration = durations[randomInt(0, 4)];
        }

        // Set type and calculate price
        response.ticket.type = request.wants_vip ? 2 : 1;
        int is_discounted = (request.age < CHILD_DISCOUNT_AGE || request.age > SENIOR_AGE);
        response.ticket.price = calculate_ticket_price(
            request.wants_vip, 
            is_discounted, 
            response.ticket.duration
        );

        // Set validity time
        time_t current_time;
        time(&current_time);
        response.ticket.valid_until = current_time + (response.ticket.duration * 3600);

        if (msgsnd(queue_id, &response, sizeof(TicketResponse) - sizeof(long), 0) == -1) {
            perror("msgsnd error");
            continue;
        }

        printf("Cashier %d: Sold %s ticket for %dh to %s %d for %.2f PLN\n",
               cashier_id,
               response.ticket.type == 2 ? "VIP" : "normal",
               response.ticket.duration,
               request.parent_ticket_duration > 0 ? "child" : "skier",
               request.skier_id,
               response.ticket.price);
    }
    
    printf("Cashier %d finishing work\n", cashier_id);
}