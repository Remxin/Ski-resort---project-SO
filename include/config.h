#ifndef CONFIG_H
#define CONFIG_H

// Ticket durations (in hours)
#define TICKET_TIME_2H 2
#define TICKET_TIME_4H 4
#define TICKET_TIME_8H 8
#define TICKET_TIME_12H 12

// Base ticket prices for different durations
#define NORMAL_PRICE_2H 50.0
#define NORMAL_PRICE_4H 90.0
#define NORMAL_PRICE_8H 160.0
#define NORMAL_PRICE_12H 200.0

#define VIP_MULTIPLIER 1.5  // VIP tickets are 50% more expensive
#define DISCOUNT_MULTIPLIER 0.75  // 25% discount

// Age limits
#define CHILD_MIN_AGE 3
#define CHILD_MAX_AGE 7
#define ADULT_MIN_AGE 18
#define ADULT_MAX_AGE 100
#define SENIOR_AGE 65
#define CHILD_DISCOUNT_AGE 12

// System limits
#define MAX_QUEUE_LENGTH 100
#define MAX_SKIERS 1000
#define MAX_CHILDREN 2

// Timing configurations (in seconds)
#define MIN_SERVICE_TIME 1
#define MAX_SERVICE_TIME 3
#define MIN_ARRIVAL_TIME 3
#define MAX_ARRIVAL_TIME 10

// IPC keys
#define SHM_KEY 12345
#define QUEUE_KEY_1 12346
#define QUEUE_KEY_2 12347

// VIP probability
#define VIP_PROBABILITY 0.2

#endif