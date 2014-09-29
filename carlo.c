// Written by Jordan Williams (jwill49)

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <math.h>
#include <limits.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>
#include <setjmp.h>

void set_signals(int);
float print_status();

// Globals to keep track of seg faults
unsigned long int attempts = 0, misses = 0, signals = 0;
jmp_buf continue_loc;
jmp_buf exit_clean_loc;
jmp_buf exit_sigint_loc;
jmp_buf exit_sigusr2_loc;
jmp_buf exit_sigalrm_loc;
long unsigned int precision_counter = 10;


// Handles all of the signals
void sig_handler(int signo) {
	// Reset the signal watchers
	set_signals(1);
	signals++;
	
	switch (signo) {
		case SIGINT: siglongjmp( exit_sigint_loc, 0 );
					 break;
		case SIGSEGV: misses++;
					  siglongjmp( continue_loc, 0 );
					  break;
  		case SIGTSTP: print_status();
					  siglongjmp( continue_loc, 0 );
  					  break;
  		case SIGUSR2: siglongjmp(exit_sigusr2_loc, 0);
  					  break;
		case SIGALRM: siglongjmp(exit_sigalrm_loc, 0);
					  break;		  
		default: break;
	}
}


// Wrapper function to set signals
void set_signals(int reset) {
	// Only set these signals on initial 
	if (!reset) {
		if (signal(SIGINT, sig_handler) == SIG_ERR)
			printf("\ncan't catch SIGINT\n");
	}
	
	// Set Segfault signal
	if (signal(SIGSEGV, sig_handler) == SIG_ERR)
		printf("\ncan't catch SIGSEGV\n");
	
	// Set Stop signal
	if (signal(SIGTSTP, sig_handler) == SIG_ERR)
		printf("\ncan't catch SIGTSTP\n");
	
	// Set SIGUSR2 signal
	if (signal(SIGUSR2, sig_handler) == SIG_ERR)
		printf("\ncan't catch SIGUSR2\n");
	
	// Set SIGALRM signal
	if (signal(SIGALRM, sig_handler) == SIG_ERR)
		printf("\ncan't catch SIGALRM\n");
}


// Print status of the monte carlo simulation
float print_status() {
	float percentage = ((float)misses / (float)attempts)*100;
	
	printf("Monte Carlo Results\n");
	printf("  attempts: %lu\n", ++attempts);
	printf("  misses: %lu\n", misses);
	printf("  percentage: %02f\n", percentage);
	printf("  signals recevied: %ld\n", signals);
	return percentage;
}


// Checks if a value is a power of 10
int is_power_of_ten () {
	if (precision_counter == attempts) {
		precision_counter *= 10;
		return 1;
	}
	return 0;
}


// Returns a random address in the available address space
uintptr_t get_address() {
	// Find the value of RAND_MAX
	// printf("The value of RAND_MAX is %d\n", RAND_MAX);
	//
	// long long unsigned int max_16_bit = (2<<16)-1;
	// long long unsigned int max_32_bit = (2<<32)-1;
	// long long unsigned int max_64_bit = (2<<64)-1;
	//
	// long long unsigned int address_max = (sizeof(int *) == 4 ? max_32_bit : max_64_bit );
	//
	// long long unsigned int result;
	//
	// result = rand() % max_32_bit;
	//
	// printf("32 bit: %x\n", max_32_bit);
	// printf("64 bit: %x\n", max_64_bit);
	// printf("address max: %x\n", address_max);
	// printf("random result: %x\n", result);
	
	return rand();
}


int main(int argc, char **argv) {
	// Seed random generator
	srand( time( NULL ));
	
	// Set the signals
	set_signals(0);
	
	// Increase valid memory addresses space
	double dummy[1000][1000] = {{0.0}};
		
	// Parse the alarm timer
	if (argc > 1) {
		float timer = atof(argv[1]);		
		if (timer > 0) alarm((int)timer);
	}
	
	// Normal exit
	if (sigsetjmp( exit_clean_loc, 1 ) > 0 ) {
		return print_status();
	}
	
	// SIGINT exit
	if (sigsetjmp( exit_sigint_loc, 1 ) > 0 ) {
		print_status();
		exit(SIGINT);
	}
	
	// SIGUSR2 exit
	if (sigsetjmp( exit_sigusr2_loc, 1 ) > 0 ) {
		print_status();
		exit(SIGUSR2);
	}
	
	// SIGALRM exit
	if (sigsetjmp( exit_sigalrm_loc, 1 ) > 0 ) {
		print_status();
		exit(SIGALRM);
	}
	
	// Loop
	unsigned long int loop_max = ULONG_MAX;
	for (attempts = 0; attempts < loop_max; attempts++) {
		if (sigsetjmp( continue_loc, 1 ) == 1) attempts++;
		
		// Exit if maximum attempts have been reached
		if (attempts >= loop_max) siglongjmp( exit_clean_loc, 0 );
		
		// Notify parent that power of ten has been reached
		if ( is_power_of_ten() ) kill(getppid(), SIGUSR1);
		
		// Attempt to access a "random" address
		int tmp;
		int *var;
		var = get_address();
		tmp = *var;
	}

	siglongjmp( exit_clean_loc, 0 );
	return 0;
}