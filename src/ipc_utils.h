#ifndef IPC_UTILS_H
#define IPC_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <semaphore.h>

#define MAX_MESSAGE_LENGTH 200

//Function to get timestamp for the current message
char* get_timestamp();

//Function to generate FIFO path
void get_fifo_path(const char* username, char* path_buffer);

//Function to generate semaphore name for the user
void get_semaphore_name(const char* username, char* sem_buffer);



//Function to check if semaphore exists
int semaphore_exists(const char* sem_name);

//Function to check if FIFO exists
int fifo_exists(const char* fifo_path);


//Function to handle cleanup for both the semaphore and the FIFO file descriptor
void cleanup_mailbox(sem_t* sem, int fd);

//Function to create a new semaphore
sem_t* create_semaphore(const char* sem_name);

//Function to create a new fifo
int create_fifo(const char* fifo_path);

//Function to open or create semaphore
sem_t* open_semaphore(const char* username, int create);

//Function for writing the message to the receivers FIFO
int write_message(const char* receiver, const char* sender, const char* message);

//Function for reading a message from the FIFO
size_t read_message(const char* username, char* buffer, size_t size);

#endif //IPC_UTILS_H
