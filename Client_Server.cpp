#include <unistd.h>
//#include <error.h>
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



struct listProcess {
    int processId;
    string processName;
    time_t startTime;
    time_t endTime;
    string elapsedTime;
    bool processActive;
};



bool isListNumerical(char numberList []){
    bool isNumericalList;
    for( int index = 0 ; index < strlen(numberList); index++ ){   
        if( (numberList[index] >= '0' && numberList[index] <= '9' ) || numberList[index] == ' ' || numberList[index] == '\n' || numberList[index] == ';' || numberList[index]== '-' || numberList[index]== '.') {  
           continue;
        }
        else{ 
            write(STDOUT_FILENO,"You have entered a non-numeric value. Please enter only numeric values.\n",strlen("You have entered a non-numeric value. Please enter only numeric values.\n"));
            return isNumericalList;
        }
    }
    isNumericalList = true;
    return isNumericalList;
}

char * tokenizer(char numberList []){
    char * listToken = strtok(numberList, " \n");  
    return listToken;
}

int main (){
    int bufferSize = 100;
    int pipeBetweenClientAndServer[2];

    if (pipe(pipeBetweenClientAndServer) < 0){
        perror("Error in creating pipe between client and server.");
    }



    int ChildId = fork();
    if(ChildId < 0){
        perror("Error while forking to create client & server.");
    }
    else if (ChildId == 0){  
        //child process
        bool keepRunning = true;
        while(keepRunning){
            char instruction[bufferSize], response[bufferSize] = {};
            int ret;
            char outputMessageForInstruction [] = "You have the following commands: add, sub, mul, div, run, kill, list, listall, exit.\n" ;
            char outputMessageForSyntax [] = "For arithmetic operations syntax example: add 1 2 ; . Add <space> ; in every command.\n";
            char outputMessageForInput [] = "Enter your instruction: ";
            cout <<endl;

            if(write(STDOUT_FILENO, &outputMessageForInstruction , strlen(outputMessageForInstruction)) < 0){
                perror("Error message 1. ");
            }

            if (write(STDOUT_FILENO,outputMessageForSyntax, strlen(outputMessageForSyntax))< 0){
                perror("Error message 2. ");             
            }

            if(write(STDOUT_FILENO, &outputMessageForInput, strlen(outputMessageForInput)) < 0){
                perror("Error message 3. "); 
            }




            ret = read(STDIN_FILENO, instruction, bufferSize); 
            if(ret < 0){
                perror("Error message 4. ");
            }
            
            


            if(write(pipeBetweenClientAndServer[1], instruction, ret) < 0){
                perror("Error message 5. ");
            }


            sleep(1);


            char * instructionToken = tokenizer(instruction);
            string toktemp = (string) instructionToken;
            if(instruction[0]=='e' && instruction[1]=='x' && instruction[2]=='i' && instruction[3]=='t'){
                exit(1);
            }
            

            ret = read (pipeBetweenClientAndServer[0], response, 500);
            if (ret < 0){
                perror("Error message 6. ");
            }

            if(write(STDOUT_FILENO, response, ret) < 0){
                perror("Error message 7. ");
            }    
            
            write(STDOUT_FILENO,"\n",1);
        }

    }







    else{   
        //Server process
        int listSize = bufferSize;
        listProcess processList [listSize];
        bool keepRunning = true;


        /*  
            initialize process IDs to 0 since no process will
            have id = 0
        */
        for (size_t index = 0; index < listSize; index++){
             processList[index].processId = 0;
        }
        

        while(keepRunning){
            int ret;
            char recievedCommand [bufferSize] = {};     
            char * instructionTokens;
            string instruction;

            write(STDOUT_FILENO,"\n",1);
        
            ret = read(pipeBetweenClientAndServer[0], recievedCommand, bufferSize);
            if(ret < 0){
                perror("Error while reading from pipe b/w client & server.");
            }

            instructionTokens = tokenizer(recievedCommand);
            instruction = (string) instructionTokens;
            
            
            
            
            
            
            
            
            
            
            if((instruction == "add") || (instruction == "sub") || (instruction == "mul") || (instruction == "div")){   
                char buffer [bufferSize]; 
                double answer = 0;
                bool firstNumberFromTheNumberList = true;
                instructionTokens = strtok(NULL," ");

                while(instructionTokens!=NULL){

                    if(!isListNumerical(instructionTokens)){
                        break;
                    }

                    if (*instructionTokens == ';'){
                        int sprinfReturn = sprintf(buffer, "ANSWER: %.2f\n", answer);
                        ret = write(pipeBetweenClientAndServer [1],buffer,sprinfReturn);
                        if(ret < 0){
                            perror("Error message 8. ");
                        }
                        instructionTokens = strtok(NULL, " ");
                        answer = 0;
                    } 
                    else if (*instructionTokens == '\n'){
                        continue;
                    }
                    else{
                        if (instruction == "add" ){
                            answer += atof(instructionTokens); 
                        }
                        else if (instruction =="sub"){
                            if (firstNumberFromTheNumberList){
                                answer=atof(instructionTokens);
                                firstNumberFromTheNumberList= false;
                            }
                            else{
                                answer -= atof(instructionTokens);
                            }    
                        }
                        else if (instruction == "mul"){
                            if (answer == 0){
                                answer = 1;
                            }
                            
                            answer = answer * atof(instructionTokens);
                            }
                            else{
                                if (*instructionTokens != '0'){
                                    if (answer == 0){
                                        answer = atof(instructionTokens);
                                    }else{
                                        answer = answer / atof(instructionTokens);
                                    }       
                                }
                                else{
                                    if(write(STDOUT_FILENO, "Can't divide by 0\n", strlen("Can't divide by 0\n")) < 0){
                                        perror("Error message 9. ");
                                    }
                                    break;
                                } 
                            }
                            
                            instructionTokens = strtok(NULL, " ");
                        } 

                }
                sleep(1);
            }








            else if (instruction == "run"){   
                char buffer [bufferSize];
                int pipeBetweenServerAndExecProcess[2];
                if (pipe2(pipeBetweenServerAndExecProcess, O_CLOEXEC) < 0){
                    perror("Error in piping for exec ");
                }
                
                
                int pId = fork();
                if(pId < 0){
                    perror("error while forking in run. ");
                }
                else if (pId > 0){   
                    
                    instructionTokens = strtok(NULL, " \n");
                    if(write(pipeBetweenServerAndExecProcess[1], instructionTokens, strlen(instructionTokens)) < 0){
                        perror("Error while writing on execpipe. ");
                    }

                    sleep(1);

                    close(pipeBetweenServerAndExecProcess[1]);
                    ret = read(pipeBetweenServerAndExecProcess[0], buffer, bufferSize);
                    if(ret == 0){
                        int listIterator = 0;
                        while(processList[listIterator].processId != 0){
                            listIterator++;
                        }

                        processList[listIterator].processId = pId;
                        char processName [strlen(instructionTokens)];
                        sprintf(processName, "%s", instructionTokens);
                        processList[listIterator].processName = processName;
                        time_t currentTime;    
                        time(&currentTime);
                        processList[listIterator].startTime = currentTime;
                        processList[listIterator].processActive = true;


                        if(write(pipeBetweenClientAndServer[1], "Success", strlen("success")) < 0){
                            perror("Error message 10. ");
                        }
                    }
                    else{
                        if(write(pipeBetweenClientAndServer[1], buffer, strlen(buffer)) < 0){
                            perror("Error while piping message. ");
                        }
                    }
                }
                
                
                else{
                    char application [bufferSize];
                    char path[bufferSize] = {'/','u','s','r','/','b','i','n','/'};
                    int ret = read(pipeBetweenServerAndExecProcess[0], application, bufferSize);

                    close(pipeBetweenServerAndExecProcess[0]);

                    if(ret < 0){
                        perror("Error while reading filename. ");
                    }

                    for (int index = 0; index < strlen(application); index++){
                        path[9 + index] = application [index];
                    }                

                    if(execlp(path,path,NULL) < 0){
                        perror("Error while exec()");
                    }

                    if(write(pipeBetweenServerAndExecProcess[1], "Failed", strlen("Failed"))< 0){
                        perror("Error while piping message. ");
                    }
                }
            }








            else if (instruction == "kill"){
                int status, processListIterator;
                instructionTokens = strtok(NULL, " \n");
                int processId = atoi (instructionTokens);
                
                if (processId > 0){
                    processListIterator = 0;
                    bool pIdFound;
                        
                    while(processList[processListIterator].processId !=0){
                        if(processList[processListIterator].processId==processId){
                            pIdFound = true;
                            break;
                        }  
                        processListIterator++;
                    }
                    if (pIdFound){
                        ret = kill(processId,SIGTERM);
                        if (ret < 0){
                            if(write(pipeBetweenClientAndServer[1], "Process not killed as it doesnt exist.",strlen("process not killed as it doesnt exist.")) < 0){
                                perror("Error while piping. ");
                            }
                        }
                        else{
                            write(pipeBetweenClientAndServer[1], "Successfully killed",strlen("successfully killed"));                 
                        }

                        int waitCheck = waitpid(processId,&status,0);
                        if(waitCheck==-1){
                            perror("Error in waitpid");
                        }

                        processList[processListIterator].processActive = false;
                        time_t currentTime; 
                        time(&currentTime);   
                        processList[processListIterator].endTime = currentTime;

                        processList[processListIterator].elapsedTime = difftime(processList[processListIterator].endTime,processList[processListIterator].startTime);
                    }
                    else{
                            write(pipeBetweenClientAndServer[1], "Unsuccessful kill",strlen("unsuccessful kill"));
                    }
                }else if (processId==0){
                    
                    bool processFound;
                    processListIterator = 0;
                    string processName  = (string) instructionTokens;
                    while(processList[processListIterator].processId!=0){
                        if( processName == processList[processListIterator].processName && processList[processListIterator].processActive){
                            processFound = true;
                            break;
                        }
                        processListIterator++;
                    }

                    if(processFound){
                        ret = kill(processList[processListIterator].processId, SIGTERM);
                        if (ret < 0){
                            if(write(pipeBetweenClientAndServer[1], "Process not killed",strlen("process not killed")) < 0){
                                perror("Error while killing. ");
                            }
                        }else{
                            if(write(pipeBetweenClientAndServer[1], "Successfully killed",strlen("successfully killed")) < 0){
                                perror("Error while killing. ");
                            };                 
                        }

                        processList[processListIterator].processActive = false;
                        time_t currentTime;    
                        processList[processListIterator].endTime = currentTime;
                        processList[processListIterator].elapsedTime = difftime(processList[processListIterator].endTime,processList[processListIterator].startTime);
                    }
                    else{
                        if(write(pipeBetweenClientAndServer[1], "Process not killed as it doesnt exist",strlen("process not killed as it doesnt exist")) < 0)
                            perror("Error while piping. ");
                    }
                }else {
                    write(pipeBetweenClientAndServer[1], "Failed in killing process",strlen("failed in killing process"));
                }
            }









            else if (instruction == "exit"){
                exit(getpid());
                sleep(1);
            }






            else if (instruction == "list" || instruction == "listall"){
                int processListIterator;        
                if (instruction == "listall"){
                    processListIterator = 0;
                    char temperoryList[bufferSize], List[bufferSize] = {};
                    
                    while(processList[processListIterator].processId!= 0 ){
                        if(!processList[processListIterator].processActive){
                            tm startTime = *localtime(&processList[processListIterator].startTime);
                            tm endTime = *localtime(&processList[processListIterator].endTime);

                            int startTimeHour = startTime.tm_hour;
                            int startTimeMinute = startTime.tm_min;
                            int startTimeSecond = startTime.tm_sec;

                            int endTimeHour = endTime.tm_hour;
                            int endTimeMinute = endTime.tm_min;
                            int endTimeSecond = endTime.tm_sec;

                            int elapsedHour = endTimeHour - startTimeHour;
                            int elapsedMinute = endTimeMinute - startTimeMinute;
                            int elapsedSecond = endTimeSecond - startTimeSecond;

                            if (elapsedHour < 0 ){
                                elapsedHour = -1 * elapsedHour;
                            }else if (elapsedMinute < 0){
                                elapsedMinute = -1 * elapsedMinute;
                            }else if (elapsedSecond < 0){
                                elapsedSecond = -1 * elapsedSecond;
                            }
                            
                            sprintf(temperoryList, "\n\nProcess id: %d \nStarting time: %d hour %d min %d sec \n Ending time: %d hour %d min %d sec \n Elapsed time: %d hour %d min %d sec\n\n",
                            processList[processListIterator].processId, startTimeHour,startTimeMinute,startTimeSecond,endTimeHour,endTimeMinute,endTimeSecond,elapsedHour,elapsedMinute,elapsedSecond);
                            strcat(List,temperoryList);
                                    
                        }
                        else{
                            tm startTime = *localtime(&processList[processListIterator].startTime);
                                                                            
                            int startTime_h = startTime.tm_hour;
                            int startTime_m = startTime.tm_min;
                            int startTimeSecond = startTime.tm_sec;
                            
                            sprintf(temperoryList, "\n\nProcess id: %d \nStarting time: %d hour %d min %d sec \n\n ",
                            processList[processListIterator].processId, startTime_h,startTime_m,startTimeSecond);
                                
                            strcat(List,temperoryList);  
                        }
                        
                        processListIterator++;
                    }

                    if(write(pipeBetweenClientAndServer[1],List,strlen(List)) < 0){
                        perror("Error:");
                    }
                }
                else{
                    processListIterator = 0;
                    char temperoryList [500], List[500] = {};
                    while(processList[processListIterator].processId!= 0){
                        if(processList[processListIterator].processActive){
                            tm startTime = *localtime(&processList[processListIterator].startTime);                                               
                            int startTime_h = startTime.tm_hour;
                            int startTime_m = startTime.tm_min;
                            int startTimeSecond = startTime.tm_sec;
                            
                            sprintf(temperoryList, "\n\nProcess id: %d  \nStarting time: %d hour %d min %d sec \n\n ",
                            processList[processListIterator].processId, startTime_h,startTime_m,startTimeSecond);
                                
                            strcat(List,temperoryList);  
                        }
                        processListIterator++;
                    }
                    if(strlen(List) <= 0){
                        if(write(pipeBetweenClientAndServer[1],"No active processes",strlen("No active processes")) < 0){
                            perror("Error message 11. ");
                        }
                    }
                    else{
                        if(write(pipeBetweenClientAndServer[1],List,strlen(List)) < 0){
                            perror("Error message 12. ");
                        }
                    }    
                }
            }









            else{
                if(write(pipeBetweenClientAndServer[1], "Invalid", strlen("invalid")) < 0){
                    perror("Error while piping message. ");
                }
            }
            sleep(1);
        }
    }
    return 0;
}