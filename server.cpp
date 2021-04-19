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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
using namespace std;



/*1)error in run command 
  2)check kill
  3)check lists

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



bool anyChildKilled;
void handler(int sig){
     if(sig == SIGCHLD){
         anyChildKilled = true;
     }
}

int main(void){
    signal(SIGCHLD, handler);
	int sock, length;
	struct sockaddr_in server;
	int msgsock;
	char buf[1024];
	int rval;
	int i;

	/* Create socket */
	sock = socket(AF_INET, SOCK_STREAM , 0);
	if (sock < 0) {
		perror("opening stream socket");
		exit(1);
	}
	/* Name socket using wildcards */
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = 0;
	if (bind(sock, (struct sockaddr *) &server, sizeof(server))) {
		perror("binding stream socket");
		exit(1);
	}
	/* Find out assigned port number and print it out */
	length = sizeof(server);
	if (getsockname(sock, (struct sockaddr *) &server, (socklen_t*) &length)) {
		perror("getting socket name");
		exit(1);
	}
	printf("Socket has port #%d\n", ntohs(server.sin_port));
	fflush(stdout);

	/* Start accepting connections */
	listen(sock, 5);
	do {
		msgsock = accept(sock, 0, 0);
		if (msgsock == -1)
			perror("accept");
		
        
        //Server process
        int listSize, bufferSize = 100;
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

            int childIdofKilledProcess = waitpid(-1, NULL, WNOHANG);
            if(anyChildKilled){
                anyChildKilled = false;
                bool processFound;
                int processListIterator = indexFinderByComparingProcessId(processList, childIdofKilledProcess, listSize);
                    if(processListIterator >= 0){
                         processFound = true;
                    }
                    
                    if (processFound){

    

                        processList[processListIterator].active = false;
                        time_t currentTime; 
                        time(&currentTime);   
                        processList[processListIterator].endTime = currentTime;

                        processList[processListIterator].elapsedTime = difftime(processList[processListIterator].endTime,processList[processListIterator].startTime);
                    
                    }
            }


            ret = read(msgsock, recievedCommand, bufferSize);
            if(ret < 0){
                perror("Error while reading from pipe b/w client & server.");
            }

            write(1, "hello mohammad",strlen("hello mohammad"));

            instructionTokens = tokenizer(recievedCommand);
            instruction = (string) instructionTokens;
            
            
            
            
            
            
            
            
            
            
            if(instructionIsForArithmeticOperations(instruction)){   
                char buffer [bufferSize]; 
                double answer = 0;
                bool firstNumberFromTheNumberList = true;
                instructionTokens = strtok(NULL," ");

                while(numberListHasNotEnded(instructionTokens)){

                    if(!isListNumerical(instructionTokens)){
                        ret = write(msgsock, "List is not numerical.\n", strlen("List is not numerical.\n"));
                        if(ret < 0){
                            perror("Error in list. ");
                        }
                        break;
                    }

                    if (*instructionTokens == ';'){
                        int sprinfReturn = sprintf(buffer, "ANSWER: %.2f\n", answer);
                        ret = write(msgsock,buffer,sprinfReturn);
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
                                ret = write(msgsock ,"Can't divide by 0\n", strlen("Can't divide by 0\n"));
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
                // // int pipeBetweenServerAndExecProcess[2];
                // if (pipe2(pipeBetweenServerAndExecProcess, O_CLOEXEC) < 0){
                //     perror("Error in piping for exec ");
                // }
                
                int pId = fork();
                if(pId < 0){
                    perror("error while forking in run. ");
                }
                else if (isParentProcess(pId)){   
                    
                    instructionTokens = strtok(NULL, " \n");
                    if(write(msgsock, instructionTokens, strlen(instructionTokens)) < 0){
                        perror("Error while writing on execpipe. ");
                    }

                    sleep(1);
                    close(msgsock);
                    ret = read(msgsock, buffer, bufferSize);
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


                        if(write(msgsock, "Success\n", strlen("success\n")) < 0){
                            perror("Error message 10. ");
                        }
                    }
                    else{
                        if(write(msgsock, buffer, strlen(buffer)) < 0){
                            perror("Error while piping message. ");
                        }
                    }
                }
                
                
                else{
                    char application [bufferSize];
                    char path[bufferSize] = {'/','u','s','r','/','b','i','n','/'};
                    int ret = read(msgsock, application, bufferSize);

                    close(msgsock);

                    if(ret < 0){
                        perror("Error while reading filename. ");
                    }

                    for (int index = 0; index < strlen(application); index++){
                        path[9 + index] = application [index];
                    }                

                    if(execlp(path,path,NULL) < 0){
                        perror("Error while exec()");
                    }

                    if(write(msgsock, "Failed.\n", strlen("Failed.\n"))< 0){
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
                            if(write(msgsock, "Problem in killing Process.\n",strlen("Problem in killing Process.\n")) < 0){
                                perror("Error while piping. ");
                            }
                        }
                        else{
                            write(msgsock, "Successfully killed.\n",strlen("successfully killed.\n"));                 
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
                            write(msgsock, "Unsuccessful kill. Process not found.\n",strlen("unsuccessful kill. Process not found.\n"));
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
                            if(write(msgsock, "Process not killed.\n",strlen("process not killed.\n")) < 0){
                                perror("Error while killing. ");
                            }
                        }else{
                            if(write(msgsock, "Successfully killed.\n",strlen("successfully killed.\n")) < 0){
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
                        if(write(msgsock, "Process not killed as it doesnt exist.\n",strlen("process not killed as it doesnt exist.\n")) < 0)
                            perror("Error while piping. ");
                    }
                }
                else {
                    write(msgsock, "Failed in killing process.\n",strlen("failed in killing process.\n"));
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

                        
                            int elapsedHour = endTimeHour - startTimeHour;
                            if(elapsedHour<0){elapsedHour*=-1;}
                            int elapsedMinute = endTimeMinute - startTimeMinute;
                            if(elapsedMinute<0){elapsedMinute*=-1;}
                            int elapsedSecond = endTimeSecond - startTimeSecond;
                            if(elapsedSecond<0){elapsedSecond*=-1;}
                            
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


                            strcat(List,temperoryList);    


                        }
                        processListIterator++;
                    }
                    if(strlen(List) <= 0){
                        if(write(msgsock,"No processes.\n",strlen("No active processes.\n")) < 0){
                            perror("Error message 11. ");
                        }
                    }
                    else if(write(msgsock,List,strlen(List)) < 0){
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
                        if(write(msgsock,"No active processes.\n",strlen("No active processes.\n")) < 0){
                            perror("Error message 11. ");
                        }
                    }
                    else{
                        if(write(msgsock,List,strlen(List)) < 0){
                            perror("Error message 12. ");
                        }
                    }    
                }
                sleep(1);
            }





            else{
                if(write(msgsock, "Invalid instruction.\n", strlen("invalid  instruction.\n")) < 0){
                    perror("Error while piping message. ");
                }
            }
            sleep(1);
        }
    






		close(msgsock);
	} while (true);
	/*
	 * Since this program has an infinite loop, the socket "sock" is
	 * never explicitly closed.  However, all sockets will be closed
	 * automatically when a process is killed or terminates normally. 
	 */
}
