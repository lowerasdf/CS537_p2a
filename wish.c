#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <ctype.h>
#include <fcntl.h>

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

int parse(char* line, char** args, int *redirection_num, int *redirection_idx) {
    char *sentence;
    const char *delimiters = " \t\n";
    int i = 0;
    int j = 1;
    *redirection_num = 0;
    *redirection_idx = 0;

    while((sentence = strsep(&line, ">")) != NULL) {
        *redirection_num += 1;
        if (strlen(sentence) != 0) {
            char* token;
            while((token = strsep(&sentence, delimiters)) != NULL) {
                if (i >= MAX_ARGS_COUNT || token == NULL) {
                    break;
                }
                if (strlen(token) != 0) {
                    // args[i] = token;
                    args[i] = malloc((strlen(token) + 1) * sizeof(char));
                    if (args[i] == NULL) {
                        return -1;
                    }
                    strcpy(args[i], token);
                    // printf("token no.%d: %s\n", i, args[i]);
                    i += 1;
                }
            }
            if (j) {
                *redirection_idx += i;
                j = 0;
            }
        }
    }
    *redirection_num -= 1;
    if (*redirection_num == 0) {
        *redirection_idx = 0;
    }

    return i;
}

void execute(char *args[], char *path, int redirection_idx, char *error_message) {
    int rc = fork();
    if (rc < 0) {
        write(STDERR_FILENO, error_message, strlen(error_message));
    } else if (rc == 0) {
        if (redirection_idx > 0) {
            char* temp[redirection_idx];
            for (int i = 0; i < redirection_idx; i++) {
                // char curr[strlen(args[i]) + 1];
                // strcpy(curr, args[i]);
                // temp[i] = curr;
                temp[i] = args[i];
            }
            temp[redirection_idx] = NULL;

            // close(STDOUT_FILENO);
	        int file = open(args[redirection_idx], O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
            dup2(file, STDOUT_FILENO);
            dup2(file, STDERR_FILENO);
            close(file);

            int err = execv(path, temp);
            if (err == -1) {
                file = open(args[redirection_idx], O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
                write(file, error_message, strlen(error_message));
                close(file);
            }
        } else {
            int err = execv(path, args);
            if (err == -1) {
                write(STDERR_FILENO, error_message, strlen(error_message));
            }
        }
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
    int num_args = 0;

    char *path[MAX_PATH_COUNT];
    path[0] = malloc((strlen(DEFAULT_PATH) + 1) * sizeof(char));
    if (path[0] == NULL) {
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
    }
    strcpy(path[0], DEFAULT_PATH);
    path[1] = NULL;

    int loop_count = 0;
    char *temp_args[MAX_ARGS_COUNT];
    int max_count = 0;

    int redirection_num = 0;
    int redirection_idx = 0;

    for(;;) {
        // int i = 0;
        // while(path[i] != NULL) {
        //     printf("PATH: %s, ", path[i]);
        //     i += 1;
        // }
        // printf("\n");

        for(int i = 0; i < num_args; i++) {
            free(args[i]);
        }

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

            num_args = parse(line, args, &redirection_num, &redirection_idx);
            if (num_args == -1) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                continue;
            }
            args[num_args] = NULL;   
        } else {
            int count = max_count - loop_count + 1;
            int loop_count_size = snprintf(NULL, 0, "%d", count);
            int err = 0;
            for (int i = 0; i < num_args; i++) {
                if (strcmp(temp_args[i], "$loop") == 0) {
                    args[i] = malloc((loop_count_size + 1) * sizeof(char));
                    if (args[i] == NULL) {
                        err = 1;
                        break;
                    }
                    sprintf(args[i], "%d", count);
                } else {
                    args[i] = malloc(sizeof(temp_args[i]));
                    if (args[i] == NULL) {
                        err = 1;
                        break;
                    }
                    // memcpy(args[i], temp_args[i], sizeof(temp_args[i]));
                    strcpy(args[i], temp_args[i]);
                }
            }
            args[num_args] = NULL;

            if (err) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                continue;
            }

            loop_count -= 1;

            if (loop_count == 0) {
                for (int i = 0; i < num_args; i++) {
                    free(temp_args[i]);
                }
            }
        }

        // for (int i = 0; i < num_args; i++) {
        //     if (strcmp(args[i], ">") == 0) {
        //         redirection_num += 1;
        //         redirection_idx = i;
        //     }
        // }
        // if (redirection_num > 1 || (redirection_num == 1 && num_args - redirection_idx - 1 != 1)) {
        //     write(STDERR_FILENO, error_message, strlen(error_message));
        //     continue;
        // }
        if (redirection_num > 1 || (redirection_num == 1 && num_args - redirection_idx != 1)) {
            write(STDERR_FILENO, error_message, strlen(error_message));
            continue;
        }

        // printf("REDIRECTION IDX: %d\n", redirection_idx);
        // for(int i = 0; i < num_args; i++){
        //     printf("arg #%d: %s, ", i, args[i]);
        // }
        // printf("\n");

        if (args[0] == NULL) {
            continue;
        } else if (strcmp(args[0], "exit") == 0) {
            if(num_args > 1) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                continue;
			}
            exit(0);
        } else if (strcmp(args[0], "cd") == 0) {
            if(num_args != 2) {
                write(STDERR_FILENO, error_message, strlen(error_message)); 
                continue;
			}
            int res = chdir(args[1]);
            if(res != 0) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                continue;
			}
        } else if (strcmp(args[0], "path") == 0) {
            int i = 0;
            while(path[i] != NULL) {
                free(path[i]);
                i += 1;
            }

            i = 0;
            int err = 0;
            while(args[i+1] != NULL && i < MAX_PATH_COUNT) {
                path[i] = malloc((strlen(args[i+1]) + 1) * sizeof(char));
                if (path[i] == NULL) {
                    err = 1;
                    break;
                }
                strcpy(path[i], args[i+1]);
                i += 1;
            }
            if (err) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                continue;
            }

            path[i] = NULL;
        } else if (strcmp(args[0], "loop") == 0) {
            if (num_args < 2 || !isNumber(args[1])) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                continue;
            }

            loop_count = atoi(args[1]);
            max_count = loop_count;
            // free(args[0]);
            // free(args[1]);
            int err = 0;
            for (int i = 2; i < num_args; i++) {
                temp_args[i-2] = malloc(sizeof(args[i]));
                if (temp_args[i-2] == NULL) {
                    err = 1;
                    break;
                }
                //memcpy(temp_args[i-2], args[i], sizeof(args[i]));
                strcpy(temp_args[i-2], args[i]);
                // args[i-2] = args[i];
            }
            if (err) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                continue;
            }
            temp_args[num_args - 2] = NULL;
            num_args = num_args - 2;
            redirection_idx = redirection_idx - 2;
            // args[num_args-2] = NULL;
            // num_args -= 2;
        } else {
            int i = 0;
            int found = 0;
            while (path[i] != NULL) {
                char str[strlen(path[i]) + strlen("/" + strlen(args[0])) + 1];
                strcpy(str, path[i]);
                strcat(str, "/");
                strcat(str, args[0]);

                int fd = access(str, X_OK);
                if (fd == -1) {
                    i += 1;
                } else {
                    execute(args, str, redirection_idx, error_message);
                    found = 1;
                    break;
                }
            }
            if (found == 0) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                continue;
            }
        }

        // for(int i = 0; i < num_args; i++) {
        //     free(args[i]);
        // }
        // free(args);
    }

    for(int i = 0; i < num_args; i++) {
        free(args[i]);
    }

    int i = 0;
    while(path[i] != NULL) {
        free(path[i]);
        i += 1;
    }
}