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

        if (write(STDOUT_FILENO,"For arithmetic operations syntax example: add 1 2 ; . Add <space> ; in every command.\n", strlen("For arithmetic operations syntax example: add 1 2 ; . Add <space> ; in every command.\n"))< 0){
            perror("Error while displaying message.8 "); 
        }

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

        char * tok = tokenizer(command);
        string toktemp = (string) tok;
        if(command[0]=='e' && command[1]=='x' && command[2]=='i' && command[3]=='t' && command[4]=='\n'){
                exit(getpid());
        }
        
        
        ret = read (fd[0], response, 500);
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
        for (size_t k = 0; k < 50; k++)
        {
             processList[k].pid = 0;
        }
        

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
                    int size = strlen(tokens);
                    char a [size];
                    sprintf(a, "%s", tokens);
                    processList[i].name = a;
                    time_t currentTime;    
                    time(&currentTime);
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
                    int i = 0;
                    bool isfound;
                    
                    while(processList[i].pid !=0){
                        if(processList[i].pid==x){
                            isfound = true;
                        }  
                        i++;
                    }
                    cout << isfound<< endl;
                    if (isfound){
                    ret = kill(x,SIGTERM);
                    if (ret < 0){
                        if(write(fd[1], "process not killed as it doesnt exist.",17) < 0)
                            perror("Error while piping. ");
                    }else{
                    write(fd[1], "successfully killed",19);                 
                    }
                    int status;
                    int wait_chk = waitpid(x,&status,0);
                    if(wait_chk==-1)
                        perror("Error in waitpid");
                    int i = 0;
                    while(processList[i].pid != x){
                            i++;
                    }
                    processList[i].active = false;
                    time_t Time; 
                    time(&Time);   
                    processList[i].endTime = Time;

                    processList[i].elapsedTime = difftime(processList[i].endTime,processList[i].startTime);
                    }else {
                        write(fd[1], "unsuccessful kill",19);
                    }
            }else if (x==0){
                bool isfound;
                int i = 0;
                string name  = (string) tokens;
                while(processList[i].pid!=0){
                    if( name == processList[i].name && processList[i].active){
                        isfound = true;
                        break;
                     }
                    i++;
                }
               


                if(isfound){
                    ret = kill(processList[i].pid, SIGTERM);
                    if (ret < 0){
                        if(write(fd[1], "process not killed",18) < 0)
                            perror("Error while killing. ");
                    }else{
                        write(fd[1], "successfully killed",19);                 
                    }
                    processList[i].active = false;
                    time_t currentTime;    
                    processList[i].endTime = currentTime;

                    processList[i].elapsedTime = difftime(processList[i].endTime,processList[i].startTime);
                }else{
                    if(write(fd[1], "process not killed as it doesnt exist",strlen("process not killed as it doesnt exist"))< 0)
                        perror("Error while piping 15. ");
                }
            }else {
                write(fd[1], "failed in killing process",25);
            }
        }
        else if (token == "exit")
        {
              exit(getpid());
              sleep(1);
        }
        else if (token == "list" || token == "listall")
        {
                       
              if (token == "listall"){
                  int i = 0;
                  char temp [500] = {};
                  char tempall [500] = {};
               
                  while(processList[i].pid!= 0 ){
                      if(!processList[i].active){
                            tm st = *localtime(&processList[i].startTime);
                            tm et = *localtime(&processList[i].endTime);

                            int st_h = st.tm_hour;
                            int st_m = st.tm_min;
                            int st_s = st.tm_sec;

                            int ed_h = et.tm_hour;
                            int ed_m = et.tm_min;
                            int ed_s = et.tm_sec;

                            int el_h = ed_h - st_h;
                            int el_m = ed_m - st_m;
                            int el_s = ed_s - st_s;

                            if (el_h < 0 ){
                                el_h = -1 * el_h;
                            }else if (el_m < 0){
                                el_m = -1 * el_m;
                            }else if (el_s < 0){
                                el_s = -1 * el_s;
                            }
                           
                            sprintf(temp, "pid: %d \nstarting time: %d hour %d min %d sec \n ending time: %d hour %d min %d sec \n elapsed time: %d hour %d min %d sec\n\n",
                             processList[i].pid, st_h,st_m,st_s,ed_h,ed_m,ed_s,el_h,el_m,el_s);
                            strcat(tempall,temp);
                            
                            
                      }else{
                          tm st = *localtime(&processList[i].startTime);
                                                                        
                          int st_h = st.tm_hour;
                          int st_m = st.tm_min;
                          int st_s = st.tm_sec;
                           
                          sprintf(temp, "pid: %d \nstarting time: %d hour %d min %d sec \n\n ",
                          processList[i].pid, st_h,st_m,st_s);
                            
                          strcat(tempall,temp);  
                      }
                    
                      i++;
                  }

                  write(fd[1],tempall,strlen(tempall));
              }
              else{
                  int i = 0;
                  char temp [500], tempall[500] = {};
                  while(processList[i].pid!= 0){
                      if(processList[i].active){
                          tm st = *localtime(&processList[i].startTime);
                                                                        
                          int st_h = st.tm_hour;
                          int st_m = st.tm_min;
                          int st_s = st.tm_sec;
                           
                          sprintf(temp, "pid: %d  \nstarting time: %d hour %d min %d sec \n\n ",
                          processList[i].pid, st_h,st_m,st_s);
                            
                          strcat(tempall,temp);  
                      }
                      i++;
                  }
                  if(strlen(tempall) <= 0){write(fd[1],"No active processes",strlen("No active processes"));}else {write(fd[1],tempall,strlen(tempall));}  
                  
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