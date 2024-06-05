#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include <string.h>

// The added includes:
#include <signal.h>
#include <ncurses.h>
#include "queue.h"
#include "keyval_table.h"
#include <termios.h>
#include <ctype.h> // Required for isspace

#define MAX_ARGS 128
#define MAX_COMMANDS 20

// The added define:
char SHELLNAME[1024] = "hello: ";
int MAX_COMMAND_LENGTH = 1024;
int LCS = 0; // Global variable to store the exit status of the last executed command
Queue CMD_QUEUE;
KeyValueTable TABLE;
struct termios original;
int VAR = 0;

void ignore_interrupt(int num)
{
    printf("\nYou typed Control-C!\n");
    printf("%s", SHELLNAME);
    fflush(stdout);
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

void handle_cat(char *outfile)
{
    restoreOriginalMode(&original);

    FILE *fp;
    char buffer[1024]; // Adjust buffer size as needed

    // Open the file "ron.txt" in write mode ("w")
    fp = fopen(outfile, "w");

    // Use a loop to get user input line by line
    while (fgets(buffer, sizeof(buffer), stdin) != NULL)
    {
        if (fputs(buffer, fp) == EOF)
        {
            perror("Error writing to file");
            fclose(fp);
            exit(EXIT_FAILURE);
        }
    }
    fclose(fp);
    setRawMode(&original);
    LCS = 0;
    printf("Control-D\n");
}

int check_redirection_operators(char **argv, char **outfile, int limit)
{
    for (int j = 0; j < limit; j++)
    {
        if (strcmp(argv[j], "2>") == 0)
        {
            argv[j] = NULL;
            *outfile = argv[j + 1];
            return 3;
        }
        else if (strcmp(argv[j], ">>") == 0)
        {
            argv[j] = NULL;
            *outfile = argv[j + 1];
            return 2;
        }
        else if (strcmp(argv[j], ">") == 0 && strcmp(argv[j - 1], "cat") == 0)
        {
            return 4;
        }
        else if (strcmp(argv[j], ">") == 0)
        {
            argv[j] = NULL;
            *outfile = argv[j + 1];
            return 1;
        }
    }
    return 0;
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

void handle_exit()
{
    freeTable(&TABLE);
    freeQueue(&CMD_QUEUE);
    exit(0); // Exit if the command is 'quit'
}

void free_args(char **args, int count)
{
    for (int i = 0; i < count; i++)
    {
        free(args[i]);
    }
    free(args);
}

char **parse_command(char **start_token, int *argc)
{
    char **argv = malloc(MAX_ARGS * sizeof(char *));
    int i = 0;
    char *token;
    char *line = *start_token;

    while ((token = strsep(&line, " ")) != NULL)
    {
        if (*token == '\0')
            continue; // Skip any empty tokens
        argv[i++] = strdup(token);
    }

    argv[i] = NULL; // Null-terminate the argument list
    *argc = i;
    return argv;
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

void handle_variables_shell(char **argv)
{
    char key[1024];
    key[0] = '\0';
    char value[1024];
    int index = 0;
    for (int i = 0; argv[0][i] != '\0'; i++)
    {
        char c = argv[0][i];
        if (c != '$')
        {
            key[index++] = c;
        }
    }
    key[index] = '\0';
    value[strlen(argv[2])] = '\0';
    insert(&TABLE, key, argv[2]);
}

void handle_prompt(char name[])
{
    strcat(name, ": ");
    strcpy(SHELLNAME, name);
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
    LCS = 0;
    printf("\n"); // Add a newline at the end
}

void handle_echo_variable(char *key)
{
    char check_key[1024];
    check_key[0] = '\0';
    int index = 0;
    for (int i = 0; key[i] != '\0'; i++)
    {
        char c = key[i];
        if (c != '$')
        {
            check_key[index++] = c;
        }
    }
    check_key[index] = '\0';
    char *value = getValue(&TABLE, check_key);
    LCS = 0;
    printf("%s\n", value);
}

void handle_cond_execute(const char *cmd)
{
    // Use system() to execute the command for simplicity
    system(cmd);
}

// Function to read the command char char
void read_command(char *command)
{
    int index = 0;
    int if_flag = 0;
    int execute_flag = 0;
    int arrow_up_counter = 0;
    Node *curr = NULL;
    char c = '\0';
    char paragraph[1024 * 10];
    paragraph[0] = '\0';
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
                        printf("\b \b");
                        fflush(stdout);
                    }
                    strcpy(command, curr->data);
                    printf("\r%s%s", SHELLNAME, command);
                    index = strlen(command);
                    fflush(stdout);
                }
                else
                {
                    while (index > 0)
                    {
                        index--;
                        command[index] = '\0';
                        printf("\b \b");
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

            if (if_flag)
            {
                strcat(paragraph, command);
                strcat(paragraph, "\n");
                if (strcmp(command, "fi") == 0)
                {
                    execute_flag = 1;
                }
                if (execute_flag)
                {
                    system(paragraph);
                    paragraph[strlen(paragraph)] = '\0';
                    enqueue(&CMD_QUEUE, paragraph);
                    paragraph[0] = '\0';
                    if_flag = 0;
                    execute_flag = 0;
                    break;
                }
            }
            else
            {
                if (VAR)
                {
                    VAR = 0;
                    break;
                }

                if (strlen(command) > 0)
                {
                    enqueue(&CMD_QUEUE, command);
                }
                int counter = 0;
                if (4 < strlen(command))
                {
                    if (command[0] == 'i' && command[1] == 'f')
                    {
                        counter++;
                    }

                    if (command[strlen(command) - 3] == 'f' && command[strlen(command) - 2] == 'i')
                    {
                        counter++;
                    }
                }
                if (counter == 2)
                {
                    system(command);
                }
                break;
            }
            index = 0;
            command[0] = '\0';
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
            if (strcmp(command, "if") == 0 && index == 2)
            {
                if_flag = 1;
            }
        }
    }
}

void handle_last_exec()
{
    char *cmd = CMD_QUEUE.front->next->data;
    int status = system(cmd);
}

void handle_last_cmd()
{
    printf("%d\n", LCS);
}

void handle_read(char **argv)
{
    char key[1024];
    key[0] = '\0';
    char value[1024];
    value[0] = '\0';
    if (argv[1] != NULL)
    {
        strcpy(key, argv[1]);
    }
    else
    {
        strcpy(key, "REPLY");
    }
    LCS = 0; 
    printf("");
    VAR = 1;
    read_command(value);
    insert(&TABLE, key, value);
}
void check_echo(char **argv, int argc)
{
    // Run the status of the last command
    if (strcmp(argv[1], "$?") == 0) // with the ?
    {
        handle_last_cmd();
        return;
    }
    // Print the value of the variable
    if (argv[1][0] == '$')
    {
        handle_echo_variable(argv[1]);
        return; // Skip the fork and execvp process
    }
    // Check if the command is 'echo'
    if (strcmp(argv[1], "$?") != 0)
    {
        handle_echo(argv);
        return; // Skip the fork and execvp process
    }
}
void handle_exce(char *command, char **argv, int argc)
{
    if (strlen(command) > 0)
    {
        char c = argv[0][0];
        if (c == '$' && argv[1] != NULL && strcmp(argv[1], "=") == 0 && argv[2] != NULL)
        {

            handle_variables_shell(argv);
        }
    }

    /* Is command empty */
    if (argv[0] == NULL)
        return;

    char *outfile;
    outfile = NULL;
    int redirect = check_redirection_operators(argv, &outfile, argc);
    if (redirect == 0)
    { // Check if the command is 'quit'
        if (strcmp(command, "quit") == 0)
        {
            handle_exit();
        }

        // Read the command and store it
        if (strcmp(argv[0], "read") == 0)
        {
            handle_read(argv);
            return;
        }

        if (!strcmp(argv[0], "echo"))
        {
            check_echo(argv, argc);
            return;
        }

        // Check if the command is '!!'
        if (strcmp(argv[0], "!!") == 0)
        {
            handle_last_exec();
            return; // Skip the fork and execvp process
        }

        // Check if the command is 'cd'
        if (strcmp(argv[0], "cd") == 0)
        {
            handle_cd(argv[1]);
            return; // Skip the fork and execvp process
        }

        // handle the name of the shell
        if (strcmp(argv[0], "prompt") == 0 && strcmp(argv[1], "=") == 0)
        {
            handle_prompt(argv[2]);
            return; // Skip the fork and execvp process
        }
    }
    // Aviya deleted a need to return!
    int retid;
    /* Does command line end with & */
    int amper = 0;
    if (argc > 1 && strcmp(argv[argc - 1], "&") == 0)
    {
        amper = 1;
        argv[--argc] = NULL; // Correctly update i and ensure argv is NULL-terminated
    }

    /* for commands not part of the shell command language */
    int pid = fork();
    if (pid == 0)
    {
        if (redirect < 4 && redirect > 0) // to exec in the start of the function?!?!?!?!?!?!??!
        {
            redirected(redirect, outfile);
        }

        if (redirect == 4)
        {
            handle_cat(argv[2]);
        }
        else
        {
            execvp(argv[0], argv);
        }

        exit(1);
    }
    else
    { // Parent process
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status))
        {
            LCS = WEXITSTATUS(status); // Update global variable with exit status
        }
    }
    /* parent continues here */
    if (amper == 0)
        retid = wait(&LCS);
}

