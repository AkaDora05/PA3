//Eunjun Kim
//exk170230
//CS3371.0W1
//03/24/19
//Nhut Nguyen

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>

void parseCommand(char *commandLine);
char *readInCommand(char **readPtr);
void executeCommand(char *command, int input, int output);

int main() {
    displayVersion();
	
    //ask the user to input a command
    //until she input exit
    bool run=true;
    while (run) {
        printf("Enter command: ");
        char *command;
        int commandSize = 0;
        getline(&command, &commandSize, stdin);
        if (!strcmp(command, "exit")) run=false;

        parseCommand(command);
    }

    return 0;
}

//Break down each command and parse it
void parseCommand(char *commandLine) {
    char *charPtr = commandLine;

    //file descriptions for pipe and i/o
    int input = 0, output = 1;
    bool pipeBool = false;

    while (*charPtr != '\0' && *charPtr != '\n') {
        //read in each commands
        char *command = readInCommand(&charPtr);

        int pipeDescription[2];
        if (*charPtr == '|') {
            //execute piping
            pipe(pipeDescription);
            output = pipeDescription[1];
            pipeBool = true;
            charPtr++;
        }
        else if (*charPtr == '<') {
            //execute redirection
            charPtr++;
            char *name = readInCommand(&charPtr);
            input = open(name, O_RDONLY, 0777);
        }
        else if (*charPtr == '>') {
            //execute redirection out
            charPtr++;
            int mode = O_WRONLY | O_CREAT;
            if (*charPtr == '>') {
                mode = O_WRONLY | O_CREAT | O_APPEND;
            }

            char *name = readInCommand(&charPtr);
            output = open(name, mode, 0777);
        }
        else if (*charPtr == ';') {
            //change back to original
            charPtr++;
            input = 0;
            output = 1;
        }

        //run the command with the new inputs
        executeCommand(command, input, output);

        if (pipeBool == true) {
            input = pipeDescription[0];
            output = 1;
            close(pipeDescription[1]);
        }
    }
}

//function that reads in each commands
char *readInCommand(char **readPtr) {
    while (isspace(**readPtr)) (*readPtr)++;
    char *charPtr = *readPtr;
    int length = 0;
    while (*charPtr != '|' && *charPtr != ';' && *charPtr != '<' &&
            *charPtr != '>' && *charPtr != '\0' && *charPtr != '\n') {
        charPtr++;
        length++;
    }

    //copy to new string
    char *command = malloc(length+1);
    strncpy(command, *readPtr, length);
    command[length] = '\0';

    *readPtr = charPtr;
    return command;
}

//function that executes based on
//the value of pid
void executeCommand(char *command, int input, int output) {
    //count spaces
    int spaces = 0;
    for (int i=0; i<strlen(command); i++) {
        if (isspace(command[i])) {
            spaces++;
        }
    }

    //split command by spaces
    int num = 0;
    char **arguments = malloc(spaces * sizeof(char*) + 1);
    char *start = command, *ptr = command;
    while (true) {
        while (isspace(*start)) {
            start++;
            ptr = start;
        }
        while (!isspace(*ptr) && *ptr != '\0') {
            ptr++;
        }

        //copy string if greater 0
        int length = ptr - start;
        if (length > 0) {
            arguments[num] = malloc(length+1);
            strncpy(arguments[num], start, length);
            arguments[length] = '\0';
            start = ptr;
            num++;
        }

        //stop at end of string
        if (*ptr == '\0') break;
    }
    arguments[num] = NULL;

    //fork the process
    pid_t pid = fork();

    //child process
    if (pid == 0) {
        //run command and change description
        dup2(input, 0);
        dup2(output, 1);
        execvp(arguments[0], arguments);
        exit(0);
    }
    //parent process
    else if (pid > 0) {
        //wait for command to stop
        waitpid(pid);

    } else {
        perror("fork");
    }
}









