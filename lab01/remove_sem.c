#include <semaphore.h>
#include <sys/shm.h>
#include <mqueue.h>

#define SEM_SENDER "/sender_semaphore"
#define SEM_RECEIVER "/receiver_semaphore"
#define MQ_NAME "/Lab01_MQ"

int main(){
    sem_unlink(SEM_SENDER);
    sem_unlink(SEM_RECEIVER);
    mq_unlink(MQ_NAME);
    return 0;
}