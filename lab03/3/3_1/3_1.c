#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>

#define matrix_row_x 1234
#define matrix_col_x 250

#define matrix_row_y 250
#define matrix_col_y 4

FILE *fptr1;
FILE *fptr2;
FILE *fptr3;
FILE *fptr4;
FILE *fptr5;
int **x;
int **y;
int **z;

// Put file data intp x array
void data_processing(void){
    int tmp;
    fscanf(fptr1, "%d", &tmp);
    fscanf(fptr1, "%d", &tmp);
    for(int i=0; i<matrix_row_x; i++){
        for(int j=0; j<matrix_col_x; j++){
            if (fscanf(fptr1, "%d", &x[i][j])!=1){
                printf("Error reading from file");
                return;
            }
        }
    }

    fscanf(fptr2, "%d", &tmp);
    fscanf(fptr2, "%d", &tmp);
     for(int i=0; i<matrix_row_y; i++){
        for(int j=0; j<matrix_col_y; j++){
            if (fscanf(fptr2, "%d", &y[i][j])!=1){
                printf("Error reading from file");
                return;
            }
        }
    }   
}


void *thread1(void *arg){
    // print_thread_info("Thread1");
    for(int i=0; i<matrix_row_x/2; i++){
        for(int j=0; j<matrix_col_y; j++){
            for(int k=0; k<matrix_row_y; k++){
                z[i][j] += x[i][k] * y[k][j];
            }      
        }
    }
}

void *thread2(void *arg){
    // print_thread_info("Thread2");
    for(int i=matrix_row_x/2; i<matrix_row_x; i++){
        for(int j=0; j<matrix_col_y; j++){
            for(int k=0; k<matrix_row_y; k++){
                z[i][j] += x[i][k] * y[k][j];
            }     
        }
    } 
}

int main(){
    ssize_t bytesRead;
    char buffer[50];
    x = malloc(sizeof(int*)*matrix_row_x);
    for(int i=0; i<matrix_row_x; i++){
        x[i] = malloc(sizeof(int)*matrix_col_x);
    }
    y = malloc(sizeof(int*)*matrix_row_y);
    for(int i=0; i<matrix_row_y; i++){
        y[i] = malloc(sizeof(int)*matrix_col_y);
    }
    z = malloc(sizeof(int*)*matrix_row_x);
    for(int i=0; i<matrix_row_x; i++){
        z[i] = malloc(sizeof(int)*matrix_col_y);
    }
    fptr1 = fopen("m1.txt", "r");
    fptr2 = fopen("m2.txt", "r");
    fptr3 = fopen("3_1.txt", "a");
    fptr4 = fopen("/proc/Mythread_info", "r");
    fptr5 = fopen("/proc/Mythread_info", "r");

    pthread_t t1, t2;
    data_processing();
    fprintf(fptr3, "%d %d\n", matrix_row_x, matrix_col_y);

    pthread_create(&t1, NULL, thread1, NULL);
    pthread_create(&t2, NULL, thread2, NULL);
    while (fgets(buffer, sizeof(buffer), fptr4) != NULL){
        printf("%s", buffer);
    }
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    for(int i=0; i<matrix_row_x; i++){
        for(int j=0; j<matrix_col_y; j++){
            fprintf(fptr3, "%d ", z[i][j]);
            if(j==matrix_col_y-1) fprintf(fptr3, "\n");   
        }
    }
    fclose(fptr1);
    fclose(fptr2);
    fclose(fptr3);
    fclose(fptr4);
    fclose(fptr5);
}
