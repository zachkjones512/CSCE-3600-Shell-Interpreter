# Major 2 - Shell

## Authors

- Zachary Jones
- Anh Vo
- Grayson Allo

## Project Organization

### Zachary Jones
- Implemented `read_line`, `split_line`, `execute` functions.
- Implemented `exit` built-in function.
- Worked on I/O redirection.

### Anh Vo
- Implemented `path` built-in function.
- Focused on code clarity and documentation.

### Grayson Allo
- Implemented pipeline functionality.
- Worked on signal handling.

## Design Overview

Overall the code utilizes three major functions that are called in each command loop. The first is the read_line function which reads the user input into a character buffer that is passed to the split_line function. The split_line function then tokenizes each argument in the buffer that is separated by whitespace delimiters using the strtok standard library method. Following this, the results are entered into an array "args" that is then executed in order in the execute function. 

### Built-in Commands
The built-in commands play a crucial role in the overall functionality of the shell. When a user enters the exit command, the shell gracefully concludes its execution, ensuring that ongoing processes are completed before termination. The cd command, another built-in, facilitates the seamless alteration of the current working directory.The path command manages the shell's pathname list, offering three distinct functionalities. When invoked without arguments, it displays the current pathnames separated by colons. 

### Execution Workflow
The execute function has two major components. The first component is a section that checks if the executed function matches any of the three functions that are built-in to the shell: CD, Path, and Exit. If any of these are matches, then it executes those functions without forking a child process. Otherwise, the program forks the process, checks for input/output redirection, and then utilizes a child process to execute the user's task.

### Running the Shell
When the program is executed by typing "make major2" into your unix terminal, you can run the program using ./newshell. Following this, the main function calls the shell loop which prompts the user for an input until the user closes the program, passes through the interrupt signal, or inputs exit into the shell. 

## Complete Specification

The program has utilizes defensive programming throughout in order to ensure that errors are helpfully communicated to the user if any error is thrown regarding problems with the execute function, string parsing, buffer reallocation, and reading the buffer. These help ensure that the program runs following these errors without crashes while also educating the user on program or input errors that could produce an unexpected result.

## Known Bugs or Problems

Due to the size of the group, there are parts of the project description that remain unimplemented. For instance, the alias command that is mandatory for teams of 4 is unimplemented in our project because our group contains only three members. This is also true for the built-in command myhistory as our three group members each chose to implement the exit, path, and CD built-ins.

## Testing

The shell has been thoroughly tested for various scenarios, including:
- Input with and without whitespace.
- Execution of built-in commands.
- Handling of I/O redirection.
- Pipeline functionality.
- Graceful termination on user interrupt (Ctrl-C).

## Future Work

Future improvements and features may include:
