#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char** getToken(char* str, int* tokenCount);
void freeTokens(char** tokens, int tokenCount);

int main() {
    char input[256];
    int numBytes;
    // Read input
    numBytes = read(STDIN_FILENO, input, 255);
    // Throw error if read was not succesful
    if (numBytes < 0) {
        perror("Failed to read input");
        return 1;
    }
    input[numBytes] = '\0';
    // Checks for newline and removes accordingly
    if (numBytes > 0 && input[numBytes - 1] == '\n') {
        input[numBytes - 1] = '\0';
    }
    // Tokenizes input
    int tokenCount = 0;
    char** tokens = getToken(input, &tokenCount);
    // Print tokens
    for (int j = 0; j < tokenCount; j++) {
        printf("%s\n", tokens[j]);
    }
    // Free allocated memory
    freeTokens(tokens, tokenCount);
    return 0;
}
