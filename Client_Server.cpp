#include <unistd.h>
#include <error.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <sstream>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdio.h>


using namespace std;


//struct
struct list {
    int pid;
    string name;
    time_t startTime;
    time_t endTime;
    string elapsedTime;
    bool active;
};

bool errorHandler(char str []){
    for( int index = 0 ; index< strlen(str); index++ ){   
        if( (str[index] >= '0' && str[index] <= '9' )  || str[index] == ' ' || str[index] == '\n' || str[index] == ';' || str[index]== '-' /*for negative values*/ || str[index]== '.' /*for decimal values*/) {  
           continue;
        
        } else{ 
            write(STDOUT_FILENO,"You have entered a non-numeric value. Please enter only numeric values.\n",71);
            return false;
        }
    }

    return true;
}

char * tokenizer(char str []){
    char *token = strtok(str, " \n");  
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
        perror("Error in creating pipe between client and server.");

    //Creating Client & Server processes
    int id = fork();
    if(id < 0)
    {
        perror("Error while forking to create client & server.");
    }
    else if (id == 0)  //Client process
    {  
        while(true){
        //variable declarations
        char command[buff_size] = {} ;
        char response[buff_size] = {};
        int ret;
        char output_message_for_commands [] = "You have the following commands: add, sub, mul, div, run, kill, list, listall, exit.\n" ;
        char output_message_for_input [] = "Enter your command: ";
        cout <<endl;

        //Screen display
        if(write(STDOUT_FILENO, &output_message_for_commands , strlen(output_message_for_commands)) < 0)
            perror("Error while displaying message.1 ");

        if(write(STDOUT_FILENO, &output_message_for_input, strlen(output_message_for_input)) < 0)
            perror("Error while displaying message.2 "); 
        //User input  
        ret = read(STDIN_FILENO, command, buff_size); 
        if(ret < 0)
            perror("Error while displaying message.3 ");
        
        
        //Sending command to server
        if(write(fd[1], command, ret) < 0)
            perror("Error while displaying message.4 ");

        sleep(1);

        if(command[0]=='e' && command[1]=='x' && command[2]=='i' && command[3]=='t' && command[4]=='\n'){
                exit(getpid());
        }

        ret = read (fd[0], response, buff_size);
        if (ret < 0)
            perror("Error in reading response."); 

        if(write(STDOUT_FILENO, response, ret) < 0)
            perror("Error while displaying response");
        
       
        
        cout<<"\n";
        }

    }
    else   //Server process
    {   
        list processList [50];

        while(true){
        //Variable declarations
        int ret;
        char recieved_command [buff_size] = {};     
        cout <<endl;
      
        //Recieving command from client
        ret = read(fd[0], recieved_command, buff_size);
        if(ret < 0)
            perror("Error while reading from pipe b/w client & server.");
    
        char * tokens = tokenizer(recieved_command);
        string token = (string) tokens;
        
        if( (token == "add") || (token == "sub") || (token == "mul") || (token == "div") )
        {   
           char buff [buff_size]; 
           tokens = strtok(NULL," ");
           bool allowed = false;
           double answer = 0;
           bool f =false;

           while(tokens!=NULL)
           {
                allowed = errorHandler(tokens);
              //  cout<<allowed;
                if(!allowed){
                    break;
                }
               
                if ( *tokens == ';')
                {
                    int n = sprintf(buff, "ANSWER: %.2f\n", answer);
                    int w = write(fd [1],buff,n);
                    if(w < 0)
                        perror("Error while displaying message.6 ");
                    tokens = strtok(NULL, " ");
                    answer = 0;
                } 
                else if (*tokens == '\n')
                {
                    continue;
                }
                else 
                {
                    if (token == "add" )
                    {
                        answer += atof(tokens); 
                    }
                    else if (token =="sub")
                    {
                        if (!f){
                            answer=atof(tokens);
                            f= true;
                        }else{
                            answer -= atof(tokens);
                        }
                        
                    }
                    else if (token == "mul")
                    {
                        if (answer == 0){
                            answer = 1;
                        }
                        answer = answer * atof(tokens);
                    }else {
                        if (*tokens != '0')
                        {
                            if (answer == 0){
                                answer = atof(tokens);
                            }else{
                                answer = answer / atof(tokens);
                            }       
                        }
                        else
                        {
                            cout<<"can't divide by 0"<<endl;
                            break;
                        } 
                        
                    }
                    
                    tokens = strtok(NULL, " ");
                } 

           }
           sleep(1);
        }
        else if (token == "run")
        {   
            //Pipe for child processes
            char buff [buff_size];
            int execPipe[2];
            if (pipe2(execPipe, O_CLOEXEC) < 0)
                perror("Error in piping for exec");
            
            //Forking for exec
            int pid = fork();
            if(pid < 0)
            {
                perror("error while forking in run. ");
            }
            else if (pid > 0)
            {   
                
                tokens = strtok(NULL, " \n");
                
                
                if(write(execPipe[1], tokens, strlen(tokens)) < 0)
                    perror("Error while writing on execpipe. ");

                sleep(1);
                close(execPipe[1]);
                ret = read(execPipe[0], buff, buff_size);

                if(ret == 0){
                    int i = 0;
                    while(processList[i].pid != 0){
                        i++;
                    }
                    processList[i].pid = pid;
                    processList[i].name = tokens;
                    time_t currentTime;    
                    processList[i].startTime = currentTime;
                    processList[i].active = true;
                    if(write(fd[1], "success", 7) < 0)
                        perror("Error while displaying message.7 ");
                }else{
                    if(write(fd[1], buff, strlen(buff)) < 0)
                         perror("Error while piping message. ");
                }


            }
            else 
            {
                char App [buff_size];
                char path[buff_size] = {'/','u','s','r','/','b','i','n','/'};
                int i;
                int ret = read(execPipe[0], App, buff_size);

                close(execPipe[0]);

                if(ret < 0)
                    perror("Error while reading filename. ");

                for (int i = 0; i < strlen(App); i++)
                {
                    path[9 + i] = App [i];
                }                

                if(execlp(path,path,NULL) < 0)
                    perror("Error while exec()");
                

                if(write(execPipe[1], "failed", 6)< 0)
                    perror("Error while piping message. ");

             }
            
        }
        else if (token == "kill") //segmentation dump occurs when killing a non opened file.
        {
            tokens = strtok(NULL, " \n");

            int x = atoi (tokens);
            if (x > 0){
                ret = kill(x,SIGTERM);
                if (ret < 0){
                if(write(fd[1], "process not killed",17))
                    perror("Error while piping. ");
                }else{
                write(fd[1], "successfully killed",19);                 
                }
                int i = 0;
                while(processList[i].pid != x){
                        i++;
                }
                processList[i].active = false;
                time_t currentTime;    
                processList[i].endTime = currentTime;

                processList[i].elapsedTime = difftime(processList[i].endTime,processList[i].startTime);

            }else{
                int j = 0;
                bool isfound;
                while( processList[j].name!= tokens){
                    j++;
                    isfound = true;
                }

                if (isfound){
                ret = kill(processList[j].pid, SIGTERM);
                if (ret < 0){
                if(write(fd[1], "process not killed",17))
                    perror("Error while piping. ");
                }else{
                write(fd[1], "successfully killed",19);                 
                }
                processList[j].active = false;
                time_t currentTime;    
                processList[j].endTime = currentTime;

                processList[j].elapsedTime = difftime(processList[j].endTime,processList[j].startTime);
                }else {
                        if(write(fd[1], "process not killed",17))
                    perror("Error while piping. ");
                }
            }
        }
        else if (token == "exit")
        {
              exit(getpid());
              sleep(1);
        }
        else if (token == "list" || token == "listall")
        {
              if(token == "listall"){

              }else{
                  
              }
        }
        else 
        {
            if(write(fd[1], "invalid", 7) < 0)
                perror("Error while piping message. ");

        }
        sleep(1);
        }
    }
    return 0;
}