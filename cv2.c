#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define BUFFER_SIZE 15
#define INPUT_SIZE 150

// Circular buffer structure
typedef struct {
    char buffer[BUFFER_SIZE];
    int head;
    int tail;
    int count;
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} CircularBuffer;

CircularBuffer cb;
char input[INPUT_SIZE];
int finished = 0;

void* producer(void* arg) {
    int i = 0;
    printf("Input: %s\n", input);
    printf("Count: %lu characters\n", strlen(input));
    while (input[i] != '\0') {
        pthread_mutex_lock(&cb.mutex);

        while (cb.count == BUFFER_SIZE) {
            pthread_cond_wait(&cb.not_full, &cb.mutex);
        }

        cb.buffer[cb.head] = input[i];
        cb.head = (cb.head + 1) % BUFFER_SIZE;
        cb.count++;
        
        printf("Produced: %c\n", input[i]);
        
        pthread_cond_signal(&cb.not_empty);
        pthread_mutex_unlock(&cb.mutex);

        i++;
    }

    // Signal the consumer that production is finished
    pthread_mutex_lock(&cb.mutex);
    finished = 1;
    printf("Producer: done\n");
    pthread_cond_signal(&cb.not_empty);
    pthread_mutex_unlock(&cb.mutex);

    return NULL;
}

void* consumer(void* arg) {
    while (1) {
        pthread_mutex_lock(&cb.mutex);

        while (cb.count == 0 && !finished) {
            pthread_cond_wait(&cb.not_empty, &cb.mutex);
        }

        if (cb.count > 0) {
            char c = cb.buffer[cb.tail];
            cb.tail = (cb.tail + 1) % BUFFER_SIZE;
            cb.count--;

            pthread_cond_signal(&cb.not_full);
            pthread_mutex_unlock(&cb.mutex);

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
    pthread_t producer_thread, consumer_thread;

    // Initialize the circular buffer
    cb.head = 0;
    cb.tail = 0;
    cb.count = 0;
    pthread_mutex_init(&cb.mutex, NULL);
    pthread_cond_init(&cb.not_empty, NULL);
    pthread_cond_init(&cb.not_full, NULL);

    while (1) {
        printf("Enter input (type 'exit' to quit): ");
        fgets(input, INPUT_SIZE, stdin);

        // Remove newline character if present
        size_t len = strlen(input);
        if (len > 0 && input[len - 1] == '\n') {
            input[len - 1] = '\0';
        }

        // Check if the user wants to exit
        if (strcmp(input, "exit") == 0) {
            printf("Parent: done\n");
            break;
        }

        // Reset finished flag and buffer count
        finished = 0;
        cb.head = 0;
        cb.tail = 0;
        cb.count = 0;

        // Create producer and consumer threads
        pthread_create(&producer_thread, NULL, producer, NULL);
        pthread_create(&consumer_thread, NULL, consumer, NULL);

        // Wait for both threads to finish
        pthread_join(producer_thread, NULL);
        pthread_join(consumer_thread, NULL);
    }

    // Cleanup
    pthread_mutex_destroy(&cb.mutex);
    pthread_cond_destroy(&cb.not_empty);
    pthread_cond_destroy(&cb.not_full);

    return 0;
}
