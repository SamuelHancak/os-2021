#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main()
{
  int to_parent[2], to_child[2];

  pipe(to_parent);
  pipe(to_child);

  int pid = fork();

  if(pid == 0){
    // child
    char received;
    // reading one byte from child
    read(to_child[0], &received, 1);
    printf("%d: received ping\n", getpid());
    // writing one byte to parent
    write(to_parent[1], "x", 1); 
  } else {
    // parent
    // writing one byte to child
    write(to_child[1], "b", 1);
    char received;
    // reading one byte from parent
    read(to_parent[0], &received, 1);
    printf("%d: received pong\n", getpid());
  }
  
  exit(0);
}
