// CS537 Summer 2022
// P2 - B: Shell
// Michael Tang and Jaewon Choi
// mtang59@wisc.edu and choi326@wisc.edu

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>

int main(int argc, char** argv){
    int interactive; // 0 if batch mode (default), 1 if interactive mode
    char* inv_args = "Usage: mysh [batch-file]\n";
    char* file_error = "Error: Cannot open file "; // add [file].\n after use
    char* inv_cmd = ": Command not found.\n"; // add [job] before use
    char* redir_error = "Redirection misformatted.\n";
    char* fork_error = "Forking error.\n";
    char* write_error = "Cannot write to file "; // add [file].\n
    char* prompt = "mysh> "; // interactive mode prompt
    char fileLines[512]; 
    char delimiters[] = "\n\t\v\f\r "; // for use in strtok()
    char* token;
    char* destination; // stores destination of redirect
    char* commands[512];
    char output[512];
    FILE *file;

    // select mode based off argc
    if (argc == 1){ // interactive mode
        interactive = 1;
        file = stdin;
        write(1, prompt, strlen(prompt));
    }
    else if (argc == 2){ // batch mode, argv[1] is file name
        interactive = 0;
        file = fopen(argv[1], "r");
        if (file == NULL){  // check for invalid file
            write(STDERR_FILENO, file_error, strlen(file_error));
            write(STDERR_FILENO, argv[1], strlen(argv[1]));
            write(STDERR_FILENO, ".\n", strlen(".\n"));
            exit(1);
        }
    }
    else{ // invalid arguments
        write(STDERR_FILENO, inv_args, strlen(inv_args));
        return 1;
    }
    while (fgets(fileLines, 512, file) != NULL){
        char* current = fileLines;
        char* current_end = current + strlen(current) - 1;
        int index = 0; // token counter
        int redirect = 0; // 1 if we need to redirect
        int redirects = 0; // number of ">" symbols
        int status;

        // check if line is made up of only white space
        if (current == 0 || strlen(current) == 0){
            if (interactive == 1){
                write(1, prompt, strlen(prompt));
            }
            else{
                write(1, current, strlen(current));
                write(1, "\n", strlen("\n"));
                continue;
            }
        }

        if (interactive == 0){ // echo to batch file
            write(1, fileLines, strlen(fileLines));
        }
        
        // skip whitespace after line
        while (isspace((unsigned char)*current_end) != 0 && current_end > current){
            current_end--;
        }
        current_end[1] = '\0'; // null character at end
        

        // count number of ">" and account for redirection errors
        for (int i = 0; i <= strlen(current); i++){
            if (current[i] == '>'){ 
            }
        }
        if (redirects > 1 || current[0] == '>' || 
                current[strlen(current) - 1] == '>'){
            write(STDERR_FILENO, redir_error, strlen(redir_error));
            continue;
        }

        // check for white space after ">"
        token = strtok(current, ">"); // token before ">"
        destination = strtok(NULL, "> "); // token after ">"
        char* extra_token = strtok(NULL, ">"); // should be null
        if (extra_token != NULL){
            write(STDERR_FILENO, redir_error, strlen(redir_error));
            continue;
        }
        if (destination != NULL){ 
            while (isspace((unsigned char)*destination)){ // clear white space in between
                destination++;
            }
            current_end = destination + strlen(destination) - 1;
            while (current_end > destination && isspace((unsigned char)*current_end)){ // clear white space after
                current_end--;
            }
            current_end[1] = '\0'; // null character at end
            // check for misformatting
            for (int i = 0; i < strlen(destination); i++){
                if (isspace(destination[i])){
                    write(STDERR_FILENO, redir_error, strlen(redir_error));
                    if (interactive == 1){
                        write(1, prompt, strlen(prompt));
                    }
                    continue;
                }
                snprintf(output, strlen(destination) + 1, "%s\n", destination);
            }
            redirect = 1;
        }

        // create command tokens
        token = strtok(current, delimiters);
        while (token != NULL && index < 50){ // if not "exit", store tokens in commands[]
            if (strcmp(token, "exit") == 0){
                fclose(file);
                exit(0);
            }
            commands[index] = token;
            index++; // increment for next loop
            token = strtok(NULL, delimiters);
        }
        commands[index] = NULL;

        // parent and child processes
        pid_t pid = fork(); // return value = pid of child if parent, 0 if child
        if (pid == -1){ // fork error
            write(STDERR_FILENO, fork_error, strlen(fork_error));
            exit(1);
        }
        else if (pid == 0){ // child process
            // check if redirect file is valid
            if (redirect == 1){ 
                int redirect_fd = open(output, O_RDWR | O_CREAT | O_TRUNC, 0666);
                if (redirect_fd == -1){ // file cannot be opened error
                    write(STDERR_FILENO, write_error, strlen(write_error));
                    write(STDERR_FILENO, argv[1], strlen(argv[1]));
                    write(STDERR_FILENO, ".\n", strlen(".\n"));
                    continue;
                }
                dup2(redirect_fd, 1); // duplicate
                close(redirect_fd);
            }
            execv(commands[0], commands);
            // print error message if it fails
            write(STDERR_FILENO, commands[0], strlen(*commands));
            write(STDERR_FILENO, inv_cmd, strlen(inv_cmd));
            continue;
        }
        else{ // parent process
            waitpid(pid, &status, 0); // wait for child to finish before continuing
            if (interactive == 1){
                write(1, prompt, strlen(prompt));
            }
        }


    }
    fclose(file);
}