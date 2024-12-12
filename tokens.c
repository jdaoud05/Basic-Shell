#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

// Define maximum length of line
#define MAX_LINE_LENGTH 255

// Check if there are special characters for them to become tokens
bool isSpecialChar(char c) {
    return (c == '(' || c == ')' || c == '<' || c == '>' || c == ';' || c == '|');
}
char** getToken(char* str, int* tokenCount) {
    char** tokens = malloc(MAX_LINE_LENGTH * sizeof(char*));
    *tokenCount = 0;
    // Helper boolean to ensure words in quotes are not individually tokenized
    bool inQuotes = false;
    // First character in string
    char* startChar = str;
    // While loop until end of string
    while (*str != '\0') {
        if (*str == '\"') {
            inQuotes = !inQuotes;
            // If there are quotes the entire string inside is tokenized
            if (inQuotes) {
                startChar = str + 1;
            } else {
                int length = str - startChar;
                if (length > 0) {
                    tokens[*tokenCount] = malloc(length + 1);
                    strncpy(tokens[*tokenCount], startChar, length);
                    tokens[*tokenCount][length] = '\0';
                    (*tokenCount)++;
                }
                // Makes next character after end of quotes to tokenize
                startChar = str + 1;
            }
            // Tokenizes accordingly if character is not in quotes
        } else if (!inQuotes && (*str == ' ' || isSpecialChar(*str))) {
            if (startChar != str) {
                int length = str - startChar;
                // Allocates memory for token and copys it to token
                tokens[*tokenCount] = malloc(length + 1);
                strncpy(tokens[*tokenCount], startChar, length);
                tokens[*tokenCount][length] = '\0';
                (*tokenCount)++;
            }
            // Adds special character as a token
            if (isSpecialChar(*str)) {
                tokens[*tokenCount] = malloc(2);
                tokens[*tokenCount][0] = *str;
                tokens[*tokenCount][1] = '\0';
                (*tokenCount)++;
            }
            startChar = str + 1;
        }
        str++;
    }
    // Adds last token if not inside quotes
    if (startChar != str && !inQuotes) {
        tokens[*tokenCount] = malloc(str - startChar + 1);
        strncpy(tokens[*tokenCount], startChar, str - startChar);
        tokens[*tokenCount][str - startChar] = '\0';
        (*tokenCount)++;
    }
    tokens[*tokenCount] = NULL;
    return tokens;
}

// Frees allocated memory
void freeTokens(char** tokens, int tokenCount) {
    for (int i = 0; i < tokenCount; i++) {
        free(tokens[i]);
    }
    free(tokens);
}
