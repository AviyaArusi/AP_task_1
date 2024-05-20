#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include <string.h>
#include <signal.h>


void ignore_interrupt(int num)
{
    printf("\nYou typed Control-C!\n");
    printf("hello: ");
    fflush(stdout);
}

void handle_exit()
{
    exit(0); // Exit if the command is 'quit'
}

void handle_echo(char **argv) {
    int i = 1;
    while (argv[i] != NULL) {
        printf("%s", argv[i]);
        if (argv[i + 1] != NULL)
            printf(" ");  // Add space between words, but not after the last word
        i++;
    }
    printf("\n");  // Add a newline at the end
}

void handle_cd(char* dir_name)
{
    if (dir_name == NULL) {
        fprintf(stderr, "cd: expected argument\n");
    } else {
        if (chdir(dir_name) != 0) {
            perror("cd");
        }
    }
}

void check_redirection_operators(int* redirect, char** argv, char** outfile, int limit)
{
    for (int j = 0; j < limit; j++) {
        if (strcmp(argv[j], "2>") == 0) {
            *redirect = 3;
            argv[j] = NULL;
            *outfile = argv[j + 1];
            break;
        } else if (strcmp(argv[j], ">>") == 0) {
            *redirect = 2;
            argv[j] = NULL;
            *outfile = argv[j + 1];
            break;
        } else if (strcmp(argv[j], ">") == 0) {
            *redirect = 1;
            argv[j] = NULL;
            *outfile = argv[j + 1];
            break;
        }
    }
}
void redirected(int redirect_type, char* outfile)
{
    int fd;
    // Handle output redirection ">"
    if (redirect_type == 1) {
        fd = creat(outfile, 0660);
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
    // Handle append redirection ">>"
    if (redirect_type == 2) {
        fd = open(outfile, O_WRONLY | O_CREAT | O_APPEND, 0660);
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
    // Handle stderr redirection "2>"
    if (redirect_type == 3) {
        fd = creat(outfile, 0660);
        dup2(fd, STDERR_FILENO);
        close(fd);
    }
}

int main()
{
    char command[1024];
    char *token;
    char *outfile;
    int i, fd, amper, redirect, retid, status;
    char *argv[10];

    signal(SIGINT, ignore_interrupt); // If the user press CTL+C handel it.

    while (1)
    {
        printf("hello: ");
        fgets(command, 1024, stdin);
        command[strlen(command) - 1] = '\0';

        /* parse command line */
        i = 0;
        token = strtok (command," ");
        while (token != NULL)
        {
            argv[i++] = token;
            token = strtok (NULL, " ");
//            i++;
        }
        argv[i] = NULL;

        /* Is command empty */
        if (argv[0] == NULL)
            continue;

        // Check if the command is 'quit'
        if (strcmp(command, "quit") == 0) {
            handle_exit();
        }

        // Check if the command is 'echo'
        if (strcmp(command, "echo") == 0) {
            handle_echo(argv);
            continue; // Skip the fork and execvp process
        }

        // Check if the command is 'cd'
        if (strcmp(command, "cd") == 0) {
            handle_cd(argv[1]);
            continue; // Skip the fork and execvp process
        }

        /* Does command line end with & */
        amper = 0;
        if (i > 1 && strcmp(argv[i - 1], "&") == 0)
        {
            amper = 1;
            argv[--i] = NULL; // Correctly update `i` and ensure argv is NULL-terminated
        }

        redirect = 0;
        outfile = NULL;
        check_redirection_operators(&redirect, argv, &outfile, i);

        /* for commands not part of the shell command language */

        if (fork() == 0) {
            /* redirection of IO ? */
            if (redirect) {
                redirected(redirect, outfile);
                /* stdout is now redirected */
            }
            execvp(argv[0], argv);
            exit(1);
        }
        /* parent continues here */
        if (amper == 0)
            retid = wait(&status);
    }
}
