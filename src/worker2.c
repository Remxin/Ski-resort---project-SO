#include "worker2.h"
#include "config.h"

SharedData* shm_ptr;

int main() {
    printf("\033[35m\033[45mWorker2: Start working\033[0m\n");
    signal(SIGINT, SIG_IGN);
    signal(SIGUSR1, sig_handler);
    signal(SIGUSR2, sig_handler);
    
    int shmid = shmget(SHM_KEY, sizeof(SharedData), 0666);
    if (shmid == -1) {
        perror("Worker 1 shmget failed");
        exit(1);
    }
    shm_ptr = attach_shared_memory(shmid);
    if (shm_ptr == (void*)-1) {
        perror("Worker 1 shmat failed");
        exit(1);
    }
    Platform* platform = &shm_ptr->platform;
    
    int chair_index = MAX_CHAIRS / 2;  // zaczynamy od połowy krzesełek
    int skiers_on_chair;

    while(shm_ptr->is_running) {
        if (!shm_ptr->is_paused) {
            sleep(WAIT_FOR_CHAIR);

            // Sprawdź czy są narciarze na krzesełku do rozładowania
            if (shm_ptr->chair_array[chair_index] > 0) {
                skiers_on_chair = shm_ptr->chair_array[chair_index];
                shm_ptr->chair_array[chair_index] = 0;  // opróżnij krzesełko
                
                printf("\033[35mWorker2: unloaded chair %d with %d skiers\033[0m\n", 
                       chair_index, skiers_on_chair);
                
                // Pozwól narciarzom zjechać - dodaj wejścia na dolny peron
                for(int i = 0; i < skiers_on_chair; i++) {
                    sem_post(&platform->exit_chair_queue);
                }
            }

            usleep(100000);
            
            // Przejdź do następnego krzesełka
            chair_index = (chair_index + 1) % MAX_CHAIRS;
        }
    }

    printf("Worker2: Finished working\n");
    shmdt(shm_ptr);
    return 0;
}

void sig_handler(int sig) {
    if (sig == SIGUSR1) {
        printf("Worker2: Pausing lift\n");
        shm_ptr->upper_ready = 0;
        shm_ptr->lower_ready = 0;
        shm_ptr->is_paused = 1;
    }
    else if (sig == SIGUSR2) {
        printf("Worker2: Ready to resume work\n");
        shm_ptr->upper_ready = 1;
        while(1) {
            if(shm_ptr->lower_ready) {
                printf("Worker2: Lift resuming work\n");
                shm_ptr->is_paused = 0;
                break;
            }
            usleep(100000);
        }
    }
}
