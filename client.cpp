#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <iostream>
#include <signal.h>
#include <time.h>
#include <sstream>
#include <sys/wait.h>
#include <fcntl.h>
#include <pthread.h>
using namespace std;

#define PRINT_ERROR_MESSAGE "Error in printing."
#define READ_ERROR_MESSAGE "Error in reading."
#define SOCKET_ERROR_MESSAGE "Error in printing."
#define size 100

char* tokenizer(char numberList []){
    char * listToken = strtok(numberList, " \n");  
    return listToken;
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

bool instructionIsToExit(char instruction []){
    if((instruction[0]=='e' && instruction[1]=='x' && instruction[2]=='i' && instruction[3]=='t'))
        return true;
    return false;
}

void printInstructions(){
    char seperator [] = "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
    char outputMessageForInstruction [] = "You have the following commands: add, sub, mul, div, run, kill, list, listall, exit.\n" ;
    char outputMessageForSyntax [] = "\n---------Syntax Examples-------\nadd 5 8 \nrun gedit\nkill 12345\nkill gedit\nlist\nlistall\nexit\n------------------------------\n";
    char outputMessageForInput [] = "\nEnter your instruction: ";

    checkError(write(STDOUT_FILENO, &seperator , strlen(seperator)), PRINT_ERROR_MESSAGE);
    checkError(write(STDOUT_FILENO, &outputMessageForInstruction , strlen(outputMessageForInstruction)), PRINT_ERROR_MESSAGE);
    checkError(write(STDOUT_FILENO,outputMessageForSyntax, strlen(outputMessageForSyntax)), PRINT_ERROR_MESSAGE);
    checkError(write(STDOUT_FILENO, &outputMessageForInput, strlen(outputMessageForInput)), PRINT_ERROR_MESSAGE);
} 

void displayOutput(char * response, int ret){
    checkError(write(STDOUT_FILENO, response, ret), PRINT_ERROR_MESSAGE);
    checkError(write(STDOUT_FILENO,"\n\n\n",3), PRINT_ERROR_MESSAGE);
}

void* userReadFunction(void *args){
    int sock = *(int *)args;
    char instruction[size]={};
    int ret;

    printInstructions();
    checkError(ret = read(STDIN_FILENO, instruction, size), READ_ERROR_MESSAGE);
    checkError(write(sock, instruction, ret), SOCKET_ERROR_MESSAGE);
    if(instructionIsToExit(instruction))
        exit(EXIT_SUCCESS);
    return NULL;
}

int main(int argc, char *argv[])
	{
	int sock;
	struct sockaddr_in server;
	struct hostent *hp;
	char buf[1024];

	/* Create socket */
	sock = socket(AF_INET, SOCK_STREAM , 0);
	if (sock < 0) {
		perror("opening stream socket");
		exit(1);
	}

	/* Connect socket using name specified by command line. */
	server.sin_family = AF_INET;
	hp = gethostbyname(argv[1]);
	if (hp == 0) {
		fprintf(stderr, "%s: unknown host\n", argv[1]);
		exit(2);
	}

	bcopy(hp->h_addr, &server.sin_addr, hp->h_length);
	server.sin_port = htons(atoi(argv[2]));

	if (connect(sock,(struct sockaddr *) &server,sizeof(server)) < 0) {
		perror("connecting stream socket");
		exit(1);
	}

	while(1) {
        bool keepRunning = true;
        int ret;
        while(keepRunning){
            char response[size * size] = {};
            pthread_t outputThread;
            pthread_create(&outputThread, NULL, userReadFunction, (void *)&sock);
            pthread_detach(outputThread);
            checkError(ret = read (sock, response, (size*size)), READ_ERROR_MESSAGE);
            displayOutput(response, ret);
        }  
	}
	close(sock);
}