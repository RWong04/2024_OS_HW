#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "../include/command.h"
#include "../include/builtin.h"

// ======================= requirement 2.3 =======================
/**
 * @brief 
 * Redirect command's stdin and stdout to the specified file descriptor
 * If you want to implement ( < , > ), use "in_file" and "out_file" included the cmd_node structure
 * If you want to implement ( | ), use "in" and "out" included the cmd_node structure.
 *
 * @param p cmd_node structure
 * 
 */
void redirection(struct cmd_node *p){
	if ( p->in_file != NULL ){
		int fd0 = open(p->in_file, O_RDONLY, 0);
		if (fd0 < 0) {
            perror("Error opening input file");
            exit(EXIT_FAILURE);
        }
		dup2(fd0, STDIN_FILENO);
		close(fd0);
		// p->in = 0;
	}

	if ( p->out_file != NULL ){
		int fd1 = creat(p->out_file, 0644);
		if (fd1 < 0) {
            perror("Error opening output file");
            exit(EXIT_FAILURE);
        }
		dup2(fd1, STDOUT_FILENO);
		close(fd1);
		// p->out = 0;
	}
}
// ===============================================================

// ======================= requirement 2.2 =======================
/**
 * @brief 
 * Execute external command
 * The external command is mainly divided into the following two steps:
 * 1. Call "fork()" to create child process
 * 2. Call "execvp()" to execute the corresponding executable file
 * @param p cmd_node structure
 * @return int 
 * Return execution status
 */
int spawn_proc(struct cmd_node *p)
{
	pid_t pid = fork();

	if ( pid < 0 ){				// fork failed
        perror("fork failed");
        return -1;
	}
	else if ( pid == 0 ){		// child process
		// printf("Argument 0: %s\n", p->args[0]);
		// printf("Argument 1: %s\n", p->args[1]);
		// printf("Argument 2: %s\n", p->args[2]);

		redirection(p);
		if (execvp(p->args[0], p->args) < 0) {
            perror("exec failed");
            exit(EXIT_FAILURE);
        }
	}
	else{						// parent process
		int status;
        waitpid(pid, &status, 0);
		// wait(&exit_status);
        if (WIFEXITED(status)) {
            return 1;
        } else {
            return -1;
		}
	}
  	return 1;
}
// ===============================================================


// ======================= requirement 2.4 =======================
/**
 * @brief 
 * Use "pipe()" to create a communication bridge between processes
 * Call "spawn_proc()" in order according to the number of cmd_node
 * @param cmd Command structure  
 * @return int
 * Return execution status 
 */
int fork_cmd_node(struct cmd *cmd)
{
	int numPipes = cmd->pipe_num;
	struct cmd_node *current = cmd->head;
	int pipeFd[numPipes * 2];
	pid_t pid;
	int status;

	for(int i = 0; i < numPipes; i++){
        if(pipe(pipeFd + i*2) < 0) {
            perror("couldn't pipe");
            exit(EXIT_FAILURE);
        }
    }

	int j = 0;
	while (current != NULL){
		pid = fork();
		if(pid == 0) {

            //if not last command
            if(current->next != NULL){
                if(dup2(pipeFd[j + 1], 1) < 0){
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }

            //if not first command
            if(j != 0 ){
                if(dup2(pipeFd[j-2], 0) < 0){
                    perror(" dup2");///j-2 0 j+1 1
                    exit(EXIT_FAILURE);

                }
            }
            for(int i = 0; i < 2*numPipes; i++){
                    close(pipeFd[i]);
            }

            if( execvp(current->args[0], current->args) < 0 ){
                    perror(current->args[0]);
                    exit(EXIT_FAILURE);
            }

        } else if(pid < 0){
            perror("error");
            exit(EXIT_FAILURE);
        }

        current = current->next;
        j+=2;
	}
	for(int i = 0; i < 2 * numPipes; i++){
        close(pipeFd[i]);
    }

    for(int i = 0; i < numPipes + 1; i++) wait(&status);
	
	return 1;
}
// ===============================================================


void shell()
{
	while (1) {
		printf(">>> $ ");
		char *buffer = read_line();
		if (buffer == NULL)
			continue;

		struct cmd *cmd = split_line(buffer);
		
		int status = -1;
		// only a single command
		struct cmd_node *temp = cmd->head;
		
		if(temp->next == NULL){
			status = searchBuiltInCommand(temp);
			if (status != -1){
				int in = dup(STDIN_FILENO), out = dup(STDOUT_FILENO);
				if( in == -1 | out == -1)
					perror("dup");
				redirection(temp);
				status = execBuiltInCommand(status,temp);

				// recover shell stdin and stdout
				if (temp->in_file)  dup2(in, 0);
				if (temp->out_file){
					dup2(out, 1);
				}
				close(in);
				close(out);
			}
			else{
				//external command
				status = spawn_proc(cmd->head);
			}
		}
		// There are multiple commands ( | )
		else{
			status = fork_cmd_node(cmd);
		}
		// free space
		while (cmd->head) {
			
			struct cmd_node *temp = cmd->head;
      		cmd->head = cmd->head->next;
			free(temp->args);
   	    	free(temp);
   		}
		free(cmd);
		free(buffer);
		// printf("status: %d\n", status);
		if (status == 0)
			break;
	}
}
