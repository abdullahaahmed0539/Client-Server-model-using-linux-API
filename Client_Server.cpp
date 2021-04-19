#include <unistd.h>
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

/*
    things to do:
    3)Add pipe close
    4)remove sleep
    5)bug in listall---->segmentation fault new bug occured too
    7)check if exit can happen from server only
    8) when listall works it doesnt take the next input correctly

*/

struct listProcess {
    int processId;
    string processName;
    time_t startTime;
    time_t endTime;
    string elapsedTime;
    bool active;
};

bool characterIsNumerical (char character){
    if(character >= '0' && character <= '9' ){
        return true;
    }
    return false;
}

bool IsSpecialAllowedCharacter(char character){
    if(character == ' ' || character == '\n' || character == ';' ||  character== '.'){
        return true;
    }
    return false;
}

bool characterIsAllowed (char character){
    if(characterIsNumerical(character) || IsSpecialAllowedCharacter(character)){
        return true;
    }
    return false;
}

bool isListNumerical(char numberList []){
    bool isNumericalList;
    for( int index = 0 ; index < strlen(numberList); index++ ){   
        if(characterIsAllowed(numberList[index])) {  
           continue;
        }
        else{ 
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

bool instructionIsToExit(char instruction [], string instructionFromServer){
    if((instruction[0]=='e' && instruction[1]=='x' && instruction[2]=='i' && instruction[3]=='t') || instructionFromServer=="exit"){
        return true;
    }
    return false;
}

bool instructionIsForArithmeticOperations (string instruction){
    if((instruction == "add") || (instruction == "sub") || (instruction == "mul") || (instruction == "div")){
        return true;
    }
    return false;
} 

bool instructionIsToRun(string instruction){
    if(instruction == "run"){
        return true;
    }
    return false;
}

bool instructionIsToKillProcess(string instruction){
    if(instruction == "kill"){
        return true;
    }
    return false;
}

bool instructionIsToDisplayList(string instruction){
    if(instruction == "listall" || instruction == "list"){
        return true;
    }
    return false;
}

bool numberListHasNotEnded (char * instructionTokens){
    if(instructionTokens==NULL){
        return false;
    }
    return true;
}

bool isParentProcess(pid_t processId){
    if (processId > 0){
        return true;
    }
    return false;
}

int emptyIndexFinder(listProcess processList[], int listSize){
    int listIterator = 0;
    while(listIterator < listSize){
        if(processList[listIterator].processId != 0){
            listIterator++;
        }
        else{
            break;
        }
    }
    return listIterator;
}

int indexFinderByComparingProcessId(listProcess processList[], int requiredProcessId,int listSize){
    int listIterator = 0;
    bool foundProcess;
    while(listIterator < listSize){
        if(processList[listIterator].processId == requiredProcessId){
            foundProcess = true;
            break;
        }

        listIterator++;
    }
    
    if(!foundProcess){
        listIterator*=-1;
    }
    return listIterator;
}

int indexFinderByComparingNames(listProcess processList[], string processName, int listSize){
    int listIterator = 0;
    bool foundProcess;
    while(listIterator < listSize){
        if(processList[listIterator].processName == processName && processList[listIterator].active){
            foundProcess = true;
            break;
        }
        else{
            listIterator++;
        }
    }
    if(!foundProcess){
        listIterator*=-1;
    }
    return listIterator;
}

bool isChildProcess(pid_t processId){
    if (processId == 0){
        return true;
    }
    return false;
}

bool processIdIsGiven(int returnValueOfAtoiFunction){
    if(returnValueOfAtoiFunction > 0){
        return true;
    }
    return false;
}   

bool processNameIsGiven(int returnValueOfAtoiFunction){
    if(returnValueOfAtoiFunction == 0){
        return true;
    }
    return false;
} 

// void handler(int sig){
     
// }

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



    else if (isChildProcess(ChildId)){  
        bool keepRunning = true;
        while(keepRunning){
            char instruction[bufferSize], response[bufferSize] = {};
            int ret;
            char outputMessageForInstruction [] = "You have the following commands: add, sub, mul, div, run, kill, list, listall, exit.\n" ;
            char outputMessageForSyntax [] = "For arithmetic operations syntax example: add 1 2 ; . Add <space> ; in every command.\n";
            char outputMessageForInput [] = "Enter your instruction: ";

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
            if(instructionIsToExit(instruction,"")){
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
        int ret;
        bool keepRunning = true;


        /*  
            initialize process IDs to 0 since no process will
            have id = 0
        */
        for (size_t index = 0; index < listSize; index++){
             processList[index].processId = 0;
        }
        

        while(keepRunning){
            char recievedCommand [bufferSize] = {};     
            char * instructionTokens;
            string instruction;

        
            ret = read(pipeBetweenClientAndServer[0], recievedCommand, bufferSize);
            if(ret < 0){
                perror("Error while reading from pipe b/w client & server.");
            }

            instructionTokens = tokenizer(recievedCommand);
            instruction = (string) instructionTokens;
            
            
            
            
            
            
            
            
            
            
            if(instructionIsForArithmeticOperations(instruction)){   
                char buffer [bufferSize]; 
                double answer = 0;
                bool firstNumberFromTheNumberList = true;
                instructionTokens = strtok(NULL," ");

                while(numberListHasNotEnded(instructionTokens)){

                    if(!isListNumerical(instructionTokens)){
                        ret = write(pipeBetweenClientAndServer [1], "List is not numerical.\n", strlen("List is not numerical.\n"));
                        if(ret < 0){
                            perror("Error in list. ");
                        }
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
                            if (*instructionTokens != '0' || firstNumberFromTheNumberList){
                                if (firstNumberFromTheNumberList){
                                    answer = atof(instructionTokens);
                                }else{
                                    answer = answer / atof(instructionTokens);
                                }    
                                firstNumberFromTheNumberList = false;   
                            }
                            else{
                                ret = write(pipeBetweenClientAndServer [1],"Can't divide by 0\n", strlen("Can't divide by 0\n"));
                                if(ret < 0){
                                    perror("Error message 8. ");
                                 }
                                break;
                            } 
                        }
                            
                            instructionTokens = strtok(NULL, " ");
                        } 

                }
                sleep(1);
            }







            else if (instructionIsToRun(instruction)){   
                char buffer [bufferSize];
                int pipeBetweenServerAndExecProcess[2];
                if (pipe2(pipeBetweenServerAndExecProcess, O_CLOEXEC) < 0){
                    perror("Error in piping for exec ");
                }
                
              //  signal (SIGCHLD, handler);
                int pId = fork();
                if(pId < 0){
                    perror("error while forking in run. ");
                }
                else if (isParentProcess(pId)){   
                    
                    instructionTokens = strtok(NULL, " \n");
                    if(write(pipeBetweenServerAndExecProcess[1], instructionTokens, strlen(instructionTokens)) < 0){
                        perror("Error while writing on execpipe. ");
                    }

                    sleep(1);
                  //  waitpid(pId, NULL, WNOHANG);
                    close(pipeBetweenServerAndExecProcess[1]);
                    ret = read(pipeBetweenServerAndExecProcess[0], buffer, bufferSize);
                    if(ret == 0){
                        int listIterator = emptyIndexFinder(processList, listSize);
                        processList[listIterator].processId = pId;
                        char processName [strlen(instructionTokens)];
                        sprintf(processName, "%s", instructionTokens);
                        processList[listIterator].processName = processName;
                        time_t currentTime;    
                        time(&currentTime);
                        processList[listIterator].startTime = currentTime;
                        processList[listIterator].active = true;


                        if(write(pipeBetweenClientAndServer[1], "Success\n", strlen("success\n")) < 0){
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

                    if(write(pipeBetweenServerAndExecProcess[1], "Failed.\n", strlen("Failed.\n"))< 0){
                        perror("Error while piping message. ");
                    }
                }
            }








            else if (instructionIsToKillProcess(instruction)){
                int status, processListIterator;
                instructionTokens = strtok(NULL, " \n");
                int processId = atoi (instructionTokens);
                bool processFound = false;
                
                if (processIdIsGiven(processId)){
                    processListIterator = indexFinderByComparingProcessId(processList, processId, listSize);
                    if(processListIterator >= 0){
                        processFound = true;
                    }
                    
                    if (processFound){
                        ret = kill(processId, SIGTERM);
                        if (ret < 0){
                            if(write(pipeBetweenClientAndServer[1], "Problem in killing Process.\n",strlen("Problem in killing Process.\n")) < 0){
                                perror("Error while piping. ");
                            }
                        }
                        else{
                            write(pipeBetweenClientAndServer[1], "Successfully killed.\n",strlen("successfully killed.\n"));                 
                        }

                        status = 0; 
                        int waitCheck = waitpid(processId, &status, 0);
                        if(waitCheck==-1){
                            perror("Error in waitpid");
                        }

                        processList[processListIterator].active = false;
                        time_t currentTime; 
                        time(&currentTime);   
                        processList[processListIterator].endTime = currentTime;

                        processList[processListIterator].elapsedTime = difftime(processList[processListIterator].endTime,processList[processListIterator].startTime);
                    }
                    else{
                            write(pipeBetweenClientAndServer[1], "Unsuccessful kill. Process not found.\n",strlen("unsuccessful kill. Process not found.\n"));
                    }

                }
                else if (processNameIsGiven(processId)){
                    
                    processFound = false;
                    string processName  = (string) instructionTokens;
                    processListIterator = indexFinderByComparingNames(processList, processName, listSize);
                    if(processListIterator >= 0){
                        processFound =true;
                    }

                    if(processFound){
                        ret = kill(processList[processListIterator].processId, SIGTERM);
                        if (ret < 0){
                            if(write(pipeBetweenClientAndServer[1], "Process not killed.\n",strlen("process not killed.\n")) < 0){
                                perror("Error while killing. ");
                            }
                        }else{
                            if(write(pipeBetweenClientAndServer[1], "Successfully killed.\n",strlen("successfully killed.\n")) < 0){
                                perror("Error while killing. ");
                            };                 
                        }

                        processList[processListIterator].active = false;
                        time_t currentTime;    
                        time(&currentTime);
                        processList[processListIterator].endTime = currentTime;
                        processList[processListIterator].elapsedTime = difftime(processList[processListIterator].endTime,processList[processListIterator].startTime);
                    }
                    else{
                        if(write(pipeBetweenClientAndServer[1], "Process not killed as it doesnt exist.\n",strlen("process not killed as it doesnt exist.\n")) < 0)
                            perror("Error while piping. ");
                    }
                }
                else {
                    write(pipeBetweenClientAndServer[1], "Failed in killing process.\n",strlen("failed in killing process.\n"));
                }
                sleep(1);
            }









            else if (instructionIsToExit(instructionTokens ,instruction)){
                exit(getpid());
                sleep(1);
            }






            else if (instructionIsToDisplayList(instruction)){
                int processListIterator;        
                if (instruction == "listall"){
                    processListIterator = 0;
                    char temperoryList[bufferSize*10], List[bufferSize*10] = {};
                    tm startTime, endTime;
                    
                    while(processList[processListIterator].processId!= 0 ){
                        if(!processList[processListIterator].active){
                            startTime = *localtime(&processList[processListIterator].startTime);
                            endTime = *localtime(&processList[processListIterator].endTime);

                            int startTimeHour = startTime.tm_hour;
                            int startTimeMinute = startTime.tm_min;
                            int startTimeSecond = startTime.tm_sec;

                            int endTimeHour = endTime.tm_hour;
                            int endTimeMinute = endTime.tm_min;
                            int endTimeSecond = endTime.tm_sec;

                            time_t s = difftime(processList[processListIterator].endTime , processList[processListIterator].startTime);
                            tm elapsedTime;

                            elapsedTime = *localtime(&s);
                            int elapsedHour = elapsedTime.tm_hour;
                            int elapsedMinute = elapsedTime.tm_min;
                            int elapsedSecond = elapsedTime.tm_sec;
                            
                            sprintf(temperoryList, "\n*********************Process*********************\nProcess id: %d \nStarting time: %d hour %d min %d sec \nEnding time: %d hour %d min %d sec \nElapsed time: %d hour %d min %d sec\n******************************************\n",
                            processList[processListIterator].processId, startTimeHour,startTimeMinute,startTimeSecond,endTimeHour,endTimeMinute,endTimeSecond,elapsedHour,elapsedMinute,elapsedSecond);
                            strcat(List,temperoryList);
                                    
                        }
                        else{
                            tm startTime = *localtime(&processList[processListIterator].startTime);    
                            int startTimeHour = startTime.tm_hour;
                            int startTimeMinute = startTime.tm_min;
                            int startTimeSecond = startTime.tm_sec;
                            

                            sprintf(temperoryList, "\n\n*************Process**************\nProcess id: %d  \nStarting time: %d hour %d min %d sec. \n*******************************************\n ",
                            processList[processListIterator].processId, startTimeHour,startTimeMinute,startTimeSecond);


                            strcat(List,temperoryList);    //err in this. doesnt not concat the second time


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
                        if(processList[processListIterator].active){
                            tm startTime = *localtime(&processList[processListIterator].startTime);                                               
                            int startTimeHour = startTime.tm_hour;
                            int startTimeMinute = startTime.tm_min;
                            int startTimeSecond = startTime.tm_sec;
                            
                            sprintf(temperoryList, "\n*************Active Process**************\nProcess id: %d  \nStarting time: %d hour %d min %d sec. \n*******************************************\n ",
                            processList[processListIterator].processId, startTimeHour,startTimeMinute,startTimeSecond);
                                
                            strcat(List,temperoryList);  
                        }
                        processListIterator++;
                    }
                    if(strlen(List) <= 0){
                        if(write(pipeBetweenClientAndServer[1],"No active processes.\n",strlen("No active processes.\n")) < 0){
                            perror("Error message 11. ");
                        }
                    }
                    else{
                        if(write(pipeBetweenClientAndServer[1],List,strlen(List)) < 0){
                            perror("Error message 12. ");
                        }
                    }    
                }
                sleep(1);
            }





            else{
                if(write(pipeBetweenClientAndServer[1], "Invalid instruction.\n", strlen("invalid  instruction.\n")) < 0){
                    perror("Error while piping message. ");
                }
            }
            sleep(1);
        }
    }
    return 0;
}