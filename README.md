###### system-programming-project1
## About this project:

This project was collaboratively developed by the following team members during the System Programming course:
- [Serhat Kılıç](https://github.com/s192275) 
- [Umutcan Mert](https://github.com/UmutcanMert)
- [Muhammed Nihat Aydın](https://github.com/Nihat-AYDIN) 
- [Muhammet Hamza Yavuz](https://github.com/hamza37yavuz) 

## Project-1: A Communicating Multi-shell (Part-1: intro, shared file)
# Project-1: A Communicating Multi-shell (Part-1: intro, shared file)

## Prerequisites
- C system calls
- Exec family functions
- Fork
- Wait and similar system calls
- File I/O

## Learning Objectives
- Practicing fork/exec relationships.
- Understanding the multi-process environment and communication through a shared object.
- Understanding the details of a shell program.
- Understanding the need for inter-process communication mechanisms (to be taught in the coming weeks).

## Background
In this project, you are required to implement a multi-shell that shares memory among each other. You have been provided with the files `singleshell.c` and `multishell.c`.

## Details and Requirements

### singleshell.c
/* Content of the initmem() function */
fd = shm_open(MY_SHARED_FILE_NAME, O_RDWR, 0);
/* Map shared memory */
addr = mmap(NULL, 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

When using shm_open, it creates a shared file object (usage is exactly the same as open), and mmap maps this object into memory. The information you need here can be thought of exactly like malloc. In other words, you can perceive the addr variable as an array variable:
addr[0], addr[1]

You can use it like this or copy data using:
strncpy(&addr[0]), memcpy(addr,)

When copying, be careful not to overwrite what you wrote before; you always need to append to the end of the existing string. You can use strcat or strncpy() like this:
strncpy(&addr[strlen(addr)], data, max_length);

You can also perform direct writing to and reading from this area using fd.

##### Complete `singleshell.c` as follows:

- Do not use `system()`. You are required to use `exec` functions and `fork`.
- The program should write all stdout and stderr from the entire shell to `addr` separately (write with a newline at the end).
- Addition: In this section, you do not need to write the output of commands executed with exec to `addr`. Think of it as commands written in the shell, i.e., both visible in the shell and written to a file (consider it like a log).
- When writing to a file, prepend its own process id.
- When the program starts, it should write its own process id, parent process id, and the start date, month, day, and time to the file.
- When `exit` is given, it should terminate the execution. Before terminating, it should write the end date, month, day, and time to the file.
- Commands like `ls` should be executed like in a normal shell. The program should wait for the shell program to finish, then continue its execution.
- When the program name is given in the current directory or another directory, it should be able to execute them. For example, `./directory1/directory2/programname`.
- It should appropriately redirect the given arguments.
- It should close all file descriptors, etc., when exiting. Error checking should be performed on all system calls used.

#### multishell
It creates a given number of singleshell program instances. For example,

multishell 4

It creates 4 instances of singleshell programs using the xterm application (xterm ./singleshell: 2nd argument ./singleshell).
