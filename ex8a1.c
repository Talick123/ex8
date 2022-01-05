// --------include section------------------------

#include <signal.h>
#include <stdio.h>
#include <unistd.h> //for pause
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdbool.h>

// --------const and enum section------------------------


// --------prototype section------------------------


// --------main section------------------------

int main()
{
    int shm_id;
    int *shm_ptr;

    signal(SIGUSR1, catch_sig1);

    create_shared_mem(&shm_id, &shm_ptr); //create shared memmory array
    init_data(shm_ptr);
    pause();
    read_and_print_data(shm_ptr);
    close_shared_mem(&shm_id, shm_ptr);

    return EXIT_SUCCESS;
}

//-------------------------------------------------

void catch_sig1(int signum){}

//-------------------------------------------------

void create_shared_mem(int *shm_id, int **shm_ptr)
{
    key_t key;
    key = ftok(".", '5');
    if(key == -1)
        perrorandexit("ftok failed");

    if((*shm_id = shmget(key, ARR_SIZE, IPC_CREAT | 0600)) == -1)
        perrorandexit("shmget failed");

    *shm_ptr = (int*)shmat(*shm_id, NULL, 0);
    if (!(*shm_ptr))
        perrorandexit("shmat failed");

    printf("Shm_id: %d\n", *shm_id);
}
