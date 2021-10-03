#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

// returns '1' in case argument 'c' is an empty char, otherwise returns '0'
int 
is_empty_space(char c)
{
    if(c == ' ' || c == '\t'){
        return 1;
    }

    return 0;
}

int
main(int argc, char *argv[])
{
    char buf[512], ch;
	char *p = buf;
	char *arguments[MAXARG];
	int i, empty_spaces = 0, offset = 0;

    if(argc < 2){
        fprintf(2, "usage: xargs <command> [argv...]\n");
        exit(-1);
    }

	for(i = 0; i < argc; ++i){
		arguments[i] = argv[i+1];
	}
	i--;

    // reading from standard input
	while(read(0, &ch, 1) > 0){
		if(is_empty_space(ch) && !empty_spaces){
			empty_spaces = 1;
			continue;
		}

        // in case of empty space, passing '0' to ignore it
		if(empty_spaces){
			buf[offset++] = 0;
			arguments[i++] = p;
			p = buf + offset;

			empty_spaces = 0;
		}

        // storing read arguments from standard input to buf until end of line
        // then passing read values to array 'arguments' which will be send as argument of 'exec' function
		if(ch != '\n'){
			buf[offset++] = ch;
		}else{
			arguments[i++] = p;
			p = buf + offset;

            // executing passed arguments in child process
			if(fork() == 0){
                int ret = exec(arguments[0], arguments);
				exit(ret);
			}

			wait(0);
			i = argc - 1;
		}
	}

	exit(0);
}