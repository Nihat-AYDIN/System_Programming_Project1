/*
 UMUTCAN MERT 21120205006 : addr'ye yazma islemi, multishell'de shellog dosyasi olusturma ve addr'yi shelloga atma
 SERHAT KILIC 21120205005 : loglari hashleme(ekstra fonksiyon), calisma zamani ve pid'leri dosyaya atma 
 MUHAMMED NIHAT AYDIN 21120205062 : addr'ye yazma islemi, renklendirme(ekstra fonksiyon)
 MUHAMMET HAMZA YAVUZ 21120205009 : birden fazla xtermle acma, loglari hashleme
 
 NOT: Her kisinin yaninda yazan gorev gruptan ayri yaptigi calismadir. 
      Bunun disindaki fonksiyonlar grupca toplanilip hatalar ayiklanarak yapilmistir. Bu durum multishell.c icin de gecerlidir.
      Her xterm'de calisan singleshell icin tarihler hem giris hem cikis icin addr'ye eklenir.  
      Ekstra fonksiyonaliteler : - Alinan komutlari yazdirirken baslarina hash kodu eklenir. 
                                 - Shell renklendirme.

 Chatgpt'den veya diger yardimci sitelerden alinan destekler:
    -strftime fonksiyonu ve bu fonksiyona bagli kutuphanelerin implemente edilmesi
    -time ile ilgili kutuphanelerin entegre edilmesi ve kullanilmasi
    -getenv fonksiyonunun kullanimi ve chdir fonksiyonunun kullanimi
    -ANSI kullanimi ve islevi
    -strcspn fonksiyonunun kullanimi
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#define INBUF_SIZE 256

/* these should be the same as multishell.c */
#define MY_FILE_SIZE 1024
#define MY_SHARED_FILE_NAME "/sharedlogfile"

char *addr = NULL;
int fd = -1;

int initmem(){
     
    fd = shm_open(MY_SHARED_FILE_NAME, O_RDWR, 0);
    if (fd < 0){
        perror("singleshell.c:fd:line31");
        exit(1);
    }
   
    addr = mmap(NULL, MY_FILE_SIZE,
                PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == NULL){
        perror("singleshell.c:mmap:");
        close(fd);
        exit(1);
    }
    return 0;
}
/*
Dosyaya process id'leri ve tarihleri yazar.
*/
int timetofile(int fd){
    time_t now;
    struct tm *tm_info;
    char buffer[27], str_pid[16], str_ppid[16];

    /* process id ve parent process id alinir */
    pid_t pid = getpid();
    pid_t ppid = getppid();

    /* tarih ve saat bilgisi alinir */
    time(&now);
    tm_info = localtime(&now);
    strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);

    /* process id ve parent process id stringe donusur */
    sprintf(str_pid, "%d", pid);
    sprintf(str_ppid, "%d", ppid);

    strcat(addr, "parent: ");
    strcat(addr, str_ppid);
    strcat(addr, " ");

    strcat(addr, "Child: ");
    strcat(addr, str_pid);
    strcat(addr, " ");

    /* dosyaya yazdirma islemi gerceklesir */
    buffer[27]='\0';     
    strcat(addr,buffer);
    strcat(addr,"\n");
    
    write(1,buffer,strlen(buffer));
    write(1,"\n",1);

    return 0;
}

int main(int argc, char *argv[]) {
    
    initmem();

    char command[INBUF_SIZE];
    argv[INBUF_SIZE];
    char *token;
    int argv_count = 0;
    int i; /* argvlerdeki forlarda kullandik. */
    int status;
    pid_t pid;

    /* baslangicta dosyaya tarih yazma */
    timetofile(fd);
     
    /* user alma ve renklendirme */
    char *user = getenv("USER");
    char colored_user[INBUF_SIZE];
    sprintf(colored_user, "\033[31m%s$\033[0m", user);
    
    /* komutun devamliligi */
    while (1) {
        char hash[] = {'0','1','2','3','4','5','6','7','8','9',
                '!','+','%','&','/','(',')',',','='};
        char new_command[266]; 
        int s, random; /*s ==> alttaki iki forda kullanilan iterator.*/

        write(1,colored_user,strlen(colored_user));
        fflush(stdout); /* buffer'i siler */
        
        int fd_read = read(0,command,INBUF_SIZE);
        
        srand(time(NULL)); /* rastgele sayi uretmek icin zaman degeri kullandik */
    
        strcat(new_command, command); /* new_command in basina command i ekledik */
    
        for(s = strlen(command) -1 ; s >= 0 ; s--){
            new_command[s+5] = command[s];
        }
        for(s = 0; s < 5; s++) {
            random = rand() % 18; /* hash dizisinin boyutu 18 oldugu icin 0-17 arasi rastgele sayi uretilir */
            new_command[s] = hash[random];
            new_command[strlen(command)+s+5] = hash[random]; 

            new_command[strlen(command)+s+5] = '\0'; 
        }
        strcat (addr, new_command); 
        
        /* hata kontrolu */
        if(fd_read < 0){
            perror("fd_read:");
            strcat (addr, strerror(errno));
            strcat(addr, "\n");
            close(fd_read);
            exit(-1);
        } 
        command[strcspn(command, "\n")] = '\0'; /* inputtan \n karakterini kaldirdik. */
        
        /* command i argv ye tokenlar. */
        token = strtok(command, " ");
        
        while (token != NULL && argv_count < INBUF_SIZE - 1) {
            argv[argv_count++] = token;
            token = strtok(NULL, " ");
        }
        argv[argv_count] = NULL;

        /* komut kontrolleri yapilir. */
        if (strncmp(argv[0], "exit",4) == 0) {
            timetofile(fd);
            close(fd);
            exit(0);
        }
        else if (strncmp(argv[0], "cd",2) == 0) {
            if (argv_count < 2) {
                /* dizin saglanmazsa home klasorune doner. */
                chdir(getenv("HOME"));
            }
            else {
                /* belirtilen dizine gider. */
                if (chdir(argv[1]) != 0) {
                    perror("chdir:");
                }
            }
        }
        else {
            pid = fork();

            if (pid < 0) {
                perror("fork:\n");
                strcat (addr, strerror(errno));
                strcat(addr, "\n");
                exit(-1);
            }
            else if (pid == 0) {
                /* Child process */
                execvp(argv[0], argv);
                perror("170:exevp:\n");
                strcat (addr, strerror(errno));
                strcat(addr, "\n");
                exit(-1);
            }
            else {
                /* Parent process */
                waitpid(pid, &status, 0);
                
                char str_pid[16];
                pid_t pid = getpid(); 
                sprintf(str_pid, "%d", pid);
                
                strcat(addr, "process: ");
                strcat(addr, str_pid);
                strcat(addr, " ");
                
            }
        }
        
        /* command temizlenir. */
        for (i = 0; i < argv_count; i++) {
            argv[i] = NULL;
        }
        argv_count = 0;
        close(fd_read);
    }
    munmap(addr, 1024);
    close(fd);
    return 0;
}
