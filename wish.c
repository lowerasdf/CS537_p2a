#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define SHELL_PROMPT "wish> "
#define LINE_MAX 255
#define ARGS_MAX 15

void printPrompt() {
    printf(SHELL_PROMPT);
}

void parse(char* line, char** args) {
    char* token;
    const char *delimiters = " \t\n";
    int i = 0;
    while((token = strsep(&line, delimiters)) != NULL) {
        if (i >= ARGS_MAX || token == NULL) {
            break;
        }
        if (strlen(token) != 0) {
            args[i] = token;
            printf("token: %s\n", args[i]);
            i += 1;
        }
    }
}

void execute(char *args[]) {
    int rc = fork();
    if (rc < 0) {
        // TODO handle error
        exit(1);
    } else if (rc == 0) {
        // TODO add the actual PATH here
        char str[80];
        strcpy(str, "/bin/");
        strcat(str, args[0]);
        int err = execv(str, args);
        printf("error code: %d\n", err);
        exit(0);
    } else {
        wait(NULL);
        // parent goes down this path (original process)
        // int wc = wait(NULL);
        // printf("hello, I am parent of %d (wc:%d) (pid:%d)\n",
	    //    rc, wc, (int) getpid());
    }
}

int main(int argc, char *argv[])
{
    // TODO check interactive vs batch
    if (argc == 1) {
        printf("Run in interactive mode\n");
        // TODO interactive
    } else if (argc == 2) {
        printf("Run in batch mode\n");
        // TODO batch
    } else {
        printf("Invalid number of arguments\n");
        // TODO invalid usage
    }

    char *line = NULL;
    size_t len = 0;
    char **args;
    for(;;) {
        printPrompt();

        // Parse line
        int temp = getline(&line, &len, stdin);
        if (temp == -1) {
            exit(0);   
        }

        args = malloc(ARGS_MAX * sizeof(char*));
        parse(line, args);

        // for(int i = 0; i < ARGS_MAX; i++){
        //     printf("arg #%d: %s, ", i, args[i]);
        // }
        // printf("\n");

        // TODO check "extra" arguments[1:]
        if (args[0] == NULL) {
            continue;
        } else if (strcmp(args[0], "exit") == 0) {
            exit(0);
        } else if (strcmp(args[0], "cd") == 0) {
            chdir(args[1]);
        } else {
            execute(args);
        }

        free(args);
    }
}