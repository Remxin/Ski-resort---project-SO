#ifndef WORKER_H
#define WORKER_H

#include "platform.h"
#include "shared_memory.h"

typedef struct {
    int id;
    Platform* platform;
    volatile int is_running;
    volatile int emergency_stop;
} Worker;

#endif