# Client-Server-model-using-linux-API
This project depicts a client-server model. This project uses linux's system API. In this project,
the client makes requests to the server and in return the server caters the requests and sends response to clients.
The server is  multi-client so it can cater multiple client, on different machines, together simultaneous.

The project has two user modes:
  1) Client 
  2) Super User (runs instructions directly on server)

1. Client:
The client can run following commands on client terminal:
  • add <number list> : adds the provided list of numbers.(make sure to press spacebar before pressing enter)
  • sub <number list> : subtracts the provided list of numbers.(make sure to press spacebar before pressing enter)
  • mul <number list> : multiplies the provided list of numbers.(make sure to press spacebar before pressing enter)
  • div <number list> : divides the provided list of numbers.(make sure to press spacebar before pressing enter)
  • run <application name> : opens the application, whose name is provided, on server side . 
  • list active: lists all the active running apps with their process ids, app name, start time.
  • list all: lists all the apps, which were ever opened, with their process ids, app name, start time, end time.
  • kill <process id> : kills the application having process id provided.
  • kill <process name> : kills the application having process name provided.
  • exit : terminates the client.
  
2. Server:
The super user can run following commands on server terminal:
  • list all : lists all the applications currently opened on server by all clients.
  • list <client id> : lists all the applications currently opened on server by the client whose id is provided.
  • print <message> : prints message on every client's terminal. 
  • print <client id> <message> : prints message on client's terminal whose id is provided. 
   
