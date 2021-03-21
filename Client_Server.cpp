#include <unistd.h>
#include <error.h>
#include <errno.h>
#include <iostream>

using namespace std;

int main (){
    
    //Creating pipe
    int fd[2];
    if (pipe(fd) < 0)
        perror("Error in piping on line 11: " + errno);

    //Creating Client & Server processes
    int id = fork();
    if(id < 0)
    {
        perror("Error while creating Client and Server processes in line 18: "+ errno);
    }
    else if (id == 0) 
    {
        cout<<"in child"<<endl;
    }
    else
    {
        cout<<"in parent"<<endl;
    }




    return 0;
}