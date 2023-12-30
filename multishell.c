/*
 UMUTCAN MERT 
 SERHAT KILIC  
 MUHAMMED NIHAT AYDIN
 MUHAMMET HAMZA YAVUZ  
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>

/* these should be the same as multishell.c */
#define MY_FILE_SIZE 1024
#define MY_SHARED_FILE_NAME "/sharedlogfile"
#define MAX_SHELL 10
#define DEFAULT_NSHELL 2


char *addr = NULL; /*mmap addres*/
int fd = -1;       /*fd for shared file object*/

int initmem(){
    fd = shm_open(MY_SHARED_FILE_NAME,
                  O_CREAT | O_RDWR | O_TRUNC, 0666);
    if (fd < 0){
        perror("multishell.c:open file:");
        exit(1);
    }
    if (ftruncate(fd, 1024) == -1){
        perror("ftruncate");
        exit(1);
    }

    addr = mmap(NULL, MY_FILE_SIZE,
                PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == NULL){
        perror("mmap:");
        exit(1);
    }
    return 0;
}
/*
shellog dosyasi olusturur ve addr dekileri shellog dosyasina atar.
*/
int writetofile(){

    time_t now = time(NULL);
    struct tm *date = localtime(&now);
    char datetime[80];

    strftime(datetime, sizeof(datetime), "%Y-%m-%d-%H-%M-%S", date);
    
    char filename[100];
    
    strcat(filename,"shellog");
    strcat(filename,datetime);
    strcat(filename,".txt");

    int file = open(filename, O_WRONLY | O_CREAT, 0666);
    
    /* hata kontrolu */
    if(file < 0){
        perror("open file:");
        close(file); 
        exit(1);
    }

    if(write(file, addr, strlen(addr)) == -1){
        perror("write file:");
        close(file);
        exit(1);
    }
    
    close(file);
    return 0; 
}
int main(int argc, char *argv[]) {
    initmem();

    char *buf[4];
     
    int arg = atoi(argv[1]);
    pid_t pid;
    
    for(int i = 0; i < arg ; i++) {
         pid = fork();
       
        if(pid == 0) { /* Child process */
            buf[0]="xterm";
            buf[1]="-e"; /* xterm parametresi */
            buf[2]="./singleshell";
            buf[3]=NULL;
            
            int ex_er=execvp(buf[0], buf);
            
            /* hata kontrolu */
            if(ex_er < 0){
                perror("97:execvp:");
            }
            return 1;
        }
        if(pid < 0){
           perror("pid fork error:");
           exit(0);
        }
    }

    for(int i=0; i<arg ;i++){
       wait(NULL);
    }
    /* dosyaya her sey kapaninca tarih yazilir. */
    writetofile();
    
    munmap(addr, 1024);
    close(fd);

    return 0;
}
