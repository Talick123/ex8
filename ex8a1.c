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
#include <semaphore.h>
#include <fcntl.h>
#include <time.h>



#define ARR_SIZE 1000
#define START 1

// --------const and enum section------------------------


// --------prototype section------------------------

void catch_sig1(int signum);
void create_shared_mem(int *shm_id, int **shm_ptr);
void init_data(int *shm_ptr);
void read_and_print_data(int *shm_ptr);
bool new_in_shm(int prime, int curr_ind, int *shm_ptr);
void close_shared_mem(int *shm_id, int *shm_ptr);
void perrorandexit(char *msg);

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
    key = ftok(".", '8');
    if(key == -1)
        perrorandexit("ftok failed");

    if((*shm_id = shmget(key, ARR_SIZE, IPC_CREAT | 0600)) == -1)
        perrorandexit("shmget failed");

    *shm_ptr = (int*)shmat(*shm_id, NULL, 0);
    if (!(*shm_ptr))
        perrorandexit("shmat failed");

    printf("Shm_id: %d\n", *shm_id);
}

//-------------------------------------------------

// get shm_ptr and reset it
void init_data(int *shm_ptr)
{
    int index;

    shm_ptr[0] = getpid();

    for(index = 1; index < ARR_SIZE; index++)
        shm_ptr[index] = 0;
}

//-------------------------------------------------

void read_and_print_data(int *shm_ptr)
{
    int index, counter = 1, max = shm_ptr[START], min = shm_ptr[START];

    for(index = START + 1; index < ARR_SIZE; index++)
    {
        if(new_in_shm(shm_ptr[index], index, shm_ptr))
            counter++;
        if(shm_ptr[index] < min)
            min = shm_ptr[index];
        if(shm_ptr[index] > max)
            max = shm_ptr[index];
    }

    printf("The number of different primes received is: %d\n", counter);
	printf("The max prime is: %d. The min prime is: %d\n", max, min);
}

//-------------------------------------------------

// gets prime and shared memmory and check if new in array
bool new_in_shm(int prime, int curr_ind, int *shm_ptr)
{
    int index;

    for(index = START; index < curr_ind; index++)
        if(prime == shm_ptr[index])
            return false;

    return true;
}

//-------------------------------------------------

void close_shared_mem(int *shm_id, int *shm_ptr)
{
	shmdt(shm_ptr);

    if(shmctl(*shm_id, IPC_RMID, NULL) == -1)
        perrorandexit("shmctl failed");
}


//-------------------------------------------------

void perrorandexit(char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}
