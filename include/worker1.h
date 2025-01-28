#ifndef WORKER_H
#define WORKER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <semaphore.h>

#include "platform.h"
#include "config.h"
#include "shared_memory.h"

typedef struct {
    int id;
    Platform* platform;
    volatile int is_running;
    volatile int emergency_stop;
} Worker;


void sig_handler(int sig);
void init_worker();

#endif