Custom Shell
This is a custom shell implementation in C, featuring various functionalities such as command execution, redirection, piping, environment variables, conditional execution, and more.

Features
Command Execution: Execute commands entered by the user.
Redirection: Redirect standard output (>) and standard error (2>) to files, and append (>>) to files.
Piping: Allow multiple commands to be chained together using pipes (|).
Environment Variables: Support for environment variables, including setting and echoing variables.
Conditional Execution: Support for basic if-else conditional execution.
Built-in Commands: Includes built-in commands like cd, prompt, quit, read, and echo.
Usage
To use the shell, simply compile the source code (shell.c) and run the executable:

bash
Copy code
gcc shell.c -o shell
./shell
Once the shell is running, you can enter commands as you would in a regular shell environment. Here are some examples of supported commands:

Execute a command:

bash
Copy code
ls -l
Redirect output to a file:

bash
Copy code
ls -l > output.txt
Append output to a file:

bash
Copy code
echo "Hello, World!" >> output.txt
Execute multiple commands in a pipeline:

bash
Copy code
ls -l | grep "txt" | wc -l
Set environment variables:

bash
Copy code
var1=value1
Conditional execution:

bash
Copy code
if [ true ]; then
    echo "Condition is true"
else
    echo "Condition is false"
fi
Built-in commands:

Change directory:

bash
Copy code
cd /path/to/directory
Change shell prompt:

bash
Copy code
prompt = "CustomShell"
Exit the shell:

bash
Copy code
quit
Contributors
Your Name
License
This project is licensed under the MIT License - see the LICENSE file for details.
