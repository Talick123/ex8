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

#define ARR_SIZE 50000
#define SEED 17
#define LOCK 0
#define START 1

// --------const and enum section------------------------

int arr[ARR_SIZE];
enum Lock {LOCKED, UNLOCKED};

// --------prototype section------------------------

void reset_arr();
void create_threads_and_wait();
void *fill_arr(void *arg);
bool prime(int num);
int count_appearances(int curr_ind);
void print_thread_data(int new_count, int max, int max_prime);
void read_and_print_data();
void perror_and_exit(char *msg);

// --------main section------------------------

int main()
{
    srand(SEED);
    reset_arr();
    arr[LOCK] = LOCKED;

    create_threads_and_wait();
    read_and_print_data();

    return EXIT_SUCCESS;
}

//-------------------------------------------------

void reset_arr()
{
    int i;

    for(i = 0; i < ARR_SIZE; i++)
        arr[i] = 0;
}

//-------------------------------------------------

void create_threads_and_wait()
{
    pthread_t thread1, thread2, thread3;
    int status;

    status = pthread_create(&thread1, NULL, fill_arr, NULL); //creating thread
    if(status != 0)
        perror_and_exit("p_thread_create\n");

    status = pthread_create(&thread2, NULL, fill_arr, NULL);
    if(status != 0)
        perror_and_exit("p_thread_create\n");

    status = pthread_create(&thread3, NULL, fill_arr, NULL);
    if(status != 0)
        perror_and_exit("p_thread_create\n");

	arr[LOCK] = UNLOCKED;

    pthread_join(thread1, NULL); //waiting for thread
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);
}

//-------------------------------------------------

void *fill_arr(void *arg)
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

            while(arr[LOCK] == LOCKED)
                sleep(1);

            arr[LOCK] = LOCKED;

            while(arr[index] != 0 && index < ARR_SIZE)
                index++;

            if(index >= ARR_SIZE)
            {
                arr[LOCK] = UNLOCKED; //gives other threads chance to see its full
                print_thread_data(new_count, max, max_prime);
                pthread_exit(NULL);
            }

            arr[index] = num;

            other = count_appearances(index);

            if(other == 0)
                new_count++;
            else if(max <= other);
            {
                max = other + 1;
                max_prime = num;
            }
            arr[LOCK] = UNLOCKED;
        }
    }
}


//-------------------------------------------------
//check is prime
bool prime(int num)
{
	if(num < 2)
		return false;

	int i;
	for(i = 2; i*i <= num; i++)
		if(num % i == 0)
			return false;

	return true;
}
//-------------------------------------------------

int count_appearances(int curr_ind)
{
    int counter = 0, index;

    for(index = START; index < curr_ind; index++)
        if(arr[index] == arr[curr_ind])
            counter++;

    return counter;
}

//-------------------------------------------------

void print_thread_data(int new_count, int max, int max_prime)
{
    printf("Thread %d sent %d different new primes.\n",
        pthread_self(), new_count);

    printf("The prime it sent most was %d, %d times. \n",
        	   max_prime, max);
}

//-------------------------------------------------

void read_and_print_data()
{
    int index, counter = 1, max = arr[START], min = arr[START];

    for(index = START + 1; index < ARR_SIZE; index++)
    {
        if(count_appearances(index) == 0)
            counter++;
        if(arr[index] < min)
            min = arr[index];
        if(arr[index] > max)
            max = arr[index];
    }
    printf("The number of different primes received is: %d\n", counter);
	printf("The max prime is: %d. The min prime is: %d\n", max, min);
}

//-------------------------------------------------

void perror_and_exit(char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}
