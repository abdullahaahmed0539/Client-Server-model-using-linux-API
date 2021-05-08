/*
 Copyright (c) 1986 Regents of the University of California.
 All rights reserved.  The Berkeley software License Agreement
 specifies the terms and conditions for redistribution.

	@(#)streamwrite.c	6.2 (Berkeley) 5/8/86
*/

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


/*
 * This program creates a socket and initiates a connection with the socket
 * given in the command line.  One message is sent over the connection and
 * then the socket is closed, ending the connection. The form of the command
 * line is streamwrite hostname portnumber 
 */
int size = 100;


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

void* inputFunction(void *args){
    int sock = *(int *)args;
    char instruction[size]={};
    int ret;
    char seperator [] = "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
    char outputMessageForInstruction [] = "You have the following commands: add, sub, mul, div, run, kill, list, listall, exit.\n" ;
    char outputMessageForSyntax [] = "\n---------Syntax Examples-------\nadd 5 8 \nrun gedit\nkill 12345\nkill gedit\nlist\nlistall\nexit\n------------------------------\n";
    char outputMessageForInput [] = "\nEnter your instruction: ";


    if(write(STDOUT_FILENO, &seperator , strlen(seperator)) < 0){
        perror("Error message 1. ");
    }

    if(write(STDOUT_FILENO, &outputMessageForInstruction , strlen(outputMessageForInstruction)) < 0){
        perror("Error message 1. ");
    }

    if (write(STDOUT_FILENO,outputMessageForSyntax, strlen(outputMessageForSyntax))< 0){
        perror("Error message 2. ");             
    }

    if(write(STDOUT_FILENO, &outputMessageForInput, strlen(outputMessageForInput)) < 0){
        perror("Error message 3. "); 
    }


    ret = read(STDIN_FILENO, instruction, 100); 
    if(ret < 0){
        perror("Error message 4. ");
    }
   

    if(write(sock, instruction, ret) < 0){
        perror("Error message 5. ");
    }


    char * instructionToken = tokenizer(instruction);
    if(instructionIsToExit(instruction,"")){
        exit(EXIT_SUCCESS);
    }
    

    return NULL;
}



int main(int argc, char *argv[])
	{
	int sock;
	struct sockaddr_in server;
	struct hostent *hp;
	char buf[1024];

	/* Create socket */
	sock = socket(AF_INET, SOCK_STREAM, 0);
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
        
        while(keepRunning){

            pthread_t outputThread;
            pthread_create(&outputThread, NULL, inputFunction, (void *)&sock);
            pthread_detach(outputThread);

            char response[size * size] = {};

            int ret = read (sock, response, (size*size));
            if (ret < 0){
                perror("Error message 6. ");
            }

            if(write(STDOUT_FILENO, response, ret) < 0){
                perror("Error message 7. ");
            }    

           
            write(STDOUT_FILENO,"\n\n\n",3);
            
            
            
            
        }

	  

	  
	}
	close(sock);
}