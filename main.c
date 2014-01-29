#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


//Structure to hold indivdual command information
typedef struct{
	char* name;
	int in_fd, out_fd; //file descriptor values in & out
	char** args; //arguments for the command
	int arg_count; //number of args
	int background; //run in background? 1 yes : 0 no
	int read_file; //read from file? 1 yes : 0 no (read from command line)
	int write_file; //write to file? 1 yes : no
	char* read_file_name; 
	char* write_file_name;
	int append; //append? 1 yes : no
} COMMAND;

void exit_jshell(int status){
    //TODO: clean up memory.
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


// copied from: http://stackoverflow.com/questions/122616/how-do-i-trim-leading-trailing-whitespace-in-a-standard-way
void trim(char * s) {
    char * p = s;
    int l = strlen(p);

    while(isspace(p[l - 1])) p[--l] = 0;
    while(* p && isspace(* p)) ++p, --l;

    memmove(s, p, l + 1);
}


char** parse(char* list, const char a_delim){
    /*Built to be a generic parsing function, but primarilary used to
      parse piped commands into array of commands between pipes*/
    char* parsed = NULL;
    char **return_array;
    char delim[2]; 
	delim[0]= a_delim;
	delim[1] = '\0';
    int num_of_commands = count_cmds(list);
    //printf("%i commands\n", num_of_commands);
    return_array = malloc(num_of_commands * sizeof(char*));
  
    parsed = strtok(list, delim);
    int i = 0;
    while(parsed != NULL){
	    trim(parsed); //ensuring no extra whitespace
        //printf("%s\n", parsed);
	    return_array[i] = malloc(strlen(parsed) + 1);
	    strcpy(return_array[i], parsed);
        i++;
        parsed = strtok(NULL, delim);
        
    }
    return_array[i] = NULL;
    return return_array;
}

COMMAND* parse_cmds(char** input){
    /*Function to parse and properly structure each individual command.
      Returns a list of COMMANDS*/
      COMMAND* commands;
      char** holder_array;
      //commands = malloc(sizeof(input) * sizeof(char*) + sizeof(COMMAND)) 
      commands = malloc(1024 * sizeof(COMMAND)); //mem alloc for commands array
      int i = 0; 
      int x = 0;
      
      //loop through the input array of commands with args until NULL terminator
      while(input[i] != NULL){
        holder_array = parse(input[i], ' ');
        
        //allocate mem for each index
        //commands[i] = malloc(sizeof(COMMAND*)); //THIS MAY BE REPETATIVE IF 'MALLOC'ED ABOVE...?
        
        //assigning name of command
        commands[i].name = holder_array[0]; 
        commands[i].args = holder_array; //putting name in first element of args array
        x++;
        
        //assigning defualt values for command[i];
        commands[i].in_fd = 0; //STDIN
        commands[i].out_fd = 1; //STDOUT 
        commands[i].arg_count = x; // x = 1 here... only one argument for certain
        commands[i].background = 0; //don't background
        commands[i].read_file = 0; //read from STDIN NOT file
        commands[i].write_file = 0; //write to STDOUT NOT file
        commands[i].read_file_name = NULL; //read from STDIN NOT file
        commands[i].write_file_name = NULL; //write to STDOUT NOT file
        commands[i].append = 0; //append? 1 yes : 0 no
        
        //loop through the different parts of each command to determine more COMMAND info
        while(holder_array[x] != NULL){
            if(strcmp(holder_array[x], "&") == 0){
                commands[i].background = 1; //background flag set
            }
            if(strcmp(holder_array[x], "<") == 0){
                commands[i].read_file = 1; //read from file flag set NOT from cmdline
                commands[i].read_file_name = holder_array[x+1]; //grabbing file name (assuming must be directly after)
            }
            if(strcmp(holder_array[x], ">>") == 0){
                commands[i].append = 1; //append flag set 
                commands[i].write_file_name = holder_array[x+1]; //grabbing file name (assuming must be directly after)
            }
            else if(strcmp(holder_array[x], ">") == 0){
                commands[i].write_file = 1; //write to file flag set
                commands[i].write_file_name = holder_array[x+1]; //grabbing file name (assuming must be directly after)
            }
            
            commands[i].arg_count++;
            x++;
        }
        
        	printf("command name: %s\n", commands[i].name);
	printf("command background?: %i\n", commands[i].background);
	printf("command write file?: %i\n", commands[i].write_file);
	printf("command append file?: %i\n", commands[i].append);
	printf("command write file name: %s\n", commands[i].write_file_name);	
	printf("command read file?: %i\n", commands[i].read_file);
	printf("command read file name: %s\n", commands[i].read_file_name);
        
        x = 0;
        i++;
      }
      
      commands[i].arg_count = 0; //acting as terminator
      return commands;
}
 
int main(int argc, char **argv)
{
    //decide on a prompt
    char* prompt;
    COMMAND* cmds_array;
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
            cmds_array = parse_cmds(parse(input_buffer, '|'));
            /*
            char **exec_args = (char **) malloc(sizeof(char *)*3);
            exec_args[0] = "ls";
            exec_args[1] = "-la";
            exec_args[2] = NULL;
            */
            
            //count the number of commands
            int i = 0;
            while(cmds_array[i].arg_count > 0){
                i++;
            }
            int num_cmds = i;
            
            int status;
            
            int pipefd[2];
            int* prevpipefd;
            int first = 1;
            int last = 0;
            int last_child_pid = 0;
            i = 0;
            int filefd;
            
            while(i < num_cmds){
                
                if(i == num_cmds-1){
                    last = 1;
                }else{
                    if (pipe(pipefd) < 0) {
                        perror("jsh: pipe");
                        exit(1);
                    }
                }

                if((last_child_pid = fork()) == 0){ // child process
                    
                    
                    //read from pipe (unless first)
                    if(!first){
                        close(prevpipefd[1]);
                        dup2(prevpipefd[0],0);//TODO: error check
                        close(prevpipefd[0]);
                    }
                    
                    if(first && cmds_array[i].read_file){
                        filefd = open(cmds_array[i].read_file_name, O_RDONLY, 0666);
                        dup2(filefd,0);//TODO: error check
                        close(filefd);
                    }
                    
                    if(!last){
                        //write to pipe
                        dup2(pipefd[1],1);//TODO: error check
                        
                        close(pipefd[0]);
                        close(pipefd[1]);
                    }
                    if(last && (cmds_array[i].write_file || cmds_array[i].append)){
                        int mode;
                        if(cmds_array[i].append){
                            mode = O_RDWR | O_APPEND | O_CREAT;
                        }else{
                            mode = O_RDWR | O_TRUNC |  O_CREAT;
                        }
                        
                        filefd = open(cmds_array[i].write_file_name, mode, 0666);
                        
                        dup2(filefd,1);//TODO: error check
                        close(filefd);
                    }
                    
                    /*  good error code: 
                        if (dup2(pipefd[1], 1) != 1) {
                            perror("jsh: dup2");
                         exit(1);
                        }
                    */
                    
                    int code = execvp(cmds_array[i].name , cmds_array[i].args);
                    fprintf(stderr, "jsh: %s: Error %d, %s\n", cmds_array[i].name, errno, strerror(errno));
                    exit(1);
                }else{ //jshell process
                    
                    if(first){
                        first = 0;
                    }else{
                        close(prevpipefd[1]);
                        close(prevpipefd[0]);
                    }
                    prevpipefd = pipefd;
                    
                    if(filefd > 0){
                        close(filefd);
                    }
                    filefd = 0;
                     
                }
                i++;
            }
            close(prevpipefd[1]);
            close(prevpipefd[0]);
            
            int wait_status;
            //TODO: implement &
            waitpid((pid_t) last_child_pid, &wait_status, 0);
        }
    }
    
    
    
    return 0;
}
