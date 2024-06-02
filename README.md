# Custom Shell

This is a custom shell implementation in C, featuring various functionalities such as command execution, redirection, piping, environment variables, conditional execution, and more.

## Features

- **Command Execution:** Execute commands entered by the user.
- **Redirection:** Redirect standard output (`>`), standard error (`2>`), and append (`>>`) to files.
- **Piping:** Allow multiple commands to be chained together using pipes (`|`).
- **Environment Variables:** Support for environment variables, including setting and echoing variables.
- **Conditional Execution:** Support for basic if-else conditional execution.
- **Built-in Commands:** Includes built-in commands like `cd`, `prompt`, `quit`, `read`, and `echo`.

## Usage with Makefile
Compilation
To compile your custom shell, simply run the following command in your terminal:

bash
Copy code
make
This command will compile the source files (final_shell.c, queue.c, keyval_table.c) into an executable named myshell.

Cleaning Up
If you want to remove the compiled object files and the executable, you can use the following command:

bash
Copy code
make clean
This command will delete all .o files and the myshell executable.

### Dependencies
The Makefile has a few dependencies:

queue.o: Compiled object file for the queue data structure.
final_shell.o: Compiled object file for the main shell program.
keyval_table.o: Compiled object file for the key-value table implementation.
These dependencies ensure that your shell is built correctly and that changes in any of the source files trigger recompilation as necessary.

Customization
If you want to modify the source files or add new ones, simply update the Makefile accordingly. You can add new source files and update the compilation rules as needed.

Other Targets
The Makefile also defines a .PHONY target to ensure that the all and clean targets are always considered out of date. This prevents potential conflicts with files named all or clean in the directory.
