/*
File: ex8a1.c ex8a2.c
Generate and Collect Primes from Shared Memory using Processes and Semaphore
=====================================================================
Written by: Tali Kalev, ID:208629691, Login: talikal
		and	Noga Levy, ID:315260927, Login: levyno

This program runs with 4 different processes. Three processes that generates
random numbers, when the number is prime the process sends it to adds to shared
memory while using semaphore to "lock" the other processes out.
When the filling processes sees that the shared memory is full, it sends signal to
main process, prints out data and ends. The main process wakes, counts how many
different primes are in shared memory, finds the smallest and biggest number,
prints, closes shared memory and ends.

Compile: gcc ex8a1.c -o ex8a1 -lpthread
         gcc ex8a2.c -o ex8a2 -lpthread
     (ex8a1 = main process, ex8a2 = sub process)

Run: for start run the main process.
    Then, run 3 times the sub processes and send to the vector
    arguments the number of process (1-3):
        ./ex8a1
        ./ex8a2 1
        ./ex8a2 2
        ./ex8a2 3

Input: No Input

Output:
    From main process (ex8a1) = minimum prime, max prime and number of
    different numbers in the array.
    Example: The number of different primes received is: 49987
             The max prime is: 2147429173. The min prime is: 22091
    From sub process (ex8a2) = prime number they send the most to main process
    Example: Process 1101373 sent 3488 different new primes.
             The prime it sent most was 1117382491, 1 times.

*/
// --------include section------------------------


#include <stdio.h>
#include <unistd.h> //for pause
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdbool.h>
#include <semaphore.h>
#include <fcntl.h>
#include <time.h>

#define ARR_SIZE 1000
#define START 4

// --------const and enum section------------------------

const int ARGC_SIZE = 2;

// --------prototype section------------------------

void check_argv(int argc);
void connect_to_shared_mem(int **shm_ptr);
void init_and_wait(int *shm_ptr, int id);
void fill_arr(int *shm_ptr, sem_t *mutex);
bool prime(int num);
int count_appearances(int *shm_ptr,int curr_ind);
void print_data(int new_count,int max,int max_prime);
void perrorandexit(char *msg);
void catch_sig2(int signum);

// --------main section------------------------

int main(int argc, char *argv[])
{
    int *shm_ptr;
    signal(SIGUSR2, catch_sig2);

    check_argv(argc);
    srand(atoi(argv[1]));

    sem_t* mutex; //global?

    mutex = sem_open("/semaphore", O_CREAT, 0644, 1);
    if(mutex == SEM_FAILED)
	{
        perrorandexit("parent sem_open failed\n");
	}

    connect_to_shared_mem(&shm_ptr);
    init_and_wait(shm_ptr, atoi(argv[1]));
    fill_arr(shm_ptr, mutex);

    return EXIT_SUCCESS;
}

//-------------------------------------------------

void check_argv(int argc)
{
	if(argc != ARGC_SIZE)
		perrorandexit("Error! Incorrect number of arguments");
}

//-------------------------------------------------
void catch_sig2(int signum)
{
	exit(EXIT_SUCCESS);
}

//-------------------------------------------------

void connect_to_shared_mem(int **shm_ptr)
{
    key_t key;
    int shm_id;

    key = ftok(".", '8');
    if(key == -1)
	{
        perrorandexit("ftok failed");
	}

    if((shm_id = shmget(key, ARR_SIZE, 0600)) == -1)
	{
        perrorandexit("shmget failed");
	}

    *shm_ptr = (int*)shmat(shm_id, NULL, 0);
    if (!(*shm_ptr))
        perrorandexit("shmat failed");

    printf("Shm_id: %d\n", shm_id);
}

//-------------------------------------------------

void init_and_wait(int *shm_ptr, int id)
{
    shm_ptr[id] = 1;

    while((shm_ptr[1] == 0) || (shm_ptr[2] == 0) || (shm_ptr[3] == 0))
        {sleep(0.1);}

}

//-------------------------------------------------

void fill_arr(int *shm_ptr, sem_t *mutex)
{
    int num, index, max, max_prime, new_count, other;
    max = max_prime = new_count = 0;
    index = START;


    while(true)
    {
        num = (rand() + 2);

        if(prime(num))
        {
			if(max_prime == 0)
			{
				max_prime = num;
				max++;
			}

            sem_wait(mutex);

            while(shm_ptr[index] != 0 && index < ARR_SIZE)
                index++;

            if(index >= ARR_SIZE)
            {
                kill(shm_ptr[0], SIGUSR1);
				sem_post(mutex);

                break;
            }

            shm_ptr[index] = num;

            other = count_appearances(shm_ptr, index);

            if(other == 0)
                new_count++;
            else if(max <= other)
            {
                max = other + 1;
                max_prime = num;
            }

            sem_post(mutex);

        }
    }
    sem_close(mutex);
    sem_unlink("/semaphore");
    shmdt(shm_ptr);
    print_data(new_count, max, max_prime);
}

//-------------------------------------------------
//check is prime
bool prime(int num)
{
	int i;

    if(num < 2)
		return false;

	for(i = 2; i*i <= num; i++)
		if(num % i == 0)
			return false;

	return true;
}

//-------------------------------------------------

int count_appearances(int *shm_ptr, int curr_ind)
{
    int counter = 0, index;

    for(index = 0; index < curr_ind; index++)
        if(shm_ptr[index] == shm_ptr[curr_ind])
            counter++;

    return counter;
}

//-------------------------------------------------

void print_data(int new_count,int max,int max_prime)
{
    printf("Process %d sent %d different new primes.",
        (int)getpid(), new_count);

     printf("The prime it sent most was %d, %d times \n",
        	   max_prime, max);
}

//-------------------------------------------------

void perrorandexit(char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}
