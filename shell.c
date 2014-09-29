// Written by Jordan Williams (jwill49)

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <setjmp.h>
#include <string.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_SIZE 512

#ifndef SIGRTMAX
#define SIGRTMAX NSIG
#endif

int precision_count = 0;
int precision_max = 3;

// Variables for sig jumps
jmp_buf continue_loc;
jmp_buf exit_loc;

typedef struct rusage Rusage;

// Handles all of the signals for the shell.
void sig_handler(int sig) {
	switch (sig) {
		case SIGINT: break;
		case SIGTSTP: break;
		case SIGUSR1: precision_count++;
			 		  siglongjmp(continue_loc, 0);
					  break;
		case SIGSEGV: printf("Caught a SIGSEGV\n"); break;
		default: break;
	}
}


// Parse args for shell input
void parse_args(char *buffer, char** args, size_t args_size, size_t *arg_count)
{
	char *buf_args[ args_size ];
	char **cp;
	char *buf;
	size_t i, j;
	
	buf = buffer;
	buf_args[0] = buffer; 
	args[0] = buffer;
	int quit_loop = 0;
	
	// Using strsep() instead of strtok(), but same functionality on VM
	for( cp = buf_args; (!quit_loop) && ((*cp = strsep(&buf, " \n\t")) != NULL) ;){
		if ( (*cp != '\0') && (++cp >= &buf_args[args_size]) )
			quit_loop = 1;
	}
	
	for( i = 0, j = 0; buf_args[i] != NULL; i++ ){
		if( strlen(buf_args[i]) > 0 )
			args[j++] = buf_args[i];
	}
	
	// Set arg_count and NULL terminate the last element
	*arg_count = j;
	args[j] = NULL;
}


// Sets the signal behavior for the shell
void set_signals() {
	if (signal(SIGINT, sig_handler) == SIG_ERR)
		printf("\ncan't catch SIGINT\n");
	if (signal(SIGTSTP, sig_handler) == SIG_ERR)
		printf("\ncan't catch SIGTSTP\n");
	if (signal(SIGUSR1, sig_handler) == SIG_ERR)
		printf("\ncan't catch SIGUSR!\n");
	if (signal(SIGSEGV, sig_handler) == SIG_ERR)
		printf("\ncan't catch SIGSEGV\n");
}


int main(int argc, char **argv, char **envp) {
	printf("Simple Shell - written by Jordan Williams\n\n");
	
	char buffer[MAX_SIZE];
	char *args[MAX_SIZE];
	int ret_status = 0;
	size_t arg_count = 0;
	pid_t pid = 0;
	
	// Read in the precision argument
	if (argc > 1) precision_max = atoi(argv[1]);
	if (precision_max < 1) precision_max = 3;
	
	while( printf("$ ") ){
		fgets(buffer, MAX_SIZE, stdin);
		parse_args(buffer, args, MAX_SIZE, &arg_count);
		
		// If nothing is entered, restart the loop
		if (arg_count==0) continue;
		
		// Exit condition
		if (!strcmp(args[0], "exit" )) exit(0);
		
		// Fork this process
		pid = fork();
		
		set_signals();
		
		// Child process
		if (pid){
			printf("Starting child process (%d)\n", pid);
			Rusage *usage = malloc(sizeof(Rusage));
			
			// Set the return point for receiving SIGUSR1
			sigsetjmp( continue_loc, 1 );
			
			// If precision limit has been reached send signal back to child to exit
			if (precision_count >= precision_max) kill(pid, SIGUSR2);
			
			// Wait for child to return
			pid = wait4(pid, &ret_status, WSTOPPED, usage);
			
			// Determines how child was terminated
			if (WIFEXITED(ret_status)) {
				if (WEXITSTATUS(ret_status) > SIGRTMAX || WEXITSTATUS(ret_status) == 0) 
					printf("child process (%d) exited cleanly with value of: %d\n", pid, WEXITSTATUS(ret_status));
				else
					printf("child process (%d) terminated by %s\n", pid, strsignal(WEXITSTATUS(ret_status)));
			}
			
			// Display extra information
			printf("Number of page faults without any I/O activity: %ld\n", usage->ru_minflt);
			printf("Number of page faults with I/O activity: %ld\n", usage->ru_majflt);
			printf("Number of page swaps: %ld\n", usage->ru_nswap);
		}
		
		// Parent process
		else {
			if( execvp(args[0], args) ) {
				puts(strerror(errno));
				exit(127);
			}
		}
		
		// Resert precision count for subsequent children
		precision_count = 0;
	}
	return 0;
}