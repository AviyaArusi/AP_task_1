# Custom Shell

This is a custom shell implementation in C, featuring various functionalities such as command execution, redirection, piping, environment variables, conditional execution, and more.

## Features

- **Command Execution:** Execute commands entered by the user.
- **Redirection:** Redirect standard output (`>`), standard error (`2>`), and append (`>>`) to files.
- **Piping:** Allow multiple commands to be chained together using pipes (`|`).
- **Environment Variables:** Support for environment variables, including setting and echoing variables.
- **Conditional Execution:** Support for basic if-else conditional execution.
- **Built-in Commands:** Includes built-in commands like `cd`, `prompt`, `quit`, `read`, and `echo`.

## Usage

To use the shell, simply compile the source code (`shell.c`) and run the executable:

```bash
gcc shell.c -o shell
./shell
