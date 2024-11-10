// This code is by partners James Daoud and Lee Tamir
// Code to include standard libraries for code
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

// Declare helper functions used in code
char** getToken(char* str, int* tokenCount);
void freeTokens(char** tokens, int tokenCount);
void executeCommand(char **args, int tokenCount);
void handleRedirection(char **args, int tokenCount);
void executePipe(char **args, int tokenCount);

// Define maximum line and input length
#define MAX_LINE_LENGTH 255
#define MAX_INPUT_LENGTH 1024 
// To store previous command when 'prev' is called
char prevInput[MAX_LINE_LENGTH];

void executeCommand(char **args, int tokenCount) {
    // Checks for no arguments and returns accordingly
    if (tokenCount == 0 || args[0] == NULL) return;
    // Checks if commands called by user are available
    if (strcmp(args[0], "exit") == 0) {
        write(STDOUT_FILENO, "Bye bye.\n", 9);
        exit(0);
    // Else there are the following built in commands    
    } else if (strcmp(args[0], "cd") == 0) {
        if (!args[1]) args[1] = getenv("HOME");
        if (chdir(args[1]) != 0) {
        perror("cd failed");
        }
        return;
    } else if (strcmp(args[0], "source") == 0) {
        if (!args[1]) {
            write(STDOUT_FILENO, "source: No filename provided\n", 30);
        return;
        }
        FILE *file = fopen(args[1], "r");
        if (!file) {
            perror("source: Unable to open file");
            return;
        }
        char line[MAX_LINE_LENGTH];
        while (fgets(line, sizeof(line), file)) {
            line[strcspn(line, "\n")] = '\0';  // Remove newline character
            int tokenCount = 0;
            char **tokens = getToken(line, &tokenCount);
            executeCommand(tokens, tokenCount);
            freeTokens(tokens, tokenCount);
        }
        fclose(file);
        return;
    } else if (strcmp(args[0], "prev") == 0) {
        if (strlen(prevInput) == 0) {
            write(STDOUT_FILENO, "prev: No previous command\n", 26);
            return;
        }
        // Tokenize the previous command and execute it
        int tokenCount = 0;
        char **prevTokens = getToken(prevInput, &tokenCount);
        executeCommand(prevTokens, tokenCount);
        freeTokens(prevTokens, tokenCount);
        return;
    } else if (strcmp(args[0], "help") == 0) {
        const char *helpMessage = "Built-in commands:\n"
                              "cd [path]        Change the current directory to 'path'.\n"
                              "source [file]    Execute commands from 'file'.\n"
                              "prev             Re-execute the previous command.\n"
                              "help             Display this help message.\n"
                              "exit             Exit the shell.\n";
        write(STDOUT_FILENO, helpMessage, strlen(helpMessage));
        return;
    }
    // Stores command run to previous input for when prev is called
    memset(prevInput, 0, MAX_LINE_LENGTH);
    // Creates the string of the previous command run so it can be reused
    for (int i = 0; i < tokenCount; i++) {
        if (args[i] != NULL) {
            strcat(prevInput, args[i]);
            if (i < tokenCount - 1) {
                strcat(prevInput, " ");
            }
        }
    }
    // Calls helper function for pipe if pipe is detectec
    for (int i = 0; i < tokenCount; i++) {
        if (args[i] != NULL && strcmp(args[i], "|") == 0) {
            executePipe(args, tokenCount);
            return;
        }
    }
    // Calls helper function for redirection
    handleRedirection(args, tokenCount);
}

void executePipe(char **args, int tokenCount) {
    // Finds where in string the pipe is
    int pipePos = -1;
    for (int i = 0; i < tokenCount; i++) {
        if (strcmp(args[i], "|") == 0) {
            pipePos = i;
            break;
        }
    }
    // Returns function if pipe isn't found
    if (pipePos == -1) return;
    // Creates pipe
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("Pipe failed");
        return;
    }
    // Fork the first child for left hand side command
    pid_t pid1 = fork();
    if (pid1 < 0) {
        perror("Fork failed");
        return;
    }
    // First child process
    if (pid1 == 0) {
        // Close read end of pipe
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        // Close write end of pipe
        close(pipefd[1]);
        args[pipePos] = NULL;
        execvp(args[0], args);
        perror("LHS command failed");
        exit(1);
    }
    // Fork the second child for right hand side command
    pid_t pid2 = fork();
    if (pid2 < 0) {
        perror("Fork failed");
        return;
    }
    // Second child process
    if (pid2 == 0) {
        // Close write end of pipe
        close(pipefd[1]);
        dup2(pipefd[0], STDIN_FILENO);
        // Close read end of pipe
        close(pipefd[0]);
        execvp(args[pipePos + 1], args + pipePos + 1);
        perror("RHS command failed");
        exit(1);
    }
    // Close all read writes and wait for null for parent process
    close(pipefd[0]);
    close(pipefd[1]);
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}

// Helper function for < > symbols
void handleRedirection(char **args, int tokenCount) {
    // Initialize helper chars to determine if < or > is being used
    char *inputFile = NULL;
    char *outputFile = NULL;
    // Check for < or >
    for (int i = 0; i < tokenCount; i++) {
        if (strcmp(args[i], "<") == 0 && i + 1 < tokenCount) {
            inputFile = args[i + 1];
            args[i] = NULL;  // Null-terminate before input file
        } else if (strcmp(args[i], ">") == 0 && i + 1 < tokenCount) {
            outputFile = args[i + 1];
            args[i] = NULL;  // Null-terminate before output file
        }
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork failed");
        return;
    }
    if (pid == 0) {
        // Checks if inputfile (<) is specified
        if (inputFile) {
            int fd = open(inputFile, O_RDONLY);
            if (fd < 0) {
                perror("Error opening input file");
                exit(1);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }
        // Checks if outputfile (>) is specified
        if (outputFile) {
            int fd = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                perror("Error opening output file");
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }
        // Execute command accordingly
        execvp(args[0], args);
        perror("Command not found");
        exit(1);
    } else {
        // Parent waits for child to finish
        wait(NULL);
    }
}

// Helper function to read line of input and handle long commands
void readInput(char *inputBuffer) {
    int totalBytes = 0;
    while (1) {
        // Reads the input and puts it into a buffer
        int bytesRead = read(STDIN_FILENO, inputBuffer + totalBytes, MAX_INPUT_LENGTH - totalBytes - 1);
        if (bytesRead < 0) {
            perror("Error reading input");
            exit(1);
        }
        totalBytes += bytesRead;
        // Check for end of the file
        if (bytesRead == 0) {
            inputBuffer[totalBytes] = '\0';
            if (totalBytes > 0) {
                break;
            } else {
                write(STDOUT_FILENO, "\nBye bye.\n", 10);
                exit(0);
            }
        }
        // Checks for new line or if buffer is at limit
        if (strchr(inputBuffer, '\n') || totalBytes >= MAX_INPUT_LENGTH - 1) {
            break;
        }
    }
    inputBuffer[totalBytes] = '\0';
}

// Main function where shell is run and messages are outputted
int main() {
    char input[MAX_INPUT_LENGTH];
    write(STDOUT_FILENO, "Welcome to mini-shell.\n", 23);
    while (1) {
        write(STDOUT_FILENO, "shell $ ", 8);
        // Read input using readInput function
        readInput(input);
        // Remove newline character if present
        if (strlen(input) > 0 && input[strlen(input) - 1] == '\n') {
            input[strlen(input) - 1] = '\0';
        }
        // Check multiple commands splitting based on ;
        char *command = strtok(input, ";\n");
        while (command != NULL) {
            // Trim whitespace
            while (*command == ' ') command++;
            char *end = command + strlen(command) - 1;
            while (end > command && (*end == ' ' || *end == '\n')) end--;
            end[1] = '\0';  // Null-terminate
            int tokenCount = 0;
            char **tokens = getToken(command, &tokenCount);
            // Execute command if valid
            if (tokenCount > 0) {
                executeCommand(tokens, tokenCount);
            }
            // Free allocated memory and move to next command
            freeTokens(tokens, tokenCount);
            command = strtok(NULL, ";\n");
        }
    }
    // Finish running shell
    return 0;
}
