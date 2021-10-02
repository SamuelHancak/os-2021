#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// reutrns reading end of pipe which stores numbers from 2 up to 35
int
numbers_generator() {
	int output[2];

    // handling of pipe opening
    if(pipe(output) < 0){
        printf("pipe failed\n");
        exit(-1);
    }

	if(fork() == 0){
		for(int i = 2; i < 36; ++i){
            // writing numbers to pipe
			write(output[1], &i, sizeof(int));
		}
        // closing write end of pipe in child process
		close(output[1]);

		exit(0);
	}
    // closing write end of pipe in parent process
	close(output[1]);

	return output[0];
}

// returns reading end of the pipe which includes only not divisble numbers, from the first argument of the function 'fd_numbers', by the second argument 'prime'
int 
prime_numbers(int fd_numbers, int prime) 
{
	int num;
	int output[2];

    // handling of pipe opening
    if(pipe(output) < 0){
        printf("pipe failed\n");
        exit(-1);
    }

	if(fork() == 0){
		while(read(fd_numbers, &num, sizeof(int))){
			if(num % prime){
                // writing only not divisible numbers by the prime number to the write end of the pipe
				write(output[1], &num, sizeof(int));
			}
		}
        // closing read end of pipe passed as argument of the function in child process
		close(fd_numbers);
        // closing write end of pipe in child process
		close(output[1]);
		
		exit(0);
	}
    // closing read end of pipe passed as argument of the function in parent process
	close(fd_numbers);
    // closing write end of pipe in parent process
	close(output[1]);

	return output[0];
}

int
main()
{
  	int prime;
    // storing read end of pipe returned by numbers_genrator function
	int numbers = numbers_generator();

    // reading from pipe until there are numbers to read
	while(read(numbers, &prime, sizeof(int))){
		printf("prime %d\n", prime);
		numbers = prime_numbers(numbers, prime);
	}

	exit(0);
}



