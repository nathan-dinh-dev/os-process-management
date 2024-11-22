#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUFFER_SIZE 128

int main() {
    int pipefd[2];
    pid_t pid;
    char input[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];

    // Create a pipe
    if (pipe(pipefd) == -1) {
        perror("Pipe failed");
        exit(EXIT_FAILURE);
    }

    // Create a child process
    pid = fork();
    if (pid < 0) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {  // Parent process: Sender
        close(pipefd[0]); // Close unused read end

        while (1) {
            printf("$ "); // Prompt for user input
            fgets(input, BUFFER_SIZE, stdin);
            input[strcspn(input, "\n")] = 0; // Remove trailing newline

            // Write the command to the pipe
            write(pipefd[1], input, strlen(input) + 1);

            // Terminate when user enters 'T'
            if (strcmp(input, "T") == 0) {
                close(pipefd[1]); // Close write end
                break;
            }
        }
    } else {  // Child process: Receiver
        close(pipefd[1]); // Close unused write end

        while (1) {
            // Read the command from the pipe
            int bytesRead = read(pipefd[0], buffer, BUFFER_SIZE);
            if (bytesRead <= 0) {
                perror("Read failed");
                exit(EXIT_FAILURE);
            }

            printf("Received command: %s\n", buffer);

            // Exit if the command is 'T'
            if (strcmp(buffer, "T") == 0) {
                printf("Terminating simulation.\n");
                close(pipefd[0]); // Close read end
                break;
            }
        }
    }

    return 0;
}
