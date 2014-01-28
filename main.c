#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

<<<<<<< HEAD
//Structures to pass arguments between pthreads
typedef struct{
	char flag;
	char** input; //parameters for the flag... 
		      //could be more than one so array of inputs
} OPTION; 

typedef struct{
	char* name;
	OPTION* options;
} COMMAND;

void exit_jshell(int status){
    exit(status);
}

int count_cmds(char* list){
    int i, count;
    for(i=0 ; list[i]!='\0' ; i++)
	if(list[i]=='|')
	    count++;
    //return count + 1;
    return 1024;
}

char** parse(char* list, const char a_delim){
    //parsing piped commands into array of commands with flags & args
    char* parsed = NULL;
    char **cmds_array;
    char delim[2]; 
	delim[0]= a_delim;
	delim[1] = '\0';
    int num_of_commands = count_cmds(list);
    //printf("%i commands\n", num_of_commands);
    cmds_array = malloc(num_of_commands * sizeof(char*));
    //char delims[] = {" \n"};
    parsed = strtok(list, delim);
    int i = 0;
    while(parsed != NULL){
	//want to remove leading white space here... TODO
        printf("%s\n", parsed);
	cmds_array[i] = malloc(strlen(parsed) + 1);
	strcpy(cmds_array[i], parsed);
        i++;
        parsed = strtok(NULL, delim);
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
    
    
    //parse input

     /* Our first simple C basic program */
     
    char input_buffer[4096];
    while(1){
        fprintf(stdout, prompt);
	fgets(input_buffer, sizeof(input_buffer), stdin);
        if (input_buffer == 0 // end of file (CTRL+D) => exit jshell
            || strcmp(input_buffer, "exit\n") == 0){
            fprintf(stdout, "\n");
            exit_jshell(0);
        }else{
            cmds_array = parse(input_buffer, '|');
		
        //testing if cmds_array has what it should
	    int x = 0;
	    while(cmds_array[x] != '\0'){
		    printf("%i: %s\n", x, cmds_array[x]);
		    x++;
        }

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
