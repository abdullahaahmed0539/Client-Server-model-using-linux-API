#include <unistd.h>
#include <error.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <stdlib.h>
#include <signal.h>

using namespace std;

void errorHandler(char str []){
    for( int index = 0 ; index< strlen(str)-1; index++ ){   
        if( (str[index] >= '0' && str[index] <= '9' )  || str[index] == ' ' || str[index] == '\n' || str[index] == ';' || str[index]== '-' /*for negative values*/ || str[index]== '.' /*for decimal values*/) {  
           continue;
        
        } else{ 
            write(STDOUT_FILENO,"You have entered a non-numeric value. Please enter only numeric values.\n",71);
            exit(0);
        }
    }
}

char * tokenizer(char str []){
    char *token = strtok(str, " ");  
    return token;
}

int main (){
    int buff_size = 100;

    /*
        Creating pipe
        fd[0] ----> for reading
        fd[1] ----> for writing
    */
    int fd[2];
    if (pipe(fd) < 0)
        perror("Error in piping on line 19: " + errno);

    //Creating Client & Server processes
    int id = fork();
    if(id < 0)
    {
        perror("Error while creating Client and Server processes in line 23: "+ errno);
    }
    else if (id == 0)  //Client process
    {  
        //variable declarations
        char command[buff_size], response[buff_size];
        int ret;
        char output_message_for_commands [] = "You have the following commands: add, sub, mul, div, run, exit.\n" ;
        char output_message_for_input [] = "Enter your command: ";

        //Screen display
        if(write(STDOUT_FILENO, &output_message_for_commands , strlen(output_message_for_commands)) < 0)
            perror("Error while displaying message in line 40: " + errno);

        if(write(STDOUT_FILENO, &output_message_for_input, strlen(output_message_for_input)) < 0)
            perror("Error while displaying message in line 43: " + errno);

        //User input  
        ret = read(STDIN_FILENO, command, buff_size); 
        if(ret < 0)
            perror("Error while taking user input in line 48: " + errno); 
        
        
        //Sending command to server
        if(write(fd[1], command, ret) < 0)
            perror("Error while displaying message in line 52: " + errno);
    }
    else   //Server process
    {   
        //Variable declarations
        int ret;
        char recieved_command [buff_size];     

        //Recieving command from client
        ret = read(fd[0], recieved_command, buff_size);
        if(ret < 0)
            perror("Error while reading from pipe in line: "+ errno);
    
        char * tokens = tokenizer(recieved_command);
        string token = (string) tokens;
        
        if( (token == "add") || (token == "sub") || (token == "mul") || (token == "div") )
        {   

            //errorHandler(recieved_command);
        }
        else if (token == "run")
        {   
            //Pipe for child processes
            int fd1[2];
            if (pipe(fd1) < 0)
                perror("Error in piping on line 19: " + errno);
            
            //Forking for exec
            int pid = fork();
            if(pid < 0)
            {
                perror("error while forking in run: "+errno);
            }
            else if (pid > 0)
            {
                tokens = strtok(NULL, " \n");
                if(write(fd1[1], tokens, strlen(tokens)) < 0)
                    perror("error"+ errno);
                                                    
            }
            else 
            {
                char App [buff_size];
                char path[buff_size] = {'/','u','s','r','/','b','i','n','/'};
                ret = read(fd1[0], App, buff_size);
                if(ret < 0)
                    perror("error"+ errno);

                for (int i = 0; i < strlen(App); i++)
                {
                    path[9 + i] = App [i];
                }
    
                if(execlp(path,path,NULL) < 0)
                    perror(""+errno);
            }
            
        }
        else if (token == "exit")
        {

        }
        else
        {
            char err_message [] = "Illegal instruction Entered. Program will terminated.";
        }
      
    
    
    
    
    }
    return 0;
}