#include "ipc_utils.h"

char* get_timestamp() {
    static char buffer[32];
    const time_t current_time = time(NULL);
    const struct tm* local_time = localtime(&current_time);
    strftime(buffer, sizeof(buffer), "[%F %H:%M:%S]", local_time);
    return buffer;
}

void get_fifo_path(const char* username, char* path_buffer){
    const size_t size = strlen(username) + 13;
    snprintf(path_buffer, size, "/tmp/mailbox_%s", username);
}

void get_semaphore_name(const char* username, char* sem_buffer) {
    const size_t size = strlen(username) + 9;
    snprintf(sem_buffer, size, "/mailsem_%s", username);
}

int semaphore_exists(const char* sem_name) {
    sem_t* sem =  sem_open(sem_name, 0);
    if (sem == SEM_FAILED) {
        return 0;
    }
    sem_close(sem);
    return 1;
}

int fifo_exists(const char* fifo_path){
    struct stat st;
    return (stat(fifo_path, &st) == 0 && S_ISFIFO(st.st_mode));
}

sem_t* create_semaphore(const char* sem_name) {
    sem_t* sem = sem_open(sem_name, O_CREAT | O_EXCL, 0644, 1);
    if (sem == SEM_FAILED && errno == EEXIST) {
        sem = sem_open(sem_name, 0);
    }
    return sem;
}

int create_fifo(const char* fifo_path) {
    if (fifo_exists(fifo_path)) {
        return 1;
    }

    if (mkfifo(fifo_path, 0666) == -1) {
        perror("Failed to create FIFO");
        return 0;
    }
    return 1;
}
int write_message(const char* recipient, const char* sender, const char* message) {
    if (message == NULL || strlen(message) == 0) {
        printf("Invalid message to send\n");
        return -1;
    }

    char fifo_path[256];
    get_fifo_path(recipient, fifo_path);

    if (fifo_exists(fifo_path) == 0) {
        perror("Recipient is offline");
        return -1;
    }

    char sem_name[256];
    get_semaphore_name(recipient, sem_name);

    sem_t* sem = sem_open(sem_name, 0);
    if (sem == SEM_FAILED) {
        return -1;
    }
    sem_wait(sem);

    int fd = open(fifo_path, O_WRONLY| O_NONBLOCK);
    if (fd == -1) {
        perror("Failed to open mailbox");
        sem_post(sem);
        sem_close(sem);
        return -1;
    }

    char msg_buffer[MAX_MESSAGE_LENGTH + 128];
    snprintf(msg_buffer,sizeof(msg_buffer),"%s from %s: %s", get_timestamp(), sender, message);
    size_t written_bytes = 0;

    while (written_bytes < strlen(msg_buffer)) {
        size_t written = write(fd, msg_buffer + written_bytes, strlen(msg_buffer) - written_bytes);
        if (written == -1) {
            perror("Write error");
            break;
        }
        written_bytes += written;
    }
    close(fd);

    sem_post(sem);
    sem_close(sem);
    return 1;
}


size_t read_message(const char* username, char* buffer, size_t size) {
    char fifo_path[256];
    get_fifo_path(username, fifo_path);
    printf("FIFOPATH: %s", fifo_path);
    if (fifo_exists(fifo_path) == 0) {
        perror("Wrong username");
        return -1;
    }

    char sem_name[256];
    get_semaphore_name(username, sem_name);

    if (semaphore_exists(sem_name) == 0) {
        perror("Mailbox is not initialized.");
        return -1;
    }
    sem_t* sem = sem_open(sem_name, 0);
    if (sem == SEM_FAILED) {
        return -1;
    }
    sem_wait(sem);
    int fd = open(fifo_path, O_RDONLY|O_NONBLOCK);
    if (fd == -1) {
        perror("Failed to open mailbox");
        sem_post(sem);
        sem_close(sem);
        return -1;
    }
    ssize_t bytes_read = read(fd, buffer, size - 1);
    close(fd);
    sem_post(sem);
    sem_close(sem);

    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        return bytes_read;
    }
    return 0;
}