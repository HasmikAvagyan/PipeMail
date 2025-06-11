#include "ipc_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define MAX_PATH_LEN 256
#define COLOR_MAGENTA "\x1B[35m"
#define COLOR_GREEN   "\x1B[32m"
#define COLOR_RESET   "\x1B[0m"

volatile sig_atomic_t keep_running = 1;

void print_banner() {
    printf(COLOR_MAGENTA "\n"
           "██████╗ ██╗██████╗ ███████╗███╗   ███╗ █████╗ ██╗██╗\n"
           "██╔══██╗██║██╔══██╗██╔════╝████╗ ████║██╔══██╗██║██║\n"
           "██████╔╝██║██████╔╝█████╗  ██╔████╔██║███████║██║██║\n"
           "██╔═══╝ ██║██╔═══╝ ██╔══╝  ██║╚██╔╝██║██╔══██║██║██║\n"
           "██║     ██║██║     ███████╗██║ ╚═╝ ██║██║  ██║██║██║\n"
           "╚═╝     ╚═╝╚═╝     ╚══════╝╚═╝     ╚═╝╚═╝  ╚═╝╚═╝╚═╝\n" COLOR_RESET);
}

void print_menu() {
    printf(COLOR_MAGENTA "\n╔══════════════════════════╗\n"
                       "║" COLOR_MAGENTA "      PipeMail Menu     " COLOR_MAGENTA "║\n"
                       "╠══════════════════════════╣\n"
                       "║ " COLOR_MAGENTA "1." COLOR_RESET " Send message        " COLOR_GREEN "║\n"
                       "║ " COLOR_MAGENTA "2." COLOR_RESET " Start mailbox      " COLOR_GREEN "║\n"
                       "║ " COLOR_MAGENTA "3." COLOR_RESET " Exit               " COLOR_GREEN "║\n"
                       "╚══════════════════════════╝\n" COLOR_RESET);
    printf(COLOR_MAGENTA "> " COLOR_RESET);
    fflush(stdout);
}


void handle_sigint(int sig) {
    const char msg[] = "\nShutting down mailbox...\n";
    write(STDERR_FILENO, msg, sizeof(msg)-1);
    keep_running = 0;
}

void setup_mailbox(const char* username) {
    char fifo_path[MAX_PATH_LEN];
    char sem_name[MAX_PATH_LEN];

    get_fifo_path(username, fifo_path);
    get_semaphore_name(username, sem_name);

    if (!fifo_exists(fifo_path)) {
        if (mkfifo(fifo_path, 0666) == -1) {
            perror("Error creating FIFO");
            exit(EXIT_FAILURE);
        }
        printf("Created new mailbox FIFO\n");
    }
    
    sem_t *sem = sem_open(sem_name, O_CREAT, 0644, 1);
    if (sem == SEM_FAILED) {
        perror("Error creating semaphore");
        exit(EXIT_FAILURE);
    }
    sem_close(sem);
}

void mailbox_loop(const char* username) {
    char fifo_path[MAX_PATH_LEN];
    get_fifo_path(username, fifo_path);
    int fd = open(fifo_path, O_RDONLY| O_NONBLOCK);
    if (fd == -1) {
        perror("Could not open mailbox for reading");
        return;
    }

    char buffer[256];
    printf("Mailbox active for %s. Press Ctrl+C to exit.\n", username);
    while (keep_running) {
        ssize_t bytes = read(fd, buffer, sizeof(buffer)-1);
        if (bytes > 0) {
            buffer[bytes] = '\0';
            printf("New message: %s\n", buffer);
        } else if (bytes == 0) {
            sleep(1);
        } else {
            perror("Read error");
            sleep(1);
        }
    }

    close(fd);
}

int main() {
    signal(SIGINT, handle_sigint);

    print_banner();
    while (1) {
        print_menu();
        fflush(stdout);

        char input[10];
        if (!fgets(input, sizeof(input), stdin)) continue;

        int choice;
        if (sscanf(input, "%d", &choice) != 1) {
            printf("Invalid input\n");
            continue;
        }

        switch (choice) {
            case 1: {  // Send message
                char sender[256], recipient[256], message[256];

                printf("Your username: ");
                fflush(stdout);
                if (!fgets(sender, sizeof(sender), stdin)) continue;
                sender[strcspn(sender, "\n")] = '\0';

                printf("Recipient: ");
                fflush(stdout);
                if (!fgets(recipient, sizeof(recipient), stdin)) continue;
                recipient[strcspn(recipient, "\n")] = '\0';

                printf("Message: ");
                fflush(stdout);
                if (!fgets(message, sizeof(message), stdin)) continue;
                message[strcspn(message, "\n")] = '\0';

                int result = write_message(recipient, sender, message);
                printf(result == 1 ? "✓ Message sent\n" : "✗ Failed to send\n");
                break;
            }

            case 2: {  // Start mailbox
                char username[256];
                printf("Your username: ");
                fflush(stdout);
                if (!fgets(username, sizeof(username), stdin)) break;
                username[strcspn(username, "\n")] = '\0';

                setup_mailbox(username);
                mailbox_loop(username);
                keep_running = 1;
                break;
            }

            case 3:
                printf("Goodbye!\n");
                exit(EXIT_SUCCESS);

            default:
                printf("Invalid choice\n");
        }
    }
    return 0;
}



