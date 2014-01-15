#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

void exit_jshell(int status){
    exit(status);
}

char** parse_pipes(char* list){
    //parsing piped commands into array of commands with flags & args
    char* parsed = NULL;
    char cmds_array[1024][4096]; //this should probably be of variable length..
    //char delims[] = {" \n"};
    parsed = strtok(list, " | ");
    int i = 0;
    while(parsed != NULL){
        printf("%s\n", parsed);
        strcpy(cmds_array[i], parsed);
        i++;
        parsed = strtok(NULL, " | ");
    }
    return cmds_array;
}

void parse_cmds(int *list){
    //parsing individual commands
    //just same as above parse_pipes but different delimitter...
    //will make single function in future
    
    char* parsed = NULL;
    char cmds_array[1024][4096]; //this should probably be of variable length..
    //char delims[] = {" \n"};
    parsed = strtok(list, " ");
    int i = 0;
    while(parsed != NULL){
        printf("%s\n", parsed);
        strcpy(cmds_array[i], parsed);
        i++;
        parsed = strtok(NULL, " ");
    }
    return cmds_array;
}
 
int main(int argc, char **argv)
{
    //decide on a prompt
    char* prompt;
    char** cmds_array;
    if(argc >= 2){
        prompt = argv[1];
    }else{
        prompt = "Jsh: ";
    }
    
    
    //parseInput();
     /* Our first simple C basic program */
     
    char input_buffer[4096];
    while(1){
        fprintf(stdout, prompt);
        if (fgets(input_buffer, sizeof(input_buffer), stdin) == 0){ // end of file (CTRL+D) => exit jshell
            fprintf(stdout, "\n");
            exit_jshell(0);
        }else{
            cmds_array = parse_pipes(input_buffer);
            char **exec_args = (char **) malloc(sizeof(char *)*3);
            exec_args[0] = "ls";
            exec_args[1] = "-la";
            exec_args[2] = NULL;
            
            int status;
            
            if(fork() == 0){ // child process
                int code = execvp(exec_args[0],exec_args);
                fprintf(stderr, "jsh: %s: Error %d, %s\n", exec_args[0], errno, strerror(errno));
                exit(1);
            }else{ //jshell process
                wait(&status);
            }
        }
    }
    
    
    
    return 0;
}