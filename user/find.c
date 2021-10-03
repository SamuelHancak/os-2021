#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

char*
fmtname(char *path)
{
	char *p;

	// looking for the first character after last slash
	for(p = path + strlen(path); p >= path && *p != '/'; p--);
	p++;

	return p;
}

void
find(char *path, char *targetname) 
{
	char buf[512], *p;
	int fd;
	struct dirent de;
	struct stat st;

	if(!strcmp(fmtname(path), targetname)){
		printf("%s\n", path);
	}

    fd = open(path, O_RDONLY);

	if(fd < 0){
		fprintf(2, "find: cannot open [%s], fd=%d\n", path, fd);
		return;
	}

	if(fstat(fd, &st) < 0){
		fprintf(2, "find: cannot stat %s\n", path);
		close(fd);
		return;
	}

	if(st.type != T_DIR){
		close(fd);
		return;
	}
	
	if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
		printf("find: path too long\n");
		close(fd);
		return;
	}

	strcpy(buf, path);
	p = buf + strlen(buf);
	*p++ = '/';

	while(read(fd, &de, sizeof(de)) == sizeof(de)){
		if(de.inum == 0)
			continue;

		// writing to 'p' from 'de.name'
		memmove(p, de.name, DIRSIZ);
		p[DIRSIZ] = 0;
		
        // handling of infinite loop by skipping '.' and '..'
		if(!strcmp(de.name, ".") || !strcmp(de.name, ".."))
			continue;

        // recursively calling function to get further
		find(buf, targetname);
	}
    
	close(fd);
}

int
main(int argc, char *argv[])
{
	if(argc < 3){
		fprintf(2, "usage: find path filename\n");
		exit(-1);
	}

	find(argv[1], argv[2]);

	exit(0);
}
