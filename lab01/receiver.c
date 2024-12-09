#include "receiver.h"
#include <fcntl.h>       // For O_* constants
#include <mqueue.h>      // For POSIX message queue
#include <sys/mman.h>    // For mmap
#include <sys/stat.h>    // For mode constants
#include <unistd.h>
#include <time.h>
// #include <linux/time.h>

#define SHM_NAME "/Lab01"
#define MQ_NAME "/Lab01_MQ"
#define SEM_SENDER "/sender_semaphore"
#define SEM_RECEIVER "/receiver_semaphore"
#define MAX_MSG_SIZE 1024

#define NONE "\033[m"
#define RED "\033[1;31m"
#define LIGHT_GREEN "\033[1;32m"
#define CYAN "\033[1;36m"


void receive(message_t* message_ptr, mailbox_t* mailbox_ptr){
    /*  TODO: 
        1. Use flag to determine the communication method
        2. According to the communication method, receive the message
    */
    if (mailbox_ptr->flag == 1){
        // message passing
        mqd_t mq = mq_open(MQ_NAME, O_RDONLY);
        mq_receive(mq, message_ptr->text, MAX_MSG_SIZE, NULL);
        mq_close(mq);
    }
    else if (mailbox_ptr->flag == 2){
        // shared memory
        strcpy(message_ptr->text, mailbox_ptr->storage.shm_addr);
    }
    
}

int main(int argc, char* argv[]){
    /*  TODO: 
        1) Call receive(&message, &mailbox) according to the flow in slide 4
        2) Measure the total receiving time
        3) Get the mechanism from command line arguments
            • e.g. ./receiver 1
        4) Print information on the console according to the output format
        5) If the exit message is received, print the total receiving time and terminate the receiver.c
    */
    int mechanism = atoi(argv[1]);
    mailbox_t mailbox;
    mailbox.flag = mechanism;

    if (mechanism == 1) {
        // message passing
        mq_open(MQ_NAME, O_CREAT | O_RDONLY, 0666, NULL);
        printf("Message Passing\n");
    } else if (mechanism == 2) {
        // shared memory
        int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
        mailbox.storage.shm_addr = mmap(0, MAX_MSG_SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
        printf("Shared Memory\n");
    }

    sem_t* semSender = sem_open(SEM_SENDER, 0);
    sem_t* semReceiver = sem_open(SEM_RECEIVER, 0);
    // int value;
    // sem_getvalue(semSender, &value);
    // printf("Sender Semaphore value at the beginning: %d\n", value);
    // sem_getvalue(semReceiver, &value);
    // printf("Receiver Semaphore value at the beginning: %d\n\n", value);
    struct timespec start, end;
    double time_taken = 0;
    message_t message;
    while (1) {  
        // sem_getvalue(semSender, &value);
        // printf("Sender Semaphore value before sending: %d\n", value);
        // sem_getvalue(semReceiver, &value);
        // printf("Receiver Semaphore value before sending: %d\n", value);   


        sem_wait(semReceiver);      // receiver訊號量-1
        clock_gettime(CLOCK_MONOTONIC, &start);
        receive(&message, &mailbox);
        clock_gettime(CLOCK_MONOTONIC, &end);
        sem_post(semSender);        // sender訊號量+1

        time_taken += (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) * 1e-9;
        
        if (strcmp(message.text, "exit") == 0) {
            printf(RED"\nSender exit!\n"NONE);
            printf("Total time taken in sending msg: %fs.\n", time_taken);
            break;
        }
        
        printf(LIGHT_GREEN"Receiving Message: "NONE);
        printf(CYAN"%s"NONE, message.text);

        
        // sem_getvalue(semSender, &value);
        // printf("Sender Semaphore value after sending: %d\n", value);
        // sem_getvalue(semReceiver, &value);
        // printf("Receiver Semaphore value after sending: %d\n\n", value);
        
    }
    sem_close(semReceiver);
    if (mechanism == 1) {
        mq_unlink(MQ_NAME);
    } else if (mechanism == 2) {
        munmap(mailbox.storage.shm_addr, MAX_MSG_SIZE);
        shm_unlink(SHM_NAME);
        sem_unlink(SEM_RECEIVER);
        sem_unlink(SEM_SENDER);
    }

    return 0;
}