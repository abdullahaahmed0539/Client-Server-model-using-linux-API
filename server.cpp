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
#include <pthread.h>
using namespace std;



/*





  4) rest of the things
  6) if ch kills client should be notified
*/

struct listProcess {
    int processId;
    string processName;
    time_t startTime;
    time_t endTime;
    time_t elapsedTime;
    bool active;
};
struct activeList {
    int clientId;
    string list;
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

bool instructionIsToExit(string instructionFromServer){
    if(instructionFromServer=="exit"){
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

bool instructionIsToShowOnlyOneClientList (string instruction){
    if(instruction == "List"){
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

int assignClientID(activeList active[], int listSize){
    int listIterator = 0;
    while(listIterator < listSize){
        if(active[listIterator].clientId != -1){
            listIterator++;
        }
        else{
            active[listIterator].clientId = listIterator;
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

void checkWriteError(int number){
    if (number < 0)
        perror("Error while writing");
}

void checkReadError(int number){
    if (number < 0)
        perror("Error while Reading");
}

void createPipe(int p []){
    if(pipe(p) < 0){
        perror("Error in pipe");
    }
}



//GLOBAL DECLARATIONS
const int listSize = 20;
listProcess processList [listSize];
int clientHandlerRead[2];
activeList active [10];


void handler(int sig){
    if(sig == SIGCHLD){
        int childIdofKilledProcess = waitpid(-1, NULL, WNOHANG);
        bool processFound;
        int processListIterator = indexFinderByComparingProcessId(processList, childIdofKilledProcess, 20);
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
}





void *clientHandler(void * arg){
    int msgsock = *(int *)arg;
    int clientHandlerWrite [2];
    createPipe(clientHandlerWrite);
    int clientId = assignClientID(active,10);
    int clientHandlerId = fork();
    if (isChildProcess(clientHandlerId)){
        close(clientHandlerRead[1]);
        close(clientHandlerWrite[0]);

        char superUserCommand [100]={};
        const int bufferSize = 50;
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

            ret = read(msgsock, recievedCommand, bufferSize);
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
                        checkWriteError(write(msgsock, "List is not numerical.\n", strlen("List is not numerical.\n")));
                        break;
                    }

                    if (*instructionTokens == '\n'){
                        int sprinfReturn = sprintf(buffer, "Answer: %.2f\n", answer);
                        checkWriteError(write(msgsock,buffer,sprinfReturn));
                        instructionTokens = strtok(NULL, " ");
                        answer = 0;
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
                                checkWriteError(write(msgsock ,"Can't divide by 0\n", strlen("Can't divide by 0\n")));
                                break;
                            } 
                        }
                            instructionTokens = strtok(NULL, " ");
                    } 
                }
                sleep(1);
            }

            else if (instructionIsToRun(instruction)){   
                char buffer [bufferSize] = {};
                int clientHandlerWrite[2];
                int execProcessWrite [2];
                if (pipe2(clientHandlerWrite, O_CLOEXEC) < 0){
                    perror("Error in piping for exec ");
                }
                if (pipe2(execProcessWrite, O_CLOEXEC) < 0){
                    perror("Error in piping for exec ");
                }

                int pId = fork();
                if(pId < 0){
                    perror("error while forking in run. ");
                }
                else if (isParentProcess(pId)){   
                    
                    close(clientHandlerWrite[0]);
                    close(execProcessWrite[1]);
                    instructionTokens = strtok(NULL, " \n");
                    checkWriteError(write(clientHandlerWrite[1], instructionTokens, strlen(instructionTokens)));
                    close(clientHandlerWrite[1]);
                    ret = read(execProcessWrite[0], buffer, bufferSize);
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

                        checkWriteError(write(msgsock, "Opening Status: SUCCESS\n", strlen("Opening Status: Success\n")));
                    }
                    else{
                        checkWriteError(write(msgsock, buffer, strlen(buffer)));
                    }
                    close(execProcessWrite[0]);
                }
                
                
                else{
                    char temp [bufferSize];
                    close(clientHandlerWrite[1]);
                    close(execProcessWrite[0]);
                    int ret = read(clientHandlerWrite[0], temp, bufferSize);
                    if(ret < 0){
                        perror("Error while reading filename. ");
                    }
                    char application[ret];
                    for (size_t i = 0; i < ret; i++)
                    {
                        application[i] = temp[i];
                    }
                    application[ret] = '\0';
                    
                    if(execlp(application, application, NULL) < 0){
                        perror("Error while exec()");
                     }

                    checkWriteError(write(execProcessWrite[1], "Opening Status: FAILED.\n", strlen("Opening Status: FAILED.\n")));
                    close(clientHandlerWrite[0]);
                    close(execProcessWrite[1]);
                }
                
            }

            else if (instructionIsToKillProcess(instruction)){
                int status, processListIterator;
                instructionTokens = strtok(NULL, " \n");
                int processId = atoi (instructionTokens);
                bool processFound = false;
                
                if (processIdIsGiven(processId)){
                    processListIterator = indexFinderByComparingProcessId(processList, processId, 20);
                    if(processListIterator >= 0){
                        processFound = true;
                    }
                    
                    if (processFound){
                        ret = kill(processId, SIGTERM);
                        if (ret < 0){
                            checkWriteError(write(msgsock, "Problem in killing Process.\n",strlen("Problem in killing Process.\n")));
                        }
                        else{
                            checkWriteError(write(msgsock, "Successfully killed.\n",strlen("successfully killed.\n")));                 
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
                        checkWriteError(write(msgsock, "Unsuccessful kill. Process not found.\n",strlen("unsuccessful kill. Process not found.\n")));
                    }

                }
                else if (processNameIsGiven(processId)){
                    
                    processFound = false;
                    string processName  = (string) instructionTokens;
                   
                    
                    processListIterator = indexFinderByComparingNames(processList, processName, 20);
                    if(processListIterator >= 0){
                        processFound =true;
                    }

                    if(processFound){
                        ret = kill(processList[processListIterator].processId, SIGTERM);
                        if (ret < 0){
                            checkWriteError(write(msgsock, "Process not killed.\n",strlen("process not killed.\n")));
                        }else{
                            checkWriteError(write(msgsock, "Successfully killed.\n",strlen("successfully killed.\n")));
                        }

                        processList[processListIterator].active = false;
                        time_t currentTime;    
                        time(&currentTime);
                        processList[processListIterator].endTime = currentTime;
                        processList[processListIterator].elapsedTime = difftime(processList[processListIterator].endTime,processList[processListIterator].startTime);
                    }
                    else{
                        checkWriteError(write(msgsock, "Process not killed as it doesnt exist.\n",strlen("process not killed as it doesnt exist.\n")));
                    }
                }
                else {
                    write(msgsock, "Failed in killing process.\n",strlen("failed in killing process.\n"));
                }
                sleep(1);
            }

            else if (instructionIsToExit(instruction)){
                exit(EXIT_SUCCESS);
            }

            else if (instructionIsToDisplayList(instruction)){
                int processListIterator;      
                int size = bufferSize*bufferSize;
                instructionTokens = strtok(NULL, " /n");
                string param =   (string) instructionTokens;

                if (param.compare("all\n") == 0){
                    processListIterator = 0;
                    char temperoryList[size], List[size] = {};
                    tm startTime, endTime, elTime;
                    
                    while(processList[processListIterator].processId!= 0 ){
                        if(!processList[processListIterator].active){
                            startTime = *localtime(&processList[processListIterator].startTime);
                            endTime = *localtime(&processList[processListIterator].endTime);
                            elTime = *localtime(&processList[processListIterator].elapsedTime);

                            int startTimeHour = startTime.tm_hour;
                            int startTimeMinute = startTime.tm_min;
                            int startTimeSecond = startTime.tm_sec;

                            int endTimeHour = endTime.tm_hour;
                            int endTimeMinute = endTime.tm_min;
                            int endTimeSecond = endTime.tm_sec;

                            int elHour = elTime.tm_hour;
                            int elMinute = elTime.tm_min;
                            int elSecond = elTime.tm_sec;

                            
                            sprintf(temperoryList, "\n*********************Process*********************\nProcess id: %d \nProcess name: %s \nStarting time: %d:%d:%d  \nEnding time: %d:%d:%d  \nElapsed time: %d min %d sec\n*************************************************\n",
                            processList[processListIterator].processId, processList[processListIterator].processName.c_str(), startTimeHour,startTimeMinute,startTimeSecond,endTimeHour,endTimeMinute,endTimeSecond, elMinute,elSecond);
                            strcat(List,temperoryList);
                                    
                        }
                        else{
                            tm startTime = *localtime(&processList[processListIterator].startTime); 
                            int startTimeHour = startTime.tm_hour;
                            int startTimeMinute = startTime.tm_min;
                            int startTimeSecond = startTime.tm_sec;
                            

                            sprintf(temperoryList, "\n\n*************Process*******************\nProcess id: %d  \nProcess name: %s \nStarting time: %d:%d:%d. \n************************************\n ",
                            processList[processListIterator].processId, processList[processListIterator].processName.c_str(),startTimeHour,startTimeMinute,startTimeSecond);


                            strcat(List,temperoryList);    


                        }
                        processListIterator++;
                    }
                    if(strlen(List) <= 0){
                        checkWriteError(write(msgsock,"No active processes.\n",strlen("No active processes.\n")));
                    }
                    else {
                        checkWriteError(write(msgsock,List,strlen(List)));
                    }
                }
                else if (param.compare("active\n")==0){
                    processListIterator = 0;
                    char temperoryList [size], List[size] = {};
                    while(processList[processListIterator].processId!= 0){
                        if(processList[processListIterator].active){
                            tm startTime = *localtime(&processList[processListIterator].startTime);                                               
                            int startTimeHour = startTime.tm_hour;
                            int startTimeMinute = startTime.tm_min;
                            int startTimeSecond = startTime.tm_sec;
                            
                            sprintf(temperoryList, "\n*************Active Process********************\nProcess id: %d  \nProcess name: %s \nStarting time: %d:%d:%d. \n***********************************\n ",
                            processList[processListIterator].processId, processList[processListIterator].processName.c_str(), startTimeHour,startTimeMinute,startTimeSecond);
                                
                            strcat(List,temperoryList);  
                        }
                        processListIterator++;
                    }
                    checkWriteError(write(clientHandlerWrite[1], &clientId,sizeof(clientId)));
                    if(strlen(List) <= 0){
                        checkWriteError(write(clientHandlerWrite[1], "No processes.\n",strlen("No active processes.\n")));
                        checkWriteError(write(msgsock,"No active processes.\n",strlen("No active processes.\n")));
                    }
                    else{
                        checkWriteError(write(clientHandlerWrite[1],List,strlen(List)));
                        checkWriteError(write(msgsock,List,strlen(List)));
                    }    
                }
                else{
                    checkWriteError(write(msgsock, "Invalid instruction.\n", strlen("invalid  instruction.\n")));
                }
              sleep(1);
            }

            else{
                checkWriteError(write(msgsock, "Invalid instruction.\n", strlen("invalid  instruction.\n")));
            }
            sleep(1);
        }
		close(msgsock);

        }
        else if (isParentProcess(clientHandlerId)){
            close(clientHandlerWrite[1]);
            while(true){
            char temp [1000000] = {};
            int id;
            int ret1 = read(clientHandlerWrite[0], &id, sizeof(int));
            int ret2 = read(clientHandlerWrite[0], temp, sizeof(temp));
            char buff [ret2] = {};
            for (size_t i = 0; i < ret2; i++){
                buff[i] = temp[i];
            }
            buff[ret2] = '\0';
            active[id].list = (string) buff;
            }


        }else{
            perror("Error while forking");
        }


    return NULL;
}

void *superUser(void *arg){
        int superUserWrite[2];
        createPipe(superUserWrite);
        int s_id = fork();
        if (s_id < 0 ){
            perror("Error while forking");
        }
        else if (isChildProcess(s_id)){
            close(superUserWrite[0]);
            while(true){
                char outputMessageForInstruction [] = "You have the following commands: print, print client, list, list client, exit.\n" ;
                char outputMessageForInput [] = "Enter your instruction: ";
                char instr[20]={};
                
                checkWriteError(write(STDOUT_FILENO, &outputMessageForInstruction , strlen(outputMessageForInstruction)));
                checkWriteError(write(STDOUT_FILENO, &outputMessageForInput, strlen(outputMessageForInput)));
                int ret = read(STDIN_FILENO, instr, 20); 
                if(ret < 0){
                    perror("Error message 4. ");
                }
                checkWriteError(write(superUserWrite[1], instr,strlen(instr)));
                sleep(1);
            }
        }
        else{
            close(superUserWrite[1]);
            char instr[20]={};
            while(true){
                int ret = read(superUserWrite[0], instr, 20); 
                if(ret < 0){
                    perror("Error message 4. ");
                }
                char * tok = tokenizer(instr);
                string command = (string) tok;


                if (instructionIsToDisplayList(command)){
                        tok = strtok(NULL, " \n");
                        string command = (string) tok;

                        if(command.compare("all") == 0){
                            int count = 0;
                            while(active[count].clientId != -1){  
                                int n = active[count].list.length();
                                char char_array[n + 1],  id [sizeof(int)];
                                strcpy(char_array, active[count].list.c_str());
                                sprintf(id, "%d", active[count].clientId);
                                checkWriteError(write(STDOUT_FILENO, "\n",1 ));
                                checkWriteError(write(STDOUT_FILENO, "Client ID :", strlen("Client ID :")));
                                checkWriteError(write(STDOUT_FILENO, id, strlen(id)));
                                checkWriteError(write(STDOUT_FILENO, char_array , strlen(char_array)));
                                checkWriteError(write(STDOUT_FILENO, "\n",1 ));
                                count++;
                            }
                        }
                        else{
                            int clientID;
                            bool correctInput = true;
                            if (command.compare("0") == 0){
                                clientID = 0;
                            }else{
                                clientID = atoi(tok);
                                if(clientID == 0){
                                    perror("Error in atoi");
                                    correctInput = false;
                                }
                            }

                            if(active[clientID].clientId == -1){
                                correctInput = false;
                            }

                            if(correctInput){
                                int count = 0;
                                while(count != clientID){
                                    count++;
                                }
                                int n = active[count].list.length();
                                char char_array[n + 1], id [sizeof(int)];
                                sprintf(id, "%d", active[count].clientId);
                                strcpy(char_array, active[count].list.c_str());
                                checkWriteError(write(STDOUT_FILENO, "\n",1 ));
                                checkWriteError(write(STDOUT_FILENO, "Client ID :", strlen("Client ID :")));
                                checkWriteError(write(STDOUT_FILENO, id, strlen(id)));
                                checkWriteError(write(STDOUT_FILENO, char_array , strlen(char_array)));
                                checkWriteError(write(STDOUT_FILENO, "\n",1 ));
                                correctInput = false;
                            }
                            else{
                                checkWriteError(write(STDOUT_FILENO, "Incorrect client ID.\n\n",strlen("Incorrect client ID.\n\n")));
                            }



                        }
                       
                }   
            }
        }
    

 
    return NULL;
}

int main(void){
    signal(SIGCHLD, handler);
    createPipe(clientHandlerRead);

    /*  
            initialize client IDs to 0 since no process will
            have id = 0
        */
    for (size_t index = 0; index < 10; index++){
        active[index].clientId = -1;
    }

    pthread_t superUserThread, clientHandlerThread;
    pthread_create(&superUserThread, NULL, superUser, NULL);
    pthread_detach(superUserThread);
    
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
	server.sin_port = htons(10000);
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
	listen(sock, 10);

	do {
		msgsock = accept(sock, 0, 0);
		if (msgsock == -1)
			perror("accept");
		
        pthread_create(&clientHandlerThread, NULL, clientHandler,(void*)&msgsock);
        pthread_detach(clientHandlerThread);

	} while (true);
}
