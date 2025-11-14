#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define maximum_token_length 100
#define maximum_variable 10
#define maximum_code_length 100

// Token Types
typedef enum {
    T_INT, T_IDENTIFIER, T_NUMBER, T_ASSIGN,
    T_PLUS, T_MINUS, T_MULT, T_DIV,
    T_IF, T_ELSE, T_EQUAL, T_LBRACE,
    T_RBRACE, T_SEMICOLON, T_UNKNOWN, T_EOF
} TokenType;

// Token Structure
typedef struct {
    TokenType type;
    char text[maximum_token_length];
} Token;

// Variable Structure
typedef struct {
    char name;
    int addr;
} Variable;

Variable variables[maximum_variable];
int varCount = 0;
int nextAddr = 100;
int no_of_labels = 0;

// Get Variable Address
int getVarAddress(char name) {
    for (int i = 0; i < varCount; i++) {
        if (variables[i].name == name) {
            return variables[i].addr;
        }
    }
    variables[varCount].name = name;
    variables[varCount].addr = nextAddr++;
    return variables[varCount++].addr;
}

// Helper to trim whitespace
void trim(char *str) {
    int i = 0, j = 0;
    while (isspace((unsigned char)str[i])) i++;
    while (str[i]) {
        str[j++] = str[i++];
    }
    str[j] = '\0';
    while (j > 0 && isspace((unsigned char)str[j - 1])) {
        str[--j] = '\0';
    }
}

// Helper to remove trailing semicolon
void removeSemicolon(char *str) {
    int len = strlen(str);
    if (len > 0 && str[len - 1] == ';')
        str[len - 1] = '\0';
}

// Lexer: Get Next Token
void tokenizeFile(FILE *file, FILE *tokensFile) {
    Token token;
    int c;
    while ((c = fgetc(file)) != EOF) {
        if (isspace(c)) continue;

        if (isalpha(c)) {
            int len = 0;
            token.text[len++] = c;
            while (isalnum(c = fgetc(file))) {
                if (len < maximum_token_length - 1) token.text[len++] = c;
            }
            ungetc(c, file);
            token.text[len] = '\0';

            if (strcmp(token.text, "int") == 0) token.type = T_INT;
            else if (strcmp(token.text, "if") == 0) token.type = T_IF;
            else if (strcmp(token.text, "else") == 0) token.type = T_ELSE;
            else token.type = T_IDENTIFIER;
            fprintf(tokensFile, "Token: %s\n", token.text);
        } else {
            char symbol[3] = {c, '\0'};
            fprintf(tokensFile, "Token: %s\n", symbol);
        }
    }
}

// Code Generation
void generateAssembly(const char *inputFile, const char *outputFile) {
    FILE *input = fopen(inputFile, "r");
    FILE *output = fopen(outputFile, "w");
    char line[maximum_code_length];

    // Reset variable data for each run
    varCount = 0;
    nextAddr = 100;
    int if_label = no_of_labels++;
    int else_label = no_of_labels++;
    int end_label = no_of_labels++;

    int in_if_block = 0;
    int in_else_block = 0;

    if (!input || !output) {
        perror("Error opening file in generateAssembly");
        return;
    }

    while (fgets(line, sizeof(line), input)) {

        trim(line);
        removeSemicolon(line);

        if (strlen(line) == 0) continue;

        if (strstr(line, "int")) {
            continue;
        } 
        
        // IF condition
        if (strstr(line, "if")) {
            char var;
            int value;
            if (sscanf(line, "if (%c == %d)", &var, &value) == 2) {
                int addr = getVarAddress(var);
                fprintf(output, "mov A M %d\n", addr);
                fprintf(output, "cmp A %d\n", value);
                fprintf(output, "jne else_part_%d\n", else_label);
                in_if_block = 1;
            }
        }

        // ELSE condition 
        else if (strstr(line, "else")) {
            fprintf(output, "jmp end_if_%d\n", end_label);
            fprintf(output, "else_part_%d:\n", else_label);
            in_if_block = 0;
            in_else_block = 1;
        }

        // Assignment statements
        if (strchr(line, '=')) {
            char var1, var2, var3, op;
            int value;

            // Case 1: a = 5;
            if (sscanf(line, "%c = %d", &var1, &value) == 2) {
                int addr1 = getVarAddress(var1);
                fprintf(output, "ldi A %d\n", value);
                fprintf(output, "mov M A %d\n", addr1);
            }

            // Case 2: a = b + c; or a = b - c;
            else if (sscanf(line, "%c = %c %c %c", &var1, &var2, &op, &var3) == 4) {
                int addr1 = getVarAddress(var1);
                int addr2 = getVarAddress(var2);
                int addr3 = getVarAddress(var3);

                fprintf(output, "mov A M %d\n", addr2);
                if (op == '+')
                    fprintf(output, "add M %d\n", addr3);
                else if (op == '-')
                    fprintf(output, "sub M %d\n", addr3);
                fprintf(output, "mov M A %d\n", addr1);
            }

            // Case 3: a = a + 1; or a = a - 1;
            else if (sscanf(line, "%c = %c %c 1", &var1, &var2, &op) == 3) {
                int addr1 = getVarAddress(var1);
                int addr2 = getVarAddress(var2);
                fprintf(output, "mov A M %d\n", addr2);
                if (op == '+')
                    fprintf(output, "add M 101\n");
                else if (op == '-')
                    fprintf(output, "sub M 101\n");
                fprintf(output, "mov M A %d\n", addr1);
            }
        }
    }

    if (in_if_block || in_else_block)
        fprintf(output, "end_if_%d:\n", end_label);
    
    
    fprintf(output, "end:\nhlt\n");
    fclose(input);
    fclose(output);
}

// AST Generation
void generateAST_node(FILE *inputFile, FILE *astFile) {
    char line[maximum_code_length];

    while (fgets(line, sizeof(line), inputFile)) {
        line[strcspn(line, "\n")] = 0;  

        if (strstr(line, "if")) {
            fprintf(astFile, "AST Node: %s {\n", line);
        }
        else if (strstr(line, "else")) {
            fprintf(astFile, "} else {\n");
        }
        else if (strchr(line, '}')) {
            fprintf(astFile, "}\n");
        }
        else {
            fprintf(astFile, "    AST Node: %s\n", line);
        }
    }
}


// Main Function
int main() {
    printf("Starting Compilation...\n");
    FILE *inputFile = fopen("input.txt", "r");
    if (!inputFile) {
        perror("Error: Could not open input.txt");
        return 1;
    }

    FILE *tokensFile = fopen("tokens.txt", "w");
    FILE *astFile = fopen("ast_output.txt", "w");
    if (!tokensFile || !astFile) {
        perror("Error: Could not open output files");
        return 1;
    }

    printf("Generating Tokens...\n");
    tokenizeFile(inputFile, tokensFile);
    rewind(inputFile);

    printf("Generating AST...\n");
    generateAST_node(inputFile, astFile);

    fclose(tokensFile);
    fclose(astFile);
    fclose(inputFile);

    printf("Generating Assembly...\n");
    generateAssembly("input.txt", "output.asm");
    
    printf("Compilation Completed Successfully!\n");

    return 0;
}
