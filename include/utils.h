#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>

struct CheckoutQueues {
    int queue1_id;
    int queue2_id;
};

struct CheckoutQueues init_queues();
struct CheckoutQueues get_queues();

int randomInt(int min, int max);
bool random_chance(double probability);

#endif