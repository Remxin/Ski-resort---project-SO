#ifndef SKIER_GENERATOR_H
#define SKIER_GENERATOR_H

#include <stdio.h>    // dla perror
#include <stdlib.h>   // dla random
#include <string.h>   // dla sprintf (nie spirntf - popraw literówkę)
#include "utils.h"    // zakładając że tam jest randomInt
#include "shared_memory.h"
#include "skier.h"

void handle_shutdown();
int main(int argc, char *argv[]);

#endif