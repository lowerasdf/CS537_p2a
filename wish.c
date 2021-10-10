#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <ctype.h>

#define SHELL_PROMPT "wish> "
#define MAX_LINE_LENGTH 255
#define MAX_ARGS_COUNT 50
#define MAX_PATH_COUNT 100
#define MAX_PATH_LENGTH 255
#define DEFAULT_PATH "/bin"
#define MAX_LOOP_VARIABLE_SIZE 1000000

void printPrompt() {
    printf(SHELL_PROMPT);
}

int isNumber(char *s) {
    if (s[0] == '\0') {
        return 0;
    }
    for (int i = 0; s[i] != '\0'; i++){
        if (isdigit(s[i]) == 0) {
            return 0;
        }
    }
    return 1;
}

int parse(char* line, char** args) {
    char* token;
    const char *delimiters = " \t\n";
    int i = 0;
    while((token = strsep(&line, delimiters)) != NULL) {
        if (i >= MAX_ARGS_COUNT || token == NULL) {
            break;
        }
        if (strlen(token) != 0) {
            args[i] = token;
            // args[i] = malloc((strlen(token) + 1) * sizeof(char));
            // strcpy(args[i], token);
            // printf("token no.%d: %s\n", i, args[i]);
            i += 1;
        }
    }
    return i;
}

void execute(char *args[], char *path) {
    int rc = fork();
    if (rc < 0) {
        // TODO handle error
        exit(1);
    } else if (rc == 0) {
        int err = execv(path, args);
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
    char error_message[30] = "An error has occurred\n";
    FILE *f = NULL;

    // TODO check interactive vs batch
    if (argc == 1) {
        f = stdin;
    } else if (argc == 2) {
        f = fopen(argv[1], "r");
        if(f == NULL) {
            write(STDERR_FILENO, error_message, strlen(error_message));
            exit(1);
        }
    } else {
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
    }

    char *line = NULL;
    size_t len = 0;
    char *args[MAX_ARGS_COUNT];
    int num_args;

    char *path[MAX_PATH_COUNT];
    path[0] = malloc((strlen(DEFAULT_PATH) + 1) * sizeof(char));
    strcpy(path[0], DEFAULT_PATH);
    path[1] = NULL;

    int loop_count = 0;

    for(;;) {
        // int i = 0;
        // while(path[i] != NULL) {
        //     printf("PATH: %s, ", path[i]);
        //     i += 1;
        // }
        // printf("\n");

        if (loop_count == 0) {
            if (argc == 1) {
                printPrompt();
            }

            // args = malloc(MAX_ARGS_COUNT * sizeof(char*));

            // Parse line
            int temp = getline(&line, &len, f);
            if (temp == -1) {
                exit(0);   
            }

            num_args = parse(line, args);
            args[num_args] = NULL;   
        } else {
            loop_count -= 1;
        }

        // for(int i = 0; i < MAX_ARGS_COUNT; i++){
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
        } else if (strcmp(args[0], "path") == 0) {
            int i = 0;
            while(path[i] != NULL) {
                free(path[i]);
                i += 1;
            }

            i = 0;
            while(args[i+1] != NULL && i < MAX_PATH_COUNT) {
                path[i] = malloc((strlen(args[i+1]) + 1) * sizeof(char));
                strcpy(path[i], args[i+1]);
                i += 1;
            }
            path[i] = NULL;
        } else if (strcmp(args[0], "loop") == 0) {
            if (num_args < 2 || !isNumber(args[1])) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                continue;
            }

            loop_count = atoi(args[1]);
            for (int i = 2; i < num_args; i++) {
                args[i-2] = args[i];
            }
            args[num_args-2] = NULL;
            num_args -= 2;
        } else {
            int i = 0;
            while (path[i] != NULL) {
                char str[strlen(path[i]) + strlen("/" + strlen(args[0])) + 1];
                strcpy(str, path[i]);
                strcat(str, "/");
                strcat(str, args[0]);

                int fd = access(str, X_OK);
                if (fd == -1) {
                    i += 1;
                } else {
                    execute(args, str);
                    break;
                }
            }
        }

        // for(int i = 0; i < num_args; i++) {
        //     free(args[i]);
        // }
        // free(args);
    }

    int i = 0;
    while(path[i] != NULL) {
        free(path[i]);
        i += 1;
    }
}