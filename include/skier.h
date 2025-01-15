#ifndef SKIER_H
#define SKIER_H

#include <stdbool.h>
#include <pthread.h>


#include "config.h"
#include "utils.h"


// Structure to represent a skier
typedef struct {
    int id;
    int age;
    bool isVIP;           // Change to bool for VIP status
    int children_count;
    pthread_t* children;  // Threads for children (if skier has children)
} Skier;

// Function to create a skier
Skier* create_skier();


#endif // SKIER_H
