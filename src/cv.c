//Christian Lovetere | U46489387 | Sec 001
//This program uses a circular buffer accessible by a consumer and producer thread. The producer thread puts chars in the buffer, and the consumer takes them out and prints them to
//stdout. Since they run in parallel, they use a conditional variable and mutex locks to ensure that they do not create a race condition and attempt to access similar spots in similar
//time, which would cause items to be skipped or printed multiple times. Input buffer can hold up to 150 chars, but strings of greater than 50 chars get cut off and only the first
//50 chars are serviced.

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define BUFFER_SIZE 15 //amount of chars storeable by the circular buffer
#define INPUT_SIZE 150 //overall maximum size of the input
#define MAX_INPUT_LENGTH 50 //maximum serviceable chars at once


typedef struct { //circular buffer structure
    char buffer[BUFFER_SIZE];
    int head; //stores the active location of the producer
    int tail; //stores the active location of the consumer
    int count; //stores the number of items in the buffer
    pthread_mutex_t mutex; //mutex for interactions between prod and cons
    pthread_cond_t not_empty; //cond var to indicate there's still room for the producer to write
    pthread_cond_t not_full; //cond var to indicate there's still room for the consumer to read
} CircularBuffer;

CircularBuffer cb; //instantiate global cb
char input[INPUT_SIZE]; //reserve stack space for the user input
int finished = 0; //indicates if the full string has been serviced

void* producer(void* arg) { //producer function. takes chars from user input and puts them in the cb
    int i = 0; //iterator
    printf("Input: %s\n", input); //print the user input after cutting it off at 50 chars
    printf("Count: %lu characters\n", strlen(input)); //print number of chars in the input
    while (input[i] != '\0') { //while we haven't reached the end of the string,
        pthread_mutex_lock(&cb.mutex); //lock the mutex so the consumer doesn't mess with our prod

        while (cb.count == BUFFER_SIZE) { //if the circular buffer is full
            pthread_cond_wait(&cb.not_full, &cb.mutex); //wait for some consumption to happen and free up space for more production
        }

        cb.buffer[cb.head] = input[i]; //put the current input char into the head position of the cb
        cb.head = (cb.head + 1) % BUFFER_SIZE; //move the head forward (wrap around to 0 if necessary)
        cb.count++; //increase the number of entries present in the cb
        
        printf("Produced: %c\n", input[i]);
        
        pthread_cond_signal(&cb.not_empty); //any time we've produced at all, we know the cb isn't empty afterwards.
        pthread_mutex_unlock(&cb.mutex); //unlock the mutex so we can do some consumption

        i++;
    }

    //signal to the consumer that production is finished
    pthread_mutex_lock(&cb.mutex);
    finished = 1;
    pthread_cond_signal(&cb.not_empty);
    pthread_mutex_unlock(&cb.mutex);

    printf("Producer: done\n");
    return NULL;
}

void* consumer(void* arg) { //consumer function. takes chars from cb and prints them to stdout
    while (1) {
        pthread_mutex_lock(&cb.mutex); //lock the mutex to prevent race conditions

        while (cb.count == 0 && !finished) { //while empty and there's more work,
            pthread_cond_wait(&cb.not_empty, &cb.mutex); //wait on 'not empty' cv
        }

        if (cb.count > 0) { //if not empty,
            char c = cb.buffer[cb.tail]; //get char at the tail
            cb.tail = (cb.tail + 1) % BUFFER_SIZE; //increment the tail
            cb.count--; //decrement num of stored, unserviced chars

            pthread_cond_signal(&cb.not_full); //since we just consumed, we signal that cb is not full
            pthread_mutex_unlock(&cb.mutex); //unlock the mutex

            printf("Consumed: %c\n", c);
        } else if (finished && cb.count == 0) {
            pthread_mutex_unlock(&cb.mutex);
            break;
        } else {
            pthread_mutex_unlock(&cb.mutex);
        }
    }
    printf("Consumer: done\n");
    return NULL;
}

int main() {
    pthread_t producer_thread, consumer_thread; //instantiate prod and cons threads

    cb.head = 0; //init cb
    cb.tail = 0;
    cb.count = 0;
    pthread_mutex_init(&cb.mutex, NULL); //init mutex and cvs
    pthread_cond_init(&cb.not_empty, NULL);
    pthread_cond_init(&cb.not_full, NULL);

    while (1) {
        printf("Enter input (type 'exit' to quit): ");
        fgets(input, INPUT_SIZE, stdin);

        
        size_t len = strlen(input); //get user input len
        if (len > 0 && input[len - 1] == '\n') { //remove newline character if present
            input[len - 1] = '\0';
            len--;
        }

        if (len > MAX_INPUT_LENGTH) { //limit input length to MAX_INPUT_LENGTH characters
            input[MAX_INPUT_LENGTH] = '\0';
            len = MAX_INPUT_LENGTH;
        }

        if (strcmp(input, "exit") == 0) { //check if the user wants to exit
            printf("Parent: done\n");
            break;
        }


        finished = 0; //reset finished flag and buffer count
        cb.head = 0;
        cb.tail = 0;
        cb.count = 0;

        //create producer and consumer threads
        pthread_create(&producer_thread, NULL, producer, NULL);
        pthread_create(&consumer_thread, NULL, consumer, NULL);

        //wait for the threads to finish
        pthread_join(producer_thread, NULL);
        pthread_join(consumer_thread, NULL);

        //ready for another iteration of while loop (ready for another string to service)
    }

    //cleanup mutexes and cvs
    pthread_mutex_destroy(&cb.mutex);
    pthread_cond_destroy(&cb.not_empty);
    pthread_cond_destroy(&cb.not_full);

    return 0;
}