int main()
{
    char command[1024];
    int fds[2], oldfds;
    char *token;
    char **cmds[MAX_COMMANDS];
    int argcs[MAX_COMMANDS], cmd_count = 0;

    // struct termios original;
    setRawMode(&original); // Set terminal to raw mode

    // Initialize a queue for the command
    initializeQueue(&CMD_QUEUE);
    // Initialize a table for the variables
    initializeTable(&TABLE);

    // If the user press CTL+C handel it.
    signal(SIGINT, ignore_interrupt);

    while (1)
    {
        printf("%s", SHELLNAME);
        fflush(stdout);        // Ensure the prompt is displayed immediately
        read_command(command); // Read command with arrow key handling
        if (strlen(command) == 0)
            continue;

        token = strtok(command, "|"); // ls -l
        char *token_copy = strdup(token);
        while (token != NULL)
        {
            cmds[cmd_count] = parse_command(&token, &argcs[cmd_count]);
            cmd_count++;
            token = strtok(NULL, "|"); // ls -l
        }
        cmds[cmd_count] = NULL;

        if (cmd_count == 1)
        {
            handle_exce(token_copy, cmds[0], argcs[0]);
            cmd_count = 0;
            continue;
        }

        for (int k = 0; k < cmd_count; k++) // Run cmd_count pipes.
        {
            if (k < cmd_count - 1)
            { // Need to pipe to the next command
                pipe(fds);
            }

            int pid = fork();
            //            if (fork() == 0)
            if (pid == 0)
            { // Child process

                if (k > 0)
                { // Not the first command - define the input from the prev pipe.
                    dup2(oldfds, STDIN_FILENO);
                    close(oldfds);
                }

                if (k < cmd_count - 1)
                { // Not the last command - define the output to the next pipe.
                    close(fds[0]);
                    dup2(fds[1], STDOUT_FILENO);
                    close(fds[1]);
                }

                handle_exce(cmds[k][0], cmds[k], argcs[k]);

                exit(0);
            }

            if (k > 0)
            { // Not the first command
                close(oldfds);
            }

            if (k < cmd_count - 1)
            { // Not the last command
                close(fds[1]);
                oldfds = fds[0];
            }

            free_args(cmds[k], argcs[k]);
        }

        while (cmd_count > 0)
        {
            wait(NULL);
            cmd_count--;
        }
    }

    return 0;
}
