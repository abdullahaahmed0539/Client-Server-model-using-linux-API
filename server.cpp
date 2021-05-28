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



//STRUCTS
struct listProcess {
    int processId;
    string processName;
    time_t startTime, endTime, elapsedTime;
    bool active;
};

struct activeList {
    int clientId, socket;
    string list;
};



//GLOBAL DECLARATIONS
const int LISTSIZE = 20;
int clientHandlerRead[2];
listProcess processList [LISTSIZE];
activeList active [LISTSIZE];

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

bool instructionIsToExit(string instructionFromServer){
    if(instructionFromServer=="exit")
        return true;
    return false;
}

bool instructionIsForArithmeticOperations (string instruction){
    
    if((instruction == "add") || (instruction == "sub") || (instruction == "mul") || (instruction == "div"))
        return true;
    return false;
} 

bool instructionIsToRun(string instruction){
    if(instruction == "run")
        return true;
    return false;
}

bool instructionIsToKillProcess(string instruction){
    if(instruction == "kill")
        return true;
    return false;
}

bool instructionIsToDisplayList(string instruction){
    if(instruction == "listall" || instruction == "list")
        return true;
    return false;
}

bool instructionIsToPrintOnClient(string instruction){
    if(instruction == "print")
        return true;
    return false;
}


bool numberListHasNotEnded (char * instructionTokens){
    if(instructionTokens==NULL)
        return false;
    return true;
}

bool isParentProcess(pid_t processId){
    if (processId > 0)
        return true;
    return false;
}

int emptyIndexFinder(listProcess processList[]){
    int listIterator = 0;
    while(listIterator < LISTSIZE){
        if(processList[listIterator].processId != 0){
            listIterator++;
        }
        else{
            break;
        }
    }
    return listIterator;
}

