# Shell

This is a custom shell implementation in C, featuring various functionalities such as command execution, redirection, piping, environment variables, conditional execution, and more.

## Features

- **Command Execution:** Execute commands entered by the user.
- **Redirection:** Redirect standard output (`>`), standard error (`2>`), and append (`>>`) to files.
- **Piping:** Allow multiple commands to be chained together using pipes (`|`).
- **Environment Variables:** Support for environment variables, including setting and echoing variables.
- **Conditional Execution:** Support for basic if-else conditional execution.
- **Built-in Commands:** Includes built-in commands like `cd`, `prompt`, `quit`, `read`, and `echo`.

## Makefile
Compilation
To compile your custom shell, simply run the following command in your terminal:


`make`<br><br>
This command will compile the source files (final_shell.c, queue.c, keyval_table.c) into an executable named myshell.

Cleaning Up
If you want to remove the compiled object files and the executable, you can use the following command:


`make clean`<br><br>
This command will delete all .o files and the myshell executable.

### Dependencies
The Makefile has a few dependencies:<br>
**queue.o:** Compiled Object File for Command History
The queue.o file contains the compiled object code for the queue data structure used in the command history feature of the shell. This feature enables users to navigate through previously executed commands using the arrow keys (up and down). Each command entered by the user is stored in the history queue, allowing for easy access and retrieval. The `queue.o` file ensures the smooth functioning of this command history mechanism within the shell.

**keyval_table.o:** Compiled Object File for Variable Storage
The keyval_table.o file contains the compiled object code for the key-value table implementation utilized in storing variables. When the `read` command is invoked within the shell, this component of the shell infrastructure handles the assignment of user input to specified variables. For instance, when a user employs the `read` command to capture input and assign it to a variable, such as `read VAR`, the `keyval_table.o` module is responsible for efficiently managing and storing this association between the variable name (`VAR`) and the input provided by the user. Thus, `keyval_table.o` plays a crucial role in enabling the functionality of the `read` command within the shell environment.

These dependencies ensure that your shell is built correctly and that changes in any of the source files trigger recompilation as necessary.

Customization
If you want to modify the source files or add new ones, simply update the Makefile accordingly. You can add new source files and update the compilation rules as needed.

Other Targets
The Makefile also defines a .PHONY target to ensure that the all and clean targets are always considered out of date. This prevents potential conflicts with files named all or clean in the directory.

## Usage

To compile the shell, you can use the provided Makefile. Simply navigate to the directory containing the source code (`shell.c`) and the Makefile and run the following command:

```bash
make
```

```bash
./myshell
```

> [!NOTE]
> Before running the shell, make sure to download the source code files from the Git repository.

