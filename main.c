/**************************************************************************

@file           main.c

@authors        Group 13    (Zachary Jones, Grayson Allo, Anh Vo)

@class          CSCE3600

@date           Tuesday, November 14 2023

@desc           Implements a shell using C programming

*************************************************************************/

#include <stdlib.h> //exit
#include <unistd.h> //pipe() and fork()
#include <stdio.h> //stderr
#include <string.h> //strtok
#include <fcntl.h> //open()
#include <signal.h>

#define BUFFER 512
#define num_builtins 3 //update this with the number of builtins i.e. cd, exit, path


// Function prototypes
char *read_line();                      // function to read a line of input from the user
char **split_line(char *line);          // function to split a line into arguments using whitespace delimiters
int execute(char **args);               // function to execute a command with its arguments
void shell_exit();                      // built-in function to exit the shell gracefully
void shell_cd(char **args);             // built-in function to change the current working directory
void shell_path(char **args);           // built-in function to manage the shell's pathname list
void show_path();                       // display the current pathnames in the shell
void add_path(char *new_path);          // add a pathname to the shell's path variable
void remove_path(char *removed_path);   // remove a pathname from the shell's path variable
void pipeline();                        // function to handle pipelining of commands
void sigHand(int signal);               // signal handling function for the shell

// Global variables
char *path[BUFFER];     // array to store pathnames in the shell
int num_paths = 0;      // number of pathnames currently stored in the path array

// function to read a line of input from the user
char *read_line(){

    int buffersize = BUFFER; //buffer size
    int index = 0; //index for while loop
    char *buff = malloc(sizeof(char) * buffersize);
    int c;

    if (!buff) {
        fprintf(stderr, "buffer allocation error\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        c = getchar();

        //if we hit EOF or a newline replace with null
        if (c == EOF || c == '\n') {
            buff[index] = '\0';
            return buff; //return the buffer array as line
        }
        else{
            buff[index] = c; //otherwise read the character into the buff array
        }
        index++;
    }
}

// function to split a line into arguments using whitespace delimiters
char ** split_line(char *line){

    int buffersize = BUFFER; //buffer
    char **args = malloc(buffersize * sizeof(char*)); //argument array
    char *arg; //individual arg
    int index = 0; //index

    char *delimiters = " \t\r\n";//list of delimters (space, tab, return, newline)

    if (!args) { //if allocation fails return an error
        fprintf(stderr, "args allocation error\n");
        exit(EXIT_FAILURE);
    }

    arg = strtok(line, delimiters); //standard library command that parses string by delims
    while (arg != NULL){
        args[index] = arg; //parse each argument into the args array
        index++;
        arg = strtok(NULL, delimiters);
    }

    args[index] = NULL;
    return args;
}

// function to execute a command with its arguments
int execute(char **args){ //used to execute functions

    pid_t pid, pidw;
    int status;

    if (args[0] == NULL) {
        //no program passed through to execute
        return 1;
    }
    
    else if (strcmp(args[0], "exit") == 0) { //check if calling a builtin
        shell_exit();//call built in function
    }
    else if (strcmp(args[0], "cd") == 0) { //check if calling a builtin
        shell_cd(args);//call built in function
    }
    else if (strcmp(args[0], "path") == 0) { //check if calling a builtin
        shell_path(args);//call built in function
    }
    
    else{ //not a builtin and not null
        

        pid = fork();
    
        if (pid == 0) {
            //child process
            int input_fd, output_fd; //file descriptors for i/o redirec

            //check for input redirection
            for (int i = 1; args[i] != NULL; i++) {
                if (strcmp(args[i], "<") == 0) {//redirector found

                    //opens the input file
                    int input_fd = open(args[i + 1], O_RDONLY);
                    if (input_fd == -1) {
                        perror("input redirection error");
                        exit(EXIT_FAILURE);
                    }

                    //directs stdin to the input file
                    dup2(input_fd, STDIN_FILENO);

                    //closes the fd since it's no longer needed
                    close(input_fd);

                    //removes the redirectors and filename from args
                    args[i] = NULL;
                    args[i + 1] = NULL;

                    break;//stops the loop
                }
            }

            //check for output redirection
            for (int i = 1; args[i] != NULL; i++) {
                if (strcmp(args[i], ">") == 0) { //if redirector found

                    //opens the output file
                    int output_fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0666);
                    if (output_fd == -1) {
                        perror("output redirection error");
                        exit(EXIT_FAILURE);
                    }

                    //directs stdout to the output file
                    dup2(output_fd, STDOUT_FILENO);

                     //closes the fd since it's no longer needed
                    close(output_fd);

                     //removes the redirectors and filename from args
                    args[i] = NULL;
                    args[i + 1] = NULL;

                    break;//stops the loop
                }
            }

            //execute command
            if (execvp(args[0], args) == -1) {
                perror("execution error");
            }
                exit(EXIT_FAILURE);
        
        }
    
        else if (pid < 0){ //fork error
            perror("Error forking");
        }
    
        else{
            //parent process
            do {
                pidw = waitpid(pid, &status, WUNTRACED); //wait until child process exits
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));

        }
    }
    
    return 1; //let the caller know we can run the shell loop again
}


/*          ADD BUILTIN COMMANDS BELOW: CD, EXIT, PATH           */

// built-in function to exit the shell gracefully
void shell_exit(){
    exit(0);
}

// built-in function to change the current working directory
void shell_cd(char **args){
    if (args[1] == NULL) {
           // No argument provided, change to the HOME directory
           chdir(getenv("HOME"));
       } else {
           if (chdir(args[1]) != 0) {
               perror("Error changing directory");
           }
       }
   }

// built-in function to manage the shell's pathname list
void shell_path(char **args) {
    if (args[1] == NULL) {
        // Display current path
        show_path();
    } else if (args[2] != NULL && strcmp(args[1], "+") == 0) {
        // Add a path
        add_path(args[2]);
    } else if (args[2] != NULL && strcmp(args[1], "-") == 0) {
        // Remove a path
        remove_path(args[2]);
    } else {
        // Invalid usage of path command
        printf("Invalid usage of path command\n");
    }
}

// function to display the current pathnames in the shell
void show_path() {
    printf("Path: ");
    for (int i = 0; i < num_paths; ++i) {
        printf("%s", path[i]);
        if (i < num_paths - 1) {
            printf(":");
        }
    }
    printf("\n");
}

// function to add a pathname to the shell's path variable
void add_path(char *new_path) {
    if (num_paths < BUFFER) {
        // add the new pathname to the path variable
        path[num_paths++] = new_path;
    } else {
        // display an error if the path list is full
        printf("Path list is full. Cannot add more paths.\n");
    }
}

// function to remove a pathname from the shell's path variable
void remove_path(char *removed_path) {
    int index = -1;

    // Find the index of the path to be removed
    for (int i = 0; i < num_paths; ++i) {
        if (strcmp(path[i], removed_path) == 0) {
            index = i;
            break;
        }
    }

    // Remove the path from the list
    if (index != -1) {
        for (int i = index; i < num_paths - 1; ++i) {
            path[i] = path[i + 1];
        }
        num_paths--;
    } else {
        // Display an error if the specified path is not found
        printf("Path not found. Cannot remove.\n");
    }
}

// Function to handle pipelining of commands
void pipeline() {

  int pipefd[2];
  pid_t child1, child2;

  // Create pipe
  if (pipe(pipefd) == -1) {

    perror("pipe");
    exit(EXIT_FAILURE);

  }

  // Fork first child
  if ((child1 = fork()) == -1) {

    perror("fork");
    exit(EXIT_FAILURE);

  }

  if (child1 == 0) {
  //first command
    close(pipefd[0]);
    dup2(pipefd[1], STDOUT_FILENO);//Redirect stdout
    close(pipefd[1]);
    execlp("cmd1", "cmd1", NULL);//Replace child with cmd1
    perror("execlp cmd1");
    exit(EXIT_FAILURE);
  } else {

    if ((child2 = fork()) == -1) {

      perror("fork");
      exit(EXIT_FAILURE);

    }

    if (child2 == 0) {

      close(pipefd[1]);
      dup2(pipefd[0], STDIN_FILENO);//Redirect stdin
      close(pipefd[0]);
      execlp("cmd2", "cmd2", NULL);//Replace child with cmd2
      perror("execlp cmd2");
      exit(EXIT_FAILURE);
    } else {
            // Parent process
      close(pipefd[0]);
      close(pipefd[1]);

    }
  }
}

// Signal handling function for the shell
void sigHand(int signal) {

    if (signal == SIGINT) {
        
        // Handle Ctrl-C (SIGINT) signal
        printf("Received Ctrl-C\n");
    
    }
}

/*          LOOP and MAIN function below                         */

//command loop function
void loop(){

    char *line;
    char **args;
    int status;
    int signal;


    do {
        printf("> ");
        line = read_line();
        args = split_line(line);
        status = execute(args);
        // signal = sigHand(signal);
        sigHand(SIGINT); // Call sigHand directly with SIGINT

        free(line);
        free(args);
        //free(signal);
    } while (status);
}



int main(int argc, char **argv)
{
    // Handle Ctrl-C (SIGINT) signal
    signal(SIGINT, sigHand);

    //run command loop
    loop();


    //shutdown process
    return EXIT_SUCCESS;
}

