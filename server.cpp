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
#include <cctype>
using namespace std;

#define LIST_SIZE 20
#define BUFF_SIZE 50
#define SIZE 2500
#define WRITE_ERROR_MESSAGE "Error in write."
#define READ_ERROR_MESSAGE "Error in read."
#define SOCKET_ERROR_MESSAGE "Error in socket."
#define PIPE_ERROR_MESSAGE "Error in pipe."
#define FORK_ERROR_MESSAGE "Error in fork."


//STRUCTS
struct listProcess {
    int processId;
    string processName;
    time_t startTime, endTime, elapsedTime;
    bool active;
};

struct activeList {
    int clientId, socket, pipeWrite;
    string list;
};


//GLOBAL DECLARATIONS
listProcess processList [LIST_SIZE];
activeList active [LIST_SIZE];
int clientId;

bool characterIsNumerical (char character){
    if(character >= '0' && character <= '9' )
        return true;
    return false;
}

bool IsAllowedSpecialCharacter(char character){
    if(character == ' ' || character == '\n'  ||  character== '.')
        return true;
    return false;
}

bool characterIsAllowed (char character){
    if(characterIsNumerical(character) || IsAllowedSpecialCharacter(character))
        return true;
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

char* tokenizer(char numberList []){
    char * listToken = strtok(numberList, " \n");  
    return listToken;
}

bool exitInstruction(string instructionFromServer){
    if(instructionFromServer=="exit")
        return true;
    return false;
}

bool arithmeticInstruction (string instruction){
    if((instruction == "add") || (instruction == "sub") || (instruction == "mul") || (instruction == "div"))
        return true;
    return false;
} 

bool runInstruction(string instruction){
    if(instruction == "run")
        return true;
    return false;
}

bool killInstruction(string instruction){
    if(instruction == "kill")
        return true;
    return false;
}

bool DisplayListInstruction(string instruction){
    if(instruction == "listall" || instruction == "list")
        return true;
    return false;
}

bool printOnClientInstruction(string instruction){
    if(instruction == "print")
        return true;
    return false;
}

bool numberListHasNotEnded (char * instructionTokens){
    if(instructionTokens==NULL)
        return false;
    return true;
}

bool parentProcess(pid_t processId){
    if (processId > 0)
        return true;
    return false;
}

int assignClientID(){
    int listIterator = 0;
    while(listIterator < LIST_SIZE){
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

int emptyIndexFinder(){
    int listIterator = 0;
    bool foundProcess;
    while(listIterator < LIST_SIZE){
        if(processList[listIterator].processId == 0){
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

int indexFinderByComparingProcessId(int requiredProcessId){
    int listIterator = 0;
    bool foundProcess;
    while(listIterator < LIST_SIZE){
        if(processList[listIterator].processId == requiredProcessId && processList[listIterator].active){
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

int indexFinderByComparingNames(string processName){
    int listIterator = 0;
    bool foundProcess;
    while(listIterator < LIST_SIZE){
        if(processList[listIterator].processName == processName && processList[listIterator].active){
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

bool childProcess(pid_t processId){
    if (processId == 0)
        return true;
    return false;
}

bool idIsGiven(int returnValueOfAtoiFunction){
    if(returnValueOfAtoiFunction > 0)
        return true;
    return false;
}   

void checkError(int number, string msg){
    int n = msg.length();
    char message[n + 1],  id [sizeof(int)];
    strcpy(message, msg.c_str());
    if (number < 0){
        perror("Error");
        write(STDOUT_FILENO, message, strlen(message));
    }
}

void createPipe(int p [], int type){
    if (type == 2){
        if (pipe2(p, O_CLOEXEC) < 0)
            perror("Error in piping for exec "); 
    }else {
        if(pipe(p) < 0)
            perror("Error in pipe");
    }
}

void setStartingTime(int i){
    time_t currentTime; 
    time(&currentTime);   
    processList[i].startTime = currentTime;
}

void setEndingTime(int i){
    time_t currentTime; 
    time(&currentTime);   
    processList[i].endTime = currentTime;
    processList[i].elapsedTime = difftime(processList[i].endTime, processList[i].startTime);
}

void activateProcess(int i, bool status){
    processList[i].active = status;
}

void setProcessId(int i, int id){
    processList[i].processId = id;
}

void setProcessName(int i, string name){
    processList[i].processName = name;
}

void initializeProcessList (){
    for (size_t index = 0; index < LIST_SIZE; index++)
        processList[index].processId = 0;
}

void initializeActiveList (){
    for (size_t index = 0; index < LIST_SIZE; index++){
        active[index].clientId = -1;
        active[index].socket = -2;
        active[index].list = "No processes";
    }
}

void closePipeEnds (int end1, int end2){
    close(end1);
    close(end2);
}

void nullTerminate(char* a, char* b, int ret){
    for (size_t i = 0; i < ret; i++){
        a[i] = b[i];
    }
    a[ret] = '\0';
}

void printInstructions(){
    char seperator [] = "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
    char outputMessageForInstruction [] = "You have the following commands: print, print <client id> <message>, list all, list <client id>.\n\n" ;
    char outputMessageForInput [] = "Enter your instruction: ";
    checkError(write(STDOUT_FILENO, &seperator , strlen(seperator)), WRITE_ERROR_MESSAGE);
    checkError(write(STDOUT_FILENO, &outputMessageForInstruction , strlen(outputMessageForInstruction)), WRITE_ERROR_MESSAGE);
    checkError(write(STDOUT_FILENO, &outputMessageForInput, strlen(outputMessageForInput)), WRITE_ERROR_MESSAGE);
}

void printListOnServer(char id [] , char char_array [] ){
    checkError(write(STDOUT_FILENO, "\n", 1 ), WRITE_ERROR_MESSAGE);
    checkError(write(STDOUT_FILENO, "Client ID :", strlen("Client ID :")), WRITE_ERROR_MESSAGE);
    checkError(write(STDOUT_FILENO, id, strlen(id)), WRITE_ERROR_MESSAGE);
    checkError(write(STDOUT_FILENO, "\n",1 ), WRITE_ERROR_MESSAGE);
    if (char_array[0] == '\0'){
        checkError(write(STDOUT_FILENO, "Client not connected any more\n" , strlen("Client not connected any more\n")), WRITE_ERROR_MESSAGE);
    }else{
        checkError(write(STDOUT_FILENO, char_array , strlen(char_array)), WRITE_ERROR_MESSAGE);
    }
    
    checkError(write(STDOUT_FILENO, "\n",1 ), WRITE_ERROR_MESSAGE);
}

void printAnswer(double *answer, char buffer [], int msgsock){
    int sprinfReturn = sprintf(buffer, "Answer = %.2f\n", *answer);
    checkError(write(msgsock,buffer,sprinfReturn), SOCKET_ERROR_MESSAGE);
    *answer = 0;
}

void handleFirstNumberProblem(bool* isFirstNumberFromList, double* answer, double value){
    if (*isFirstNumberFromList){
        *answer  = value;
        *isFirstNumberFromList = false;
    }
}


void maintainListOnServer(int ipc, int ipcType){
    int processListIterator = 0, ret;
    char temperoryList [SIZE], List[SIZE] = {};
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
    if(ipcType == 1){
        checkError(ret = write(ipc, &clientId, sizeof(clientId)), PIPE_ERROR_MESSAGE);
        if(ret < 0){

        }
    }
    if(strlen(List) <= 0){
        checkError(write(ipc, "No active processes.\n",strlen("No active processes.\n")), WRITE_ERROR_MESSAGE);
    }
    else{
        checkError(write(ipc, List, strlen(List)), WRITE_ERROR_MESSAGE);
    } 
}

void handler(int sig){
    bool processFound;
    int childIdofKilledProcess, processListIterator;

    if(sig == SIGCHLD){
        childIdofKilledProcess = waitpid(-1, NULL, WNOHANG);
        processListIterator = indexFinderByComparingProcessId(childIdofKilledProcess);
       
        if(processListIterator >= 0)
            processFound = true;  

        if (processFound){
            activateProcess(processListIterator, false);
            setEndingTime(processListIterator);
            maintainListOnServer(active[clientId].pipeWrite , 1);
        }
    }
}

void *clientHandler(void * arg){
    
    int msgsock = *(int *)arg;
    int clientHandlerWrite [2];
    int clientHandlerId;
    createPipe(clientHandlerWrite, 1);

    clientId = assignClientID();
    active[clientId].socket = msgsock;
    active[clientId].pipeWrite = clientHandlerWrite[1];


    checkError(clientHandlerId = fork(), FORK_ERROR_MESSAGE);
    if (childProcess(clientHandlerId)){
        close(clientHandlerWrite[0]);
        int ret;
        bool keepRunning = true;
        initializeProcessList();
        while(keepRunning){
            char recievedCommand [BUFF_SIZE] = {};     
            string instruction;
            checkError(ret = read(msgsock, recievedCommand, BUFF_SIZE), SOCKET_ERROR_MESSAGE);
            char* instructionTokens = tokenizer(recievedCommand);
            instruction.assign(instructionTokens);
            
            if(arithmeticInstruction(instruction)){   
                char buffer [BUFF_SIZE]; 
                double answer = 0;
                bool firstNumberFromTheNumberList = true;                
                instructionTokens = strtok(NULL," ");

                while(numberListHasNotEnded(instructionTokens)){
                    if(!isListNumerical(instructionTokens)){
                        checkError(write(msgsock, "List is not numerical.\n", strlen("List is not numerical.\n")), SOCKET_ERROR_MESSAGE);
                        break;
                    }
                    if (*instructionTokens == '\n'){
                        instructionTokens = strtok(NULL, " ");
                        printAnswer(&answer, buffer, msgsock);
                    } 
                    else{
                        if (instruction.compare("add") == 0){
                            answer += atof(instructionTokens);
                        }
                        else if (instruction.compare("sub") == 0){
                            if(firstNumberFromTheNumberList){handleFirstNumberProblem(&firstNumberFromTheNumberList, &answer, atof(instructionTokens));}
                            else{answer -= atof(instructionTokens);}   
                        }
                        else if (instruction.compare("mul") == 0){
                            if (answer == 0 && firstNumberFromTheNumberList){handleFirstNumberProblem(&firstNumberFromTheNumberList, &answer, 1);}
                            answer = answer * atof(instructionTokens);
                        }
                        else{
                            if (*instructionTokens != '0' || firstNumberFromTheNumberList){
                                if (firstNumberFromTheNumberList){handleFirstNumberProblem(&firstNumberFromTheNumberList, &answer, atof(instructionTokens));}
                                else{answer = answer / atof(instructionTokens);}    
                            }
                            else{
                                checkError(write(msgsock ,"Can't divide by 0\n", strlen("Can't divide by 0\n")), SOCKET_ERROR_MESSAGE);
                                break;
                            } 
                        }
                        instructionTokens = strtok(NULL, " ");
                    } 
                }
            }

            else if (runInstruction(instruction)){   
                char buffer [BUFF_SIZE] = {};
                int parentWrite[2], execProcessWrite [2], pId;
                createPipe(parentWrite, 2);
                createPipe(execProcessWrite, 2);

                checkError(pId = fork(), FORK_ERROR_MESSAGE);
                if (parentProcess(pId)){   
                    closePipeEnds(parentWrite[0], execProcessWrite[1]);
                    instructionTokens = strtok(NULL, " \n");
                    if(instructionTokens != NULL){
                        checkError(write(parentWrite[1], instructionTokens, strlen(instructionTokens)), PIPE_ERROR_MESSAGE);
                        checkError(ret = read(execProcessWrite[0], buffer, BUFF_SIZE), PIPE_ERROR_MESSAGE);
                        if(ret == 0){ //if exec is successful
                            int listIterator = emptyIndexFinder();
                            setProcessId(listIterator, pId);
                            char processName [strlen(instructionTokens)];
                            sprintf(processName, "%s", instructionTokens);
                            setProcessName(listIterator, processName);
                            setStartingTime(listIterator);
                            activateProcess(listIterator, true);
                            checkError(write(msgsock, "Opening Status: SUCCESS\n", strlen("Opening Status: Success\n")), SOCKET_ERROR_MESSAGE);
                            maintainListOnServer(clientHandlerWrite[1], 1);              
                        }
                        else{
                            checkError(write(msgsock, buffer, strlen(buffer)), SOCKET_ERROR_MESSAGE);
                        }
                    }
                    else{
                        checkError(write(parentWrite[1], "dummy", strlen("dummy")), PIPE_ERROR_MESSAGE);
                        checkError(ret = read(execProcessWrite[0], buffer, BUFF_SIZE), PIPE_ERROR_MESSAGE);
                        checkError(write(msgsock, buffer, strlen(buffer)), SOCKET_ERROR_MESSAGE);
                    }
                }
                else if (childProcess(pId)){
                    char temp [BUFF_SIZE];
                    int ret;
                    closePipeEnds( parentWrite[1] , execProcessWrite[0]);
                    checkError(ret = read(parentWrite[0], temp, BUFF_SIZE), PIPE_ERROR_MESSAGE);
                    char application[ret];
                    nullTerminate(application, temp, ret);
                    execlp(application, application, NULL);
                    checkError(write(execProcessWrite[1], "Opening Status: FAILED.\n", strlen("Opening Status: FAILED.\n")), PIPE_ERROR_MESSAGE);
                }    
            }

            else if (killInstruction(instruction)){
                int status, processListIterator, processId, waitCheck;
                bool processFound = false;
                instructionTokens = strtok(NULL, " \n");
                
                if(instructionTokens != NULL){
                    processId = atoi (instructionTokens);
                    if (idIsGiven(processId)){
                        processListIterator = indexFinderByComparingProcessId(processId);
                        if(processListIterator >= 0){
                            processFound = true;
                        }
                        if (processFound){
                            checkError(ret = kill(processId, SIGTERM), "Error in kill");
                            if (ret >= 0)
                                checkError(write(msgsock, "Successfully killed.\n",strlen("successfully killed.\n")), SOCKET_ERROR_MESSAGE);                 
                            
                            status = 0; 
                            checkError(waitCheck = waitpid(processId, &status, 0), "Error in wait.");
                            activateProcess(processListIterator, false);
                            setEndingTime(processListIterator);
                        }
                        else{
                            checkError(write(msgsock, "Unsuccessful kill. Process not found.\n",strlen("unsuccessful kill. Process not found.\n")), SOCKET_ERROR_MESSAGE);
                        }
                    }
                    else if (!idIsGiven(processId)){
                        
                        string processName;
                        processName.assign(instructionTokens);
                        processListIterator = indexFinderByComparingNames(processName);
                        if(processListIterator >= 0){
                            processFound =true;
                        }
                        
                        if(processFound){
                            checkError(ret = kill(processList[processListIterator].processId, SIGTERM), "Error in kill.");
                            if (ret >= 0)
                                checkError(write(msgsock, "Successfully killed.\n",strlen("successfully killed.\n")), SOCKET_ERROR_MESSAGE);
                            
                            activateProcess(processListIterator, false);
                            setEndingTime(processListIterator);
                        }
                        else{
                            checkError(write(msgsock, "Process not killed as it doesn't exist.\n",strlen("process not killed as it doesn't exist.\n")), SOCKET_ERROR_MESSAGE);
                        }
                    }
                    else {
                        write(msgsock, "Failed in killing process.\n",strlen("failed in killing process.\n"));
                    }

                    maintainListOnServer(clientHandlerWrite[1], 1);
                    }
            else{
                write(msgsock, "Invalid Instruction.\n",strlen("Invalid Instruction.\n"));
            }
                sleep(1);
            }

            else if (exitInstruction(instruction)){
                exit(EXIT_SUCCESS);
            }

            else if (DisplayListInstruction(instruction)){
                int processListIterator;      
                instructionTokens = strtok(NULL, " /n");
                string param =   (string) instructionTokens;

                if (param.compare("all\n") == 0){
                    processListIterator = 0;
                    char temperoryList[SIZE], List[SIZE] = {};
                    tm startTime, endTime, elTime;
                    
                    while(processList[processListIterator].processId!= 0 ){
                        if(!processList[processListIterator].active){
                            //looks for terminated processes
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
                            //looks for active processes
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
                        checkError(write(msgsock,"No active processes.\n",strlen("No active processes.\n")), SOCKET_ERROR_MESSAGE);
                    }
                    else {
                        checkError(write(msgsock,List,strlen(List)), SOCKET_ERROR_MESSAGE);
                    }

                }
                else if (param.compare("active\n")==0){
                    maintainListOnServer(msgsock, 0);    
                }
                else{
                    checkError(write(msgsock, "Invalid instruction.\n", strlen("invalid  instruction.\n")), SOCKET_ERROR_MESSAGE);
                }

              sleep(1);
            }
            
            else{
                checkError(write(msgsock, "Invalid instruction.\n", strlen("invalid  instruction.\n")), SOCKET_ERROR_MESSAGE);
            }
            sleep(1);
        }
		close(msgsock);

        }
        else if (parentProcess(clientHandlerId)){
            close(clientHandlerWrite[1]);
            while(true){
            char temp [1000000] = {};
            int id, ret1, ret2;
            checkError(ret1 = read(clientHandlerWrite[0], &id, sizeof(int)), PIPE_ERROR_MESSAGE);
            checkError(ret2 = read(clientHandlerWrite[0], temp, sizeof(temp)), PIPE_ERROR_MESSAGE);
            char buff [ret2] = {};
            nullTerminate(buff, temp, ret2);
            active[id].list.assign(buff);
            }
        }
    return NULL;
}





void takeSuperUserInput(int pipe){
    int ret, ret1;
    while(true){
        printInstructions();
        char instr[LIST_SIZE]={};
        checkError(ret = read(STDIN_FILENO, instr, LIST_SIZE), READ_ERROR_MESSAGE); 
        checkError(ret1 = write(pipe, instr,strlen(instr)), PIPE_ERROR_MESSAGE);
        sleep(1);
    }
}

void changeListFormat(int clientId){
        int n = active[clientId].list.length();
        char char_array[n + 1],  id [sizeof(int)];
        strcpy(char_array, active[clientId].list.c_str());
        sprintf(id, "%d", active[clientId].clientId);
        printListOnServer(id, char_array);  
}

bool checkIfClientExists(int clientID){
    if(active[clientID].clientId == -1)
        return false;
    return true;                      
}

void *superUser(void *arg){
        int superUserWrite[2], superUserId;
        createPipe(superUserWrite, 1); // pipe between connection process and super user
        
        checkError(superUserId = fork(), FORK_ERROR_MESSAGE);
        if (childProcess(superUserId)){
            close(superUserWrite[0]);
            takeSuperUserInput(superUserWrite[1]);
        }
        else{
            close(superUserWrite[1]);
            char instr[LIST_SIZE]={};
            while(true){
                
                int ret;
                checkError(ret = read(superUserWrite[0], instr, LIST_SIZE), PIPE_ERROR_MESSAGE); 

                char * tok = tokenizer(instr);
                string command;
                command.assign(tok);

                if (DisplayListInstruction(command)){
                    tok = strtok(NULL, " \n");
                    command.assign(tok);

                    //printing process lists from every client handler on server
                    if(command.compare("all") == 0){
                        int count = 0;
                        while(active[count].clientId != -1){
                            changeListFormat(count);
                            count++;
                        }
                        
                    }

                    //printing process lists from specific client handler on server
                    else{
                        int clientID;
                        bool correctInput = true;

                        if (command.compare("0") == 0){
                            /*if we checked a client id of 0 with atoi, it will return 0. So to 
                              differentiate from error, used this if condition.*/
                            clientID = 0;
                        }
                        else{
                            clientID = atoi(tok);
                            if(clientID <= 0){
                                perror("Error in atoi");
                                correctInput = false;
                            }
                        }

                        correctInput = checkIfClientExists(clientID);
                        if(correctInput){
                            changeListFormat(clientID);
                            correctInput = false;
                        }
                        else{
                            checkError(write(STDOUT_FILENO, "Incorrect client ID.\n\n",strlen("Incorrect client ID.\n\n")), WRITE_ERROR_MESSAGE);
                        }
                    }    
                }

                else if (printOnClientInstruction(command)){
                    int clientId, index, n;
                    bool correctInput = true;
                    tok = strtok(NULL, " \n");
                    checkError(clientId = atoi (tok), "Error in atoi.");
                    command.assign(tok);
                     if (command.compare("0") == 0){
                            clientId = 0;
                    }
                    else{
                        clientId = atoi(tok);
                        if(clientId == 0){
                            correctInput = false;
                        }
                    }

                    char message [SIZE] = {};
                    if (!correctInput){
                        index = 0;
                        
                        while(tok != NULL){
                            strcat(message,tok);
                            strcat(message," ");
                            tok = strtok(NULL, " ");                           
                        }
                        
                        int j = 0;
                        while(message[j] != '\n'){
                            j++;
                            
                        }
                        message[j+1] = '\0';
                        
                        
                        
    
                        while(active[index].clientId!= -1){
                            write(active[index].socket, message , strlen(message));
                            index++;
                        }
                        
                     }
                     else{
                        tok = strtok(NULL, " ");         
                        while(tok != NULL){
                            strcat(message,tok);
                            strcat(message," ");
                            tok = strtok(NULL, " ");                      
                        }

                        int j = 0;
                        while(message[j] != '\n'){
                            j++;
                        }
                        message[j+1] = '\0';

                        write(active[clientId].socket, message , strlen(message));
                    }
                } 
            }
        }
    return NULL;
}

int main(void){
    signal(SIGCHLD, handler);
    initializeActiveList();

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
	server.sin_port = /*0;*/ htons(10000);
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
