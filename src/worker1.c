#include "worker1.h"
#include "utils.h"

SharedData* shm_ptr;

int main() {

    printf("Worker1: Start working\n");
    signal(SIGINT, SIG_IGN);
    signal(SIGUSR2, sig_handler);
    signal(SIGUSR2, sig_handler);

    int shmid = shmget(SHM_KEY, sizeof(SharedData), 0666);
    if (shmid == -1) {
        perror("Worker 1 shmget failed");
        exit(1);
    }
    SharedData* shm_ptr = attach_shared_memory(shmid);
    if (shm_ptr == (void*)-1) {
        perror("Worker 1 shmat failed");
        exit(1);
    }
    
    Platform* platform = &shm_ptr->platform;

    int chair_index = 0;
    int skiers_on_chair;

    while(shm_ptr->is_running) {
        if (shm_ptr->is_paused) {
            continue;
        }
        sleep(WAIT_FOR_CHAIR);
        
        // Sprawdź ile osób może wejść (max 3)
        for(skiers_on_chair = 0; skiers_on_chair < 3; skiers_on_chair++) {
            if (sem_wait(&platform->upper_exits[chair_index % 2]) == -1) {
                break;
            }
        }

        if (skiers_on_chair > 0) {
            // Aktualizuj licznik platformy
            platform->upper_platform_count -= skiers_on_chair;
            
            // Zapisz liczbę osób na krzesełku
            shm_ptr->chair_array[chair_index] = skiers_on_chair;
            
            printf("Worker1: loaded %d skiers onto chair %d\n", 
                skiers_on_chair, chair_index);
            
            // Zwolnij miejsca na platformie
            for(int i = 0; i < skiers_on_chair; i++) {
                sem_post(&platform->platform_capacity);
            }
        }

        chair_index = (chair_index + 1) % MAX_CHAIRS;
        usleep(100000);
        
    }

    printf("Worker1: Finished working\n");
    shmdt(shm_ptr);
    return 0;
}

void sig_handler(int sig) {
    if (sig == SIGUSR1) {
        printf("Worker1: Pausing lift\n");
        shm_ptr->lower_ready = 0;
        shm_ptr->upper_ready = 0;
        shm_ptr->is_paused = 1;
    }
    else if (sig == SIGUSR2) {
        printf("Worker1: Ready to resume work\n");
        shm_ptr->lower_ready = 1;
        while(1) {
            if(shm_ptr->upper_ready) {
                printf("Worker1: Lift resuming work\n");
                shm_ptr->is_paused = 0;
                break;
            }
            usleep(100000);
        }
    }
}