int assignClientID(activeList active[]){
    int listIterator = 0;
    while(listIterator < LISTSIZE){
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

int indexFinderByComparingProcessId(listProcess processList[], int requiredProcessId){
    int listIterator = 0;
    bool foundProcess;
    while(listIterator < LISTSIZE){
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

int indexFinderByComparingNames(listProcess processList[], string processName){
    int listIterator = 0;
    bool foundProcess;
    while(listIterator < LISTSIZE){
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
    if (processId == 0)
        return true;
    return false;
}

bool processIdIsGiven(int returnValueOfAtoiFunction){
    if(returnValueOfAtoiFunction > 0)
        return true;
    return false;
}   

bool processNameIsGiven(int returnValueOfAtoiFunction){
    if(returnValueOfAtoiFunction == 0)
        return true;
    return false;
} 

void checkError(int number){
    if (number < 0)
        perror("Error");
}

void createPipe(int p []){
    if(pipe(p) < 0)
        perror("Error in pipe");
}

void createPipe2(int p []){
    if (pipe2(p, O_CLOEXEC) < 0)
        perror("Error in piping for exec ");              
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


void activateProcess(int i){
    processList[i].active = true;
}

void deactivateProcess(int i){
    processList[i].active = false;
}

void setProcessId(int i, int id){
    processList[i].processId = id;
}

void setProcessName(int i, string name){
    processList[i].processName = name;
}

void initializeProcessList (){
    for (size_t index = 0; index < LISTSIZE; index++)
        processList[index].processId = 0;
}

void initializeActiveList (){
    for (size_t index = 0; index < LISTSIZE; index++){
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
    char outputMessageForInstruction [] = "You have the following commands: print, print client, list, list client, exit.\n" ;
    char outputMessageForInput [] = "Enter your instruction: ";
    checkError(write(STDOUT_FILENO, &outputMessageForInstruction , strlen(outputMessageForInstruction)));
    checkError(write(STDOUT_FILENO, &outputMessageForInput, strlen(outputMessageForInput)));
}

void printListOnServer(char id [] , char char_array [] ){
    checkError(write(STDOUT_FILENO, "\n", 1 ));
    checkError(write(STDOUT_FILENO, "Client ID :", strlen("Client ID :")));
    checkError(write(STDOUT_FILENO, id, strlen(id)));
    checkError(write(STDOUT_FILENO, "\n",1 ));
    if (char_array[0] == '\0'){
        checkError(write(STDOUT_FILENO, "Client not connected any more\n" , strlen("Client not connected any more\n")));
    }else{
        checkError(write(STDOUT_FILENO, char_array , strlen(char_array)));
    }
    
    checkError(write(STDOUT_FILENO, "\n",1 ));
}

void printAnswer(double *answer, char buffer [], int msgsock){
    int sprinfReturn = sprintf(buffer, "Answer: %.2f\n", *answer);
    checkError(write(msgsock,buffer,sprinfReturn));
    *answer = 0;
}

void handleFirstNumberProblem(bool* isFirstNumberFromList, double* answer, double value){
    if (*isFirstNumberFromList){
        *answer  = value;
        *isFirstNumberFromList = false;
    }
}


void handler(int sig){
    bool processFound;
    int childIdofKilledProcess, processListIterator;

    if(sig == SIGCHLD){
        childIdofKilledProcess = waitpid(-1, NULL, WNOHANG);
        processListIterator = indexFinderByComparingProcessId(processList, childIdofKilledProcess);
        if(processListIterator >= 0)
            processFound = true;           
        if (processFound){
            deactivateProcess(processListIterator);
            setEndingTime(processListIterator);
        }
        
    }
}

void *clientHandler(void * arg){
    
    int msgsock = *(int *)arg;
    int clientHandlerWrite [2];
    int clientId, clientHandlerId;
    createPipe(clientHandlerWrite);
    clientId = assignClientID(active);
    active[clientId].socket = msgsock;


    checkError(clientHandlerId = fork());
    if (isChildProcess(clientHandlerId)){
        closePipeEnds(clientHandlerRead[1],clientHandlerWrite[0]);
        int ret;
        const int BUFFERSIZE = 50;
        bool keepRunning = true;
        initializeProcessList();
        while(keepRunning){
            char recievedCommand [BUFFERSIZE] = {};     
            string instruction;
            checkError(ret = read(msgsock, recievedCommand, BUFFERSIZE));
            char* instructionTokens = tokenizer(recievedCommand);
            instruction.assign(instructionTokens);
            
            if(instructionIsForArithmeticOperations(instruction)){   
                char buffer [BUFFERSIZE]; 
                double answer = 0;
                bool firstNumberFromTheNumberList = true;                
                instructionTokens = strtok(NULL," ");

                while(numberListHasNotEnded(instructionTokens)){
                    if(!isListNumerical(instructionTokens)){
                        checkError(write(msgsock, "List is not numerical.\n", strlen("List is not numerical.\n")));
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
                                checkError(write(msgsock ,"Can't divide by 0\n", strlen("Can't divide by 0\n")));
                                break;
                            } 
                        }
                        instructionTokens = strtok(NULL, " ");
                    } 
                }
            }

            else if (instructionIsToRun(instruction)){   
                char buffer [BUFFERSIZE] = {};
                int parentWrite[2], execProcessWrite [2], pId;
                createPipe2(parentWrite);
                createPipe2(execProcessWrite);

                checkError(pId = fork());
                if (isParentProcess(pId)){   
                    closePipeEnds(parentWrite[0], execProcessWrite[1]);
                    instructionTokens = strtok(NULL, " \n");
                    checkError(write(parentWrite[1], instructionTokens, strlen(instructionTokens)));
                    checkError(ret = read(execProcessWrite[0], buffer, BUFFERSIZE));
                    if(ret == 0){
                        int listIterator = emptyIndexFinder(processList);
                        setProcessId(listIterator, pId);
                        char processName [strlen(instructionTokens)];
                        sprintf(processName, "%s", instructionTokens);
                        setProcessName(listIterator, processName);
                        setStartingTime(listIterator);
                        activateProcess(listIterator);
                        checkError(write(msgsock, "Opening Status: SUCCESS\n", strlen("Opening Status: Success\n")));

                        int processListIterator = 0;
                        const int SIZE = BUFFERSIZE * BUFFERSIZE;
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
                        checkError(write(clientHandlerWrite[1], &clientId,sizeof(clientId)));
                        if(strlen(List) <= 0){
                            checkError(write(clientHandlerWrite[1], "No processes.\n",strlen("No active processes.\n")));
                        }
                        else{
                            checkError(write(clientHandlerWrite[1],List,strlen(List)));
                        }    
                    }
                    else{
                        checkError(write(msgsock, buffer, strlen(buffer)));
                    }

                    
                }
                else{
                    char temp [BUFFERSIZE];
                    int ret;
                    closePipeEnds( parentWrite[1] , execProcessWrite[0]);
                    checkError(ret = read(parentWrite[0], temp, BUFFERSIZE));
                    char application[ret];
                    nullTerminate(application, temp, ret);
                    checkError(execlp(application, application, NULL));
                    checkError(write(execProcessWrite[1], "Opening Status: FAILED.\n", strlen("Opening Status: FAILED.\n")));
                }    
            }

            else if (instructionIsToKillProcess(instruction)){
                int status, processListIterator, processId, waitCheck;
                bool processFound;
                instructionTokens = strtok(NULL, " \n");
                processId = atoi (instructionTokens);
                
                if (processIdIsGiven(processId)){
                    processListIterator = indexFinderByComparingProcessId(processList, processId);
                    if(processListIterator >= 0){
                        processFound = true;
                    }
                    if (processFound){
                        checkError(ret = kill(processId, SIGTERM));
                        if (ret >= 0)
                            checkError(write(msgsock, "Successfully killed.\n",strlen("successfully killed.\n")));                 
                        
                        status = 0; 
                        checkError(waitCheck = waitpid(processId, &status, 0));
                        deactivateProcess(processListIterator);
                        setEndingTime(processListIterator);
                    }
                    else{
                        checkError(write(msgsock, "Unsuccessful kill. Process not found.\n",strlen("unsuccessful kill. Process not found.\n")));
                    }
                }
                else if (processNameIsGiven(processId)){
                    
                    string processName;
                    processName.assign(instructionTokens);
                    processListIterator = indexFinderByComparingNames(processList, processName);
                    if(processListIterator >= 0){
                        processFound =true;
                    }
                    if(processFound){
                        checkError(ret = kill(processList[processListIterator].processId, SIGTERM));
                        if (ret >= 0)
                            checkError(write(msgsock, "Successfully killed.\n",strlen("successfully killed.\n")));
                        
                        deactivateProcess(processListIterator);
                        setEndingTime(processListIterator);
                    }
                    else{
                        checkError(write(msgsock, "Process not killed as it doesnt exist.\n",strlen("process not killed as it doesnt exist.\n")));
                    }
                }
                else {
                    write(msgsock, "Failed in killing process.\n",strlen("failed in killing process.\n"));
                }

                processListIterator = 0;
                const int SIZE = BUFFERSIZE * BUFFERSIZE;
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
                checkError(write(clientHandlerWrite[1], &clientId,sizeof(clientId)));
                if(strlen(List) <= 0){
                    checkError(write(clientHandlerWrite[1], "No processes.\n",strlen("No active processes.\n")));
                }
                else{
                    checkError(write(clientHandlerWrite[1],List,strlen(List)));
                } 
                sleep(1);
            }

            else if (instructionIsToExit(instruction)){
                exit(EXIT_SUCCESS);
            }

            else if (instructionIsToDisplayList(instruction)){
                int processListIterator;      
                const int SIZE = BUFFERSIZE * BUFFERSIZE;
                instructionTokens = strtok(NULL, " /n");
                string param =   (string) instructionTokens;

                if (param.compare("all\n") == 0){
                    processListIterator = 0;
                    char temperoryList[SIZE], List[SIZE] = {};
                    tm startTime, endTime, elTime;
                    int startTimeFormatted[3];
                    int* endTimeFormatted;
                    int* elapsedTimeFormatted;
                    
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
                        checkError(write(msgsock,"No active processes.\n",strlen("No active processes.\n")));
                    }
                    else {
                        checkError(write(msgsock,List,strlen(List)));
                    }
                }
                else if (param.compare("active\n")==0){
                    processListIterator = 0;
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
                    if(strlen(List) <= 0){
                        checkError(write(msgsock,"No active processes.\n",strlen("No active processes.\n")));
                    }
                    else{
                        checkError(write(msgsock,List,strlen(List)));
                    }    
                }
                else{
                    checkError(write(msgsock, "Invalid instruction.\n", strlen("invalid  instruction.\n")));
                }
              sleep(1);
            }

            else{
                checkError(write(msgsock, "Invalid instruction.\n", strlen("invalid  instruction.\n")));
            }
            sleep(1);
        }
		close(msgsock);

        }
        else if (isParentProcess(clientHandlerId)){
            close(clientHandlerWrite[1]);
            while(true){
            char temp [1000000] = {};
            int id, ret1, ret2;
            checkError(ret1 = read(clientHandlerWrite[0], &id, sizeof(int)));
            checkError(ret2 = read(clientHandlerWrite[0], temp, sizeof(temp)));
            char buff [ret2] = {};
            nullTerminate(buff, temp, ret2);
            active[id].list.assign(buff);
            }


        }


    return NULL;
}

void *superUser(void *arg){
        int superUserWrite[2], s_id, ret;
        createPipe(superUserWrite);
        checkError(s_id = fork());
        if (isChildProcess(s_id)){
            close(superUserWrite[0]);
            while(true){
                printInstructions();
                char instr[LISTSIZE]={};
                checkError(ret = read(STDIN_FILENO, instr, LISTSIZE)); 
                checkError(write(superUserWrite[1], instr,strlen(instr)));
                sleep(1);
            }
        }
        else{
            close(superUserWrite[1]);
            char instr[LISTSIZE]={};
            while(true){
                checkError(ret = read(superUserWrite[0], instr, LISTSIZE)); 
                char * tok = tokenizer(instr);
                string command;
                command.assign(tok);

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
                            printListOnServer(id, char_array);
                            count++;
                        }
                    }
                    else{
                        int clientID;
                        bool correctInput = true;
                        if (command.compare("0") == 0){
                            clientID = 0;
                        }
                        else{
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
                            printListOnServer(id, char_array);
                            correctInput = false;
                        }
                        else{
                            checkError(write(STDOUT_FILENO, "Incorrect client ID.\n\n",strlen("Incorrect client ID.\n\n")));
                        }
                    }    
                }
                else if (instructionIsToPrintOnClient(command)){
                    int clientId, index, n;
                    bool correctInput = true;
                    tok = strtok(NULL, " \n");
                    checkError(clientId = atoi (tok));
                    command.assign(tok);
                     if (command.compare("0") == 0){
                            clientId = 0;
                    }
                    else{
                        cout << strlen(tok)<<endl;
                        clientId = atoi(tok);
                        if(clientId == 0){
                            correctInput = false;
                        }
                    }


                    
                    char message [1000] = {};
                    if (!correctInput){
                        index = 0;
                        
                        while(tok != NULL){
                            cout << tok << endl;
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

                        for (size_t k = j+1; k < sizeof(message); k++)
                        {
                            /* code */
                            message[k] = '\0';
                        }
                        write(active[clientId].socket, message , strlen(message));
                    }
                    
                    
                    
                    
                } 
            }
        }
    

 
    return NULL;
}

int main(void){
    signal(SIGCHLD, handler);
    createPipe(clientHandlerRead);
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
