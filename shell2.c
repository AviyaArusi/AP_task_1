#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include <string.h>
#include <signal.h>
#include <ncurses.h>
#include "queue.h"
#include <termios.h>

char SHELLNAME[1024] = "hello: ";
int MAX_COMMAND_LENGTH = 1024;
Queue CMD_QUEUE;
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

void handle_echo(char **argv)
{
    int i = 1;
    while (argv[i] != NULL)
    {
        printf("%s", argv[i]);
        if (argv[i + 1] != NULL)
            printf(" "); // Add space between words, but not after the last word
        i++;
    }
    printf("\n"); // Add a newline at the end
}

void handle_prompt(char name[])
{
    strcat(name, ": ");
    strcpy(SHELLNAME, name);
}

void handle_cd(char *dir_name)
{
    if (dir_name == NULL)
    {
        fprintf(stderr, "cd: expected argument\n");
    }
    else
    {
        if (chdir(dir_name) != 0)
        {
            perror("cd");
        }
    }
}

void check_redirection_operators(int *redirect, char **argv, char **outfile, int limit)
{
    for (int j = 0; j < limit; j++)
    {
        if (strcmp(argv[j], "2>") == 0)
        {
            *redirect = 3;
            argv[j] = NULL;
            *outfile = argv[j + 1];
            break;
        }
        else if (strcmp(argv[j], ">>") == 0)
        {
            *redirect = 2;
            argv[j] = NULL;
            *outfile = argv[j + 1];
            break;
        }
        else if (strcmp(argv[j], ">") == 0)
        {
            *redirect = 1;
            argv[j] = NULL;
            *outfile = argv[j + 1];
            break;
        }
    }
}
void redirected(int redirect_type, char *outfile)
{
    int fd;
    // Handle output redirection ">"
    if (redirect_type == 1)
    {
        fd = creat(outfile, 0660);
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
    // Handle append redirection ">>"
    if (redirect_type == 2)
    {
        fd = open(outfile, O_WRONLY | O_CREAT | O_APPEND, 0660);
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
    // Handle stderr redirection "2>"
    if (redirect_type == 3)
    {
        fd = creat(outfile, 0660);
        dup2(fd, STDERR_FILENO);
        close(fd);
    }
}

// To handle the arrow button
// Function to set terminal to raw mode
void setRawMode(struct termios *original)
{
    struct termios raw;
    tcgetattr(STDIN_FILENO, original); // Get current terminal attributes
    raw = *original;                   // Copy them to modify

    raw.c_lflag &= ~(ECHO | ICANON);          // Disable echo and canonical mode
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw); // Set modified attributes
}

// Function to restore terminal to original mode
void restoreOriginalMode(struct termios *original)
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, original);
}

void readCommand(char *command)
{
    int index = 0;
    char c = '\0';
    int arrow_up_counter = 0;
    Node *curr = NULL;
    command[0] = '\0';

    while (1)
    {
        c = getchar();
        if (c == '\033')
        {              // Escape sequence
            getchar(); // Skip '['
            switch (getchar())
            {
            case 'A': // Up arrow key
                if (CMD_QUEUE.front == NULL || arrow_up_counter == CMD_QUEUE.size)
                {
                    break;
                }
                if (arrow_up_counter == 0 && CMD_QUEUE.size != 0)
                {
                    arrow_up_counter++;
                    curr = CMD_QUEUE.front;
                }
                else if (curr != NULL && curr->next != NULL)
                {
                    arrow_up_counter++;
                    curr = curr->next;
                }
                if (curr != NULL)
                {
                    while (index > 0)
                    {
                        index--;
                        command[index] = '\0';
                        printf("\b \b"); // Move cursor back, print space to erase character, move cursor back again
                        fflush(stdout);
                    }
                    strcpy(command, curr->data);
                    printf("\r%s%s", SHELLNAME, command);
                    index = strlen(command);
                    fflush(stdout);
                }
                break;
            case 'B': // Down arrow key
                if (arrow_up_counter > 0 && curr != NULL && curr->prev != NULL)
                {
                    arrow_up_counter--;
                    curr = curr->prev;
                    while (index > 0)
                    {
                        index--;
                        command[index] = '\0';
                        printf("\b \b"); // Move cursor back, print space to erase character, move cursor back again
                        fflush(stdout);
                    }
                    strcpy(command, curr->data);
                    printf("\r%s%s", SHELLNAME, command);
                    index = strlen(command);
                    fflush(stdout);
                }
                else if (curr->prev == NULL)
                {
                    while (index > 0)
                    {
                        index--;
                        command[index] = '\0';
                        printf("\b \b"); // Move cursor back, print space to erase character, move cursor back again
                        arrow_up_counter = 0;
                        fflush(stdout);
                    }
                }
                break;
            }
        }
        else if (c == '\n')
        { // Enter key
            command[index] = '\0';
            printf("\n");
            if (strlen(command) > 0)
            {
                enqueue(&CMD_QUEUE, command);
            }
            break;
        }
        else if (c == 127)
        { // Backspace key
            if (index > 0)
            {
                index--;
                command[index] = '\0';
                printf("\b \b"); // Move cursor back, print space to erase character, move cursor back again
                fflush(stdout);
            }
        }
        else
        {
            if (index < MAX_COMMAND_LENGTH - 1)
            {
                command[index++] = c;
                command[index] = '\0';
                putchar(c); // Echo the character
                fflush(stdout);
            }
        }
    }
}
int main()
{
    char command[1024];
    char *token;
    char *outfile;
    int i, fd, amper, redirect, retid, status;
    char *argv[10];
    char c;

    struct termios original;
    setRawMode(&original); // Set terminal to raw mode

    // Create a queue for the command

    initializeQueue(&CMD_QUEUE);

    signal(SIGINT, ignore_interrupt); // If the user press CTL+C handel it.

    while (1)
    {
        printf("%s", SHELLNAME);
        fflush(stdout); // Ensure the prompt is displayed immediately

        readCommand(command); // Read command with arrow key handling
        // command[strlen(command)] = '\0';
        // printf("\n");

        // fgets(command, 1024, stdin);
        // char c = getchar();

        // printf("%s\n" , cmd_queue.front->data);
        /* parse command line */
        i = 0;
        token = strtok(command, " ");
        while (token != NULL)
        {
            argv[i++] = token;
            token = strtok(NULL, " ");
        }
        argv[i] = NULL;

        /* Is command empty */
        if (argv[0] == NULL)
            continue;

        // Check if the command is 'quit'
        if (strcmp(command, "quit") == 0)
        {
            handle_exit();
        }

        // Check if the command is 'echo'
        if (strcmp(command, "echo") == 0)
        {
            handle_echo(argv);
            continue; // Skip the fork and execvp process
        }

        // Check if the command is 'cd'
        if (strcmp(command, "cd") == 0)
        {
            handle_cd(argv[1]);
            continue; // Skip the fork and execvp process
        }

        // Change the name of the prompt
        if (strcmp(argv[0], "prompt") == 0 && strcmp(argv[1], "=") == 0)
        {
            handle_prompt(argv[2]);
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

        if (fork() == 0)
        {
            /* redirection of IO ? */
            if (redirect)
            {
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
    restoreOriginalMode(&original); // Restore terminal to original mode
}