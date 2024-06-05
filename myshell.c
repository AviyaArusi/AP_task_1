#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_INPUT_SIZE 1024
#define MAX_ARGS 64
#define MAX_PROMPT 128
#define HISTORY_SIZE 20

int last_command_status = 0; // Variable to store the status of the last command
char prompt[MAX_PROMPT];     // Variable to store the prompt
int background = 0;
char *command_history[HISTORY_SIZE]; // Array to store the command history
int history_count = 0;               // Number of commands in history

enum states
{
    NEUTRAL,
    WANT_THEN,
    THEN_BLOCK,
    ELSE_BLOCK
}; // enum for flow control

// Signal handler for Ctrl+C
void handle_sigint()
{
    printf("\nYou typed Control-C!\n%s ", prompt);
    fflush(stdout);
}

void setup_signal_handler()
{
    // Register the signal handler for Ctrl+C
    if (signal(SIGINT, handle_sigint) == SIG_ERR)
    {
        perror("shell");
        exit(EXIT_FAILURE);
    }
}

void add_to_history(const char *input)
{
    if (history_count == HISTORY_SIZE)
    {
        free(command_history[0]);
        for (int i = 1; i < HISTORY_SIZE; i++)
        {
            command_history[i - 1] = command_history[i];
        }
        history_count--;
    }
    command_history[history_count++] = strdup(input);
}

void read_input(char *input)
{
    char *line = readline(prompt);
    if (line == NULL)
    {
        printf("\n");
        exit(0);
    }
    strncpy(input, line, MAX_INPUT_SIZE);
    input[MAX_INPUT_SIZE - 1] = '\0'; // Ensure null-terminated string
    if (*input)
    {
        add_history(input);
        add_to_history(input);
    }
    free(line);
}

void parse_input(char *input, char **args, int *num_args)
{
    *num_args = 0;
    char *token = strtok(input, " ");

    while (token != NULL)
    {
        args[*num_args] = token;
        (*num_args)++;
        token = strtok(NULL, " ");
    }

    // Set the last argument to NULL to indicate the end of the argument list
    args[*num_args] = NULL;
}

// Function to substitute variables in a command
void substitute_variables(char **args)
{
    for (int i = 0; args[i] != NULL; i++)
    {
        // Check if the argument starts with '$'
        if (args[i][0] == '$')
        {
            // Extract variable name
            char variable_name[MAX_INPUT_SIZE];
            strcpy(variable_name, args[i] + 1);

            // Find the variable in the environment
            char *variable_value = getenv(variable_name);
            // Substitute the variable with its value (or an empty string if not found)
            if (variable_value != NULL)
            {
                // Dynamically allocate memory for args[i]
                char *new_arg = malloc(strlen(variable_value) + 1);
                if (new_arg != NULL)
                {
                    // Copy variable_value into the newly allocated memory
                    strcpy(new_arg, variable_value);
                    args[i] = new_arg;
                }
            }
            else
            {
                args[i] = ""; // Set the argument to an empty string if the variable is not found
            }
        }
    }
}

int execute_command(char **args, int input_fd, int output_fd)
{
    pid_t pid;
    int status;

    pid = fork();

    // Child process
    if (pid == 0)
    {
        // Set up input/output redirection
        if (input_fd != STDIN_FILENO)
        {
            dup2(input_fd, STDIN_FILENO);
            close(input_fd);
        }

        if (output_fd != STDOUT_FILENO)
        {
            dup2(output_fd, STDOUT_FILENO);
            close(output_fd);
        }

        // Check for stdout redirection
        for (int i = 0; args[i] != NULL; i++)
        {
            if (strcmp(args[i], ">") == 0)
            {
                // Redirect stdout to a file (truncate)
                freopen(args[i + 1], "w", stdout);
                args[i] = NULL;
                break;
            }
            else if (strcmp(args[i], ">>") == 0)
            {
                // Redirect stdout to a file (append)
                freopen(args[i + 1], "a", stdout);
                args[i] = NULL;
                break;
            }
        }
        // Check for stderr redirection
        for (int i = 0; args[i] != NULL; i++)
        {
            if (strcmp(args[i], "2>") == 0)
            {
                // Redirect stderr to a file (truncate)
                freopen(args[i + 1], "w", stderr);
                args[i] = NULL;
                break;
            }
            else if (strcmp(args[i], "2>>") == 0)
            {
                // Redirect stderr to a file (append)
                freopen(args[i + 1], "a", stderr);
                args[i] = NULL;
                break;
            }
        }

        // Substitute variables before executing the command
        substitute_variables(args);

        // Execute the command
        if (execvp(args[0], args) == -1)
        {
            perror("shell");
        }
        exit(EXIT_FAILURE);
    }
    // Forking error
    else if (pid < 0)
    {
        perror("shell");
        last_command_status = -1; // Set status to indicate an error
    }
    // Parent process
    else
    {
        if (!background)
        {
            do
            {
                waitpid(pid, &status, WUNTRACED);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        }
        last_command_status = WEXITSTATUS(status);
    }
    return 1;
}

int execute_pipeline(char ***commands, int num_commands)
{
    int pipe_fds[2];
    int input_fd = STDIN_FILENO;

    for (int i = 0; i < num_commands; i++)
    {
        if (i < num_commands - 1)
        {
            // Create a pipe for the intermediate commands
            if (pipe(pipe_fds) == -1)
            {
                perror("shell");
                exit(EXIT_FAILURE);
            }

            // Execute the command with input from the previous command's output
            execute_command(commands[i], input_fd, pipe_fds[1]);

            // Close write end of the pipe in the parent process
            close(pipe_fds[1]);

            // Set the input for the next command to the read end of the pipe
            input_fd = pipe_fds[0];
        }
        else
        {
            // Last command in the pipeline
            execute_command(commands[i], input_fd, STDOUT_FILENO);
        }
    }

    return 1;
}

int main()
{
    char input[MAX_INPUT_SIZE];
    char input_backup[MAX_INPUT_SIZE];
    char *args[MAX_ARGS];
    char **commands[MAX_ARGS];
    int num_args, num_commands;

    char org_prompt[MAX_PROMPT]; // Variable to store the original prompt
    strcpy(prompt, "shell: ");
    strcpy(org_prompt, "shell: ");

    setup_signal_handler(); // Set up the signal handler for Ctrl+C

    int if_state = NEUTRAL;
    int if_result;

    while (1)
    {
        int j = 0;
        if (if_state == NEUTRAL)
            strcpy(prompt, org_prompt);
        else
            strcpy(prompt, ">");

        read_input(input);
        strcpy(input_backup, input);

        parse_input(input_backup, args, &num_args);
        if (num_args == 0)
            continue;
        // Handle flow control
        if (strcmp(args[0], "if") == 0) // if command
        {
            if (num_args > 1)
            {
                if_state = WANT_THEN;
                j = 3;
            }
            else // syntax error
            {
                fprintf(stderr, "shell: expected command after if\n");
                last_command_status = -1; // Set status to indicate an error
                if_state = NEUTRAL;
                continue;
            }
        }
        else if (if_state == WANT_THEN) // want then command
        {
            if_result = last_command_status;
            if (strcmp(args[0], "then") == 0) // then block
            {
                if_state = THEN_BLOCK;
                j = 5;
                if (num_args == 1)
                    continue;
                if (if_result != 0)
                    continue;
            }
            else // syntax error
            {
                fprintf(stderr, "shell: expected then after if command\n");
                last_command_status = -1; // Set status to indicate an error
                if_state = NEUTRAL;
                continue;
            }
        }
        if (strcmp(args[0], "else") == 0) // else block
        {
            if (if_state == THEN_BLOCK)
            {
                if_state = ELSE_BLOCK;
                j = 5;
                if (num_args == 1)
                    continue;
                if (if_result == 0)
                    continue;
            }
            else // syntax error
            {
                fprintf(stderr, "shell: expected else only after then block\n");
                last_command_status = -1; // Set status to indicate an error
                if_state = NEUTRAL;
                continue;
            }
        }
        if (strcmp(args[0], "fi") == 0)
        {
            if (num_args > 1)
            {
                fprintf(stderr, "shell: command not found\n");
                if_state = NEUTRAL;
                last_command_status = -1; // Set status to indicate an error
            }
            if ((if_state == THEN_BLOCK) || (if_state == ELSE_BLOCK))
            {
                if_state = NEUTRAL;
            }
            else // syntax error
            {
                fprintf(stderr, "shell: expected fi only after then block or else block\n");
                if_state = NEUTRAL;
                last_command_status = -1; // Set status to indicate an error
            }
            continue;
        }
        if ((if_state == THEN_BLOCK) && (if_result != 0))
            continue;
        if ((if_state == ELSE_BLOCK) && (if_result == 0))
            continue;

        // Exit the shell if the user enters "quit"
        if (strcmp(input, "quit") == 0)
        {
            break;
        }

        // Handle the case where the user enters "echo $?"
        if (strcmp(input + j, "echo $?") == 0)
        {
            printf("Last command status: %d\n", last_command_status);
            continue;
        }

        // Save the current command as the last command
        if (strcmp(input + j, "!!") == 0)
        {
            if (history_count == 0)
            {
                fprintf(stderr, "shell: No previous command\n");
                continue;
            }
            strcpy(input + j, command_history[history_count - 2]);
            printf("Running previous command: %s\n", input + j);
            last_command_status = 0;
        }

        // Save the current command as the last command
        if (history_count > 0 && strcmp(input, command_history[history_count - 1]) != 0)
        {
            add_to_history(input);
        }

        // parsing the input by ' '
        parse_input(input + j, args, &num_args);

        // Handle variable assignment
        if (input[j] == '$')
        {

            // Check if the assignment contains '='
            if ((num_args > 2) && (strcmp("=", args[1]) == 0))
            {
                if (num_args != 3)
                {
                    fprintf(stderr, "shell: command not found\n");
                    last_command_status = -1; // Set status to indicate an error
                }
                // Set the environment variable
                else if ((num_args == 3) && (setenv(args[0] + 1, args[2], 1) == -1))
                {
                    perror("shell");
                    last_command_status = -1; // Set status to indicate an error
                }
                last_command_status = 0;
                continue;
            }
        }

        substitute_variables(args);

        // Check if the command should run in the background
        background = 0;
        if (strcmp(args[num_args - 1], "&") == 0)
        {
            background = 1;
            args[num_args - 1] = NULL;
            num_args--; // Remove the '&' character
        }

        // Handle the "cd" command separately
        if (strcmp(args[0], "cd") == 0)
        {
            if (num_args == 2)
            {
                if (chdir(args[1]) != 0)
                {
                    perror("shell");
                    last_command_status = -1; // Set status to indicate an error
                }
            }
            else
            {
                fprintf(stderr, "shell: cd: Missing directory\n");
                last_command_status = -1; // Set status to indicate an error
            }
            last_command_status = 0;
            continue;
        }

        // Handle prompt
        if (strcmp(args[0], "prompt") == 0)
        {
            if (num_args == 3)
            {
                if (strcmp(args[1], "=") != 0)
                {
                    perror("shell");
                    last_command_status = -1; // Set status to indicate an error
                }
                strcpy(prompt, args[2]);
                strcpy(org_prompt, prompt);
            }
            else
            {
                fprintf(stderr, "shell: command not found\n");
                last_command_status = -1; // Set status to indicate an error
            }
            last_command_status = 0;
            continue;
        }

        // Handle read command
        if (strcmp(args[0], "read") == 0)
        {
            if (num_args == 2)
            {
                printf("Enter the value: ");
                fgets(input + j, MAX_INPUT_SIZE, stdin);

                // Remove the newline character at the end of the input
                input[strcspn(input, "\n")] = '\0';

                // Set the environment variable
                if (setenv(args[1], input + j, 1) == -1)
                {
                    perror("shell");
                    last_command_status = -1; // Set status to indicate an error
                }
            }
            else
            {
                fprintf(stderr, "shell: command not found\n");
                last_command_status = -1; // Set status to indicate an error
            }
            last_command_status = 0;
            continue;
        }

        // Handle piping '|'
        num_commands = 0;
        commands[num_commands] = args;

        for (int i = 1; i < num_args; i++)
        {
            if (strcmp(args[i], "|") == 0)
            {
                args[i] = NULL; // Set the current position to NULL to separate commands
                num_commands++;
                commands[num_commands] = &args[i + 1];
            }
        }

        // Substitute variables in the commands
        for (int i = 0; i <= num_commands; i++)
        {
            substitute_variables(commands[i]);
        }

        // Handle pipeline
        if (num_commands > 0)
        {
            execute_pipeline(commands, num_commands + 1);
        }
        // Single command without piping
        else
        {
            execute_command(args, STDIN_FILENO, STDOUT_FILENO);
        }
    }
    return 0;
}
