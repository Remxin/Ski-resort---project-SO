#include "worker1.h"
#include <pthread.h>
#include "utils.h"

SharedData* shm_ptr;
static volatile sig_atomic_t worker_running = 1;


int main() {
    printf("\033[35m\033[45mWorker1: Start working\033[0m\n");
    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);
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

    int chair_index = 0;
    int skiers_on_chair;
    int queue_value;

    while(shm_ptr->is_running && worker_running) {
        if (shm_ptr->is_paused) {
            usleep(100000);
            continue;
        }
        sleep(WAIT_FOR_CHAIR);
        pthread_mutex_lock(&platform->queue_mutex);
        
         if (platform->lower_platform_count > 0) {
            sem_getvalue(&platform->chair_queue, &queue_value);

            // Oblicz ile osób wsiada na krzesełko
            if (queue_value < CHAIR_SIZE) {
                // Obliczamy ile osób jest na krzesełku
                skiers_on_chair = CHAIR_SIZE - queue_value;
                
                printf("\033[35mWorker1: Sending chair %d with %d skiers\033[0m\n", 
                        chair_index, skiers_on_chair);
                
                // Zapisujemy liczbę narciarzy
                shm_ptr->chair_array[chair_index] = skiers_on_chair;
                
                // Aktualizujemy licznik peronu
                platform->lower_platform_count -= skiers_on_chair;
                
                // Przygotowujemy nowe krzesełko
                for(int i = 0; i < CHAIR_SIZE; i++) {
                    sem_post(&platform->chair_queue);
                }
                for (int i = 0; i < skiers_on_chair; i++) {
                    sem_post(&platform->platform_capacity);
                }
                    
            }
        }
        pthread_mutex_unlock(&platform->queue_mutex);
        usleep(100000);
        chair_index = (chair_index + 1) % MAX_CHAIRS;
        
    }

    printf("\033[35m\033[45mWorker1: Finished working\033[0m\n");
    shmdt(shm_ptr);
    return 0;
}

void sig_handler(int sig) {
    if (sig == SIGUSR1) {
        printf("\033[31mWorker1: Pausing lift\033[0m\n");
        shm_ptr->lower_ready = 0;
        shm_ptr->upper_ready = 0;
        shm_ptr->is_paused = 1;
    }
    else if (sig == SIGUSR2) {
        printf("\033[31mWorker1: Ready to resume work\033[0m\n");
        shm_ptr->lower_ready = 1;
        while(1) {
            if(shm_ptr->upper_ready) {
                printf("\033[31mWorker1: Lift resuming work\033[0m\n");
                shm_ptr->is_paused = 0;
                break;
            }
            usleep(100000);
        }
    } else if (sig == SIGINT || sig == SIGTERM) {
        worker_running = 0;
    }
}
