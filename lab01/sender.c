#include "sender.h"
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

void send(message_t message, mailbox_t* mailbox_ptr){
    /*  TODO: 
        1. Use flag to determine the communication method
        2. According to the communication method, send the message
    */
    if (mailbox_ptr->flag == 1){
        // message passing
        mqd_t mq = mq_open(MQ_NAME, O_WRONLY);
        mq_send(mq, message.text, strlen(message.text) + 1, 0);
        mq_close(mq);
    }
    else if (mailbox_ptr->flag == 2){
        // shared memory
        strcpy(mailbox_ptr->storage.shm_addr, message.text);
    }
}

int main(int argc, char* argv[]){
    /*  TODO: 
        1) Call send(message, &mailbox) according to the flow in slide 4
        2) Measure the total sending time
        3) Get the mechanism and the input file from command line arguments
            • e.g. ./sender 1 input.txt
                    (1 for Message Passing, 2 for Shared Memory)
        4) Get the messages to be sent from the input file
        5) Print information on the console according to the output format
        6) If the message form the input file is EOF, send an exit message to the receiver.c
        7) Print the total sending time and terminate the sender.c
    */
    if (argc < 3) {
        printf("Input Invalid!\n");
        return 1;
    }

    int mechanism = atoi(argv[1]);
    char* input_file = argv[2];
    FILE* file = fopen(input_file, "r");
    
    if (!file) {
        perror("Error opening file");
        return 1;
    }

    mailbox_t mailbox;
    mailbox.flag = mechanism;

    if (mechanism == 1) {
        // message passing
        printf("Message Passing\n");
        struct mq_attr attr;
        attr.mq_flags = 0;
        attr.mq_maxmsg = 1;
        attr.mq_msgsize = MAX_MSG_SIZE;
        attr.mq_curmsgs = 0;
        mq_open(MQ_NAME, O_CREAT | O_WRONLY, 0666, &attr);

    } else if (mechanism == 2) {
        // shared memory
        printf("Shared Memory\n");
        int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);

        if (shm_fd == -1) {
        perror("Error opening shared memory");
        return 1;
        }

        // 檢查 ftruncate 是否成功
        if (ftruncate(shm_fd, MAX_MSG_SIZE) == -1) {
            perror("Error truncating shared memory");
            close(shm_fd); // 如果失敗，關閉文件描述符
            return 1;
        }

        mailbox.storage.shm_addr = mmap(0, MAX_MSG_SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0);
        if (mailbox.storage.shm_addr == MAP_FAILED) {
            perror("Error mapping shared memory");
            close(shm_fd); // 如果映射失敗，關閉文件描述符
            return 1;
        }
    }

    sem_t* semSender = sem_open(SEM_SENDER, O_CREAT, 0666, 1);
    sem_t* semReceiver = sem_open(SEM_RECEIVER, O_CREAT, 0666, 0);
    // int value;
    // sem_getvalue(semSender, &value);
    // printf("Sender Semaphore value before sending: %d\n", value);
    // sem_getvalue(semReceiver, &value);
    // printf("Receiver Semaphore value before sending: %d\n\n", value);


    // sleep(2);
    struct timespec start, end;
    double time_taken = 0;

    message_t message;
    while(fgets(message.text, MAX_MSG_SIZE, file)) {    
        // sem_getvalue(semSender, &value);
        // printf("Sender Semaphore value before sending: %d\n", value);
        // sem_getvalue(semReceiver, &value);
        // printf("Receiver Semaphore value before sending: %d\n", value); 


        // Send the message
        sem_wait(semSender);            // sender訊號量-1
        clock_gettime(CLOCK_MONOTONIC, &start);
        send(message, &mailbox);   
        clock_gettime(CLOCK_MONOTONIC, &end);     
        
        sem_post(semReceiver);          // receiver訊號量+1


        printf(LIGHT_GREEN"Sending Message: "NONE);
        printf(CYAN"%s"NONE, message.text);

        time_taken += (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) * 1e-9;
        // sem_getvalue(semSender, &value);
        // printf("Sender Semaphore value after sending: %d\n", value);
        // sem_getvalue(semReceiver, &value);
        // printf("Receiver Semaphore value after sending: %d\n\n", value);
        // sleep(1);
    }

    fclose(file);

    strcpy(message.text, "exit");

    sem_wait(semSender);
    send(message, &mailbox);
    sem_post(semReceiver);
    
    
    printf(RED"\nEnd of input file! exit!\n"NONE);
    printf("Total time taken in sending msg: %fs.\n", time_taken);

    // sem_close(semSender);

    // if (mechanism == 1) {
    //     mq_unlink(MQ_NAME);
    // } 
    // if (mechanism == 2) {
    //     munmap(mailbox.storage.shm_addr, MAX_MSG_SIZE);
    //     shm_unlink(SHM_NAME);
    //     // sem_unlink(SEM_RECEIVER);
    //     // sem_unlink(SEM_SENDER);
    // }

    return 0;
}