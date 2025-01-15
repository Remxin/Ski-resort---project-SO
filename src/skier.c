#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include "skier.h"
#include "utils.h"
#include "config.h"

pid_t create_skier_process(int id, int queue1_id, int queue2_id, SharedData* shared_data) {
    pid_t pid = fork();
    
    if (pid == 0) {  // Child process (skier)
        // Initialize skier data
        Skier skier = init_skier_data(id, 0, 0, 0);
        
        // Choose shorter queue
        int queue_number = get_shorter_queue(shared_data);
        int chosen_queue = (queue_number == 1) ? queue1_id : queue2_id;
        
        // Update queue length
        update_queue_length(shared_data, queue_number, 1);
        
        // Buy ticket
        if (buy_ticket(&skier, chosen_queue) != 0) {
            printf("Skier %d couldn't buy ticket\n", id);
            update_queue_length(shared_data, queue_number, -1);
            exit(1);
        }
        
        // Update queue length after purchase
        update_queue_length(shared_data, queue_number, -1);
        
        // If adult, possibly create children
        if (skier.age >= ADULT_MIN_AGE) {
            int num_children = randomInt(0, MAX_CHILDREN + 1);
            skier.num_children = num_children;
            
            if (num_children > 0) {
                create_children(num_children, id, skier.is_vip, queue1_id, queue2_id, shared_data);
            }
        }
        
        // Simulate skier activities until ticket expires
        while (1) {
            sleep(randomInt(1, 5));
            
            // Check ticket validity
            time_t current_time;
            time(&current_time);
            if (current_time > skier.ticket.valid_until) {
                printf("Skier %d ticket expired\n", id);
                break;
            }
        }
        
        exit(0);
    }
    
    return pid;
}

void create_children(int num_children, int parent_id, int is_vip, int queue1_id, int queue2_id, SharedData* shared_data) {
    for (int i = 0; i < num_children; i++) {
        pid_t child_pid = fork();
        
        if (child_pid == 0) {  // Child process
            Skier child = init_skier_data(parent_id * 100 + i + 1, 1, parent_id, is_vip);
            
            // Choose shorter queue
            int queue_number = get_shorter_queue(shared_data);
            int chosen_queue = (queue_number == 1) ? queue1_id : queue2_id;
            
            // Update queue length
            update_queue_length(shared_data, queue_number, 1);
            
            // Buy ticket
            if (buy_ticket(&child, chosen_queue) != 0) {
                printf("Child %d couldn't buy ticket\n", child.id);
                update_queue_length(shared_data, queue_number, -1);
                exit(1);
            }
            
            // Update queue length after purchase
            update_queue_length(shared_data, queue_number, -1);
            
            // Simulate child activities
            while (1) {
                sleep(randomInt(1, 5));
                
                // Check ticket validity
                time_t current_time;
                time(&current_time);
                if (current_time > child.ticket.valid_until) {
                    printf("Child %d ticket expired\n", child.id);
                    break;
                }
            }
            
            exit(0);
        }
    }
}

Skier init_skier_data(int id, int is_child, int parent_id, int inherit_vip) {
    Skier skier;
    skier.pid = getpid();
    skier.id = id;
    
    if (is_child) {
        skier.age = randomInt(CHILD_MIN_AGE, CHILD_MAX_AGE + 1);
        skier.num_children = 0;
        skier.is_vip = inherit_vip;
    } else {
        skier.age = randomInt(ADULT_MIN_AGE, ADULT_MAX_AGE + 1);
        skier.num_children = 0;
        skier.is_vip = random_chance(VIP_PROBABILITY);
    }
    
    skier.arrival_time = time(NULL);
    
    return skier;
}

int buy_ticket(Skier* skier, int queue_id) {
    TicketRequest request;
    TicketResponse response;
    
    // Prepare request
    request.mtype = 1;
    request.skier_id = skier->id;
    request.is_child = (skier->age < ADULT_MIN_AGE);
    request.age = skier->age;
    request.wants_vip = skier->is_vip;
    
    // Send request
    if (msgsnd(queue_id, &request, sizeof(TicketRequest) - sizeof(long), 0) == -1) {
        perror("msgsnd");
        return 1;
    }
    
    // Receive response
    if (msgrcv(queue_id, &response, sizeof(TicketResponse) - sizeof(long), skier->id, 0) == -1) {
        perror("msgrcv");
        return 1;
    }
    
    if (response.status != 0) {
        return 1;
    }
    
    // Save ticket
    skier->ticket = response.ticket;
    
    return 0;
}