#include <stdio.h>
#include <unistd.h> // read, write, close
#include <stdlib.h>
#include <fcntl.h> // open

// The maxinum buffer size
#define MAX_BUFFER_SIZE 1024
// Read/Write Buffer
char buf[MAX_BUFFER_SIZE];

int main(int argc, char const *argv[])
{
    // Record the read byte size
    ssize_t bytes_read;
    // File descriptor, default input for stdin
    int fd = STDIN_FILENO; 
    if(argc > 2){
        fprintf(stderr,"Input format: ./mycat or ./mycat Filepath\n");
        exit(EXIT_FAILURE);
    }else if(argc == 2){
        // open a file
        fd = open(argv[1], O_RDONLY); 
        // Deal with no such file case
        if( fd == -1 ){
            fprintf(stderr,"No such file\n");
            exit(EXIT_FAILURE);
        }
    }
    fprintf(stdout,"\n");
    fprintf(stdout,"====================================================================================\n");
    fprintf(stdout,"  Begin: %s\n", argv[1]);
    fprintf(stdout,"====================================================================================\n");
    // 0 represent stdin
    while((bytes_read = read(fd,buf,MAX_BUFFER_SIZE)) > 0){
        // write line to the stdout
        if(write(STDOUT_FILENO, buf, bytes_read) < 0 ){
            fprintf(stderr, "Write Error\n");
            exit(EXIT_FAILURE);
        }   
    }
    fprintf(stdout,"====================================================================================\n");
    fprintf(stdout,"  End of File.\n");
    fprintf(stdout,"====================================================================================\n");
    fprintf(stdout,"\n");
    // Don't forget close the file descriptor
    if(fd != STDIN_FILENO)
        close(fd);
    exit(EXIT_SUCCESS);
}
