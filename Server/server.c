// server

#include<stdio.h>
#include<stdlib.h>
#include<netdb.h>
#include<netinet/in.h>
#include<string.h>
#include<unistd.h>

#define MAXLINE 500
#define LISTENQ 5

void register(int connFd)
{
	char regMsg[] = "Enter desired ID and Password.(ID -- Password)";

}

void logIn(int connFd)
{
	char logMsg[] = "Enter your ID and Password for Log In.(ID -- Password)";
	
}

void startServer(int connFd)
{
	char buffer[MAXLINE], sendMsg[MAXLINE], recvMsg[MAXLINE];
	char start[] = "Do you want to Register or Log In.";
	
	char regSucc[] = "Registered successfully.";
	
	char logSucc[] = "Successfully Logged In.";
	char logOut[] = "Successfully Logged Out.";
	int n;
	strcpy(sendMsg, start);
	n = write(connFd, sendMsg, MAXLINE);
	if(n < 0)
	{
		perror("ERROR. Writing on socket.");
		exit(1);
	}
	while(true)
	{
		n = read(connFd, recvMsg, MAXLINE);
		if(n < 0)
		{
			perror("ERROR. Reading from socket.");
			exit(1);
		}
		if(strcmp("register", tolower(recvMsg)) == 0)
		{
			register(connFd);
		}
		else
			if(strcmp("log in", tolower(recvMsg)) == 0)
			{
				logIn(connFd);
			}

	}
}

int main(int argc, char const *argv[])
{
	int listenFd, connFd, port = 9876, pId;
	struct sockaddr_in servAddr, cliAddr;
	printf("Server is up and running ...\n");
	// opening the socket
	if((listenFd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("ERROR. In opening socket.\n");
		exit(1);
	}
	// setting socket address structure
	bzero(&servAddr, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = INADDR_ANY;
	servAddr.sin_port = htons(port);
	// binding the server to the port
	if(bind(listenFd, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0)
	{
		perror("ERROR. On binding server.\n");
		exit(1);
	}
	// listening on the port
	listen(listenFd, LISTENQ);
	unsigned int clilen = sizeof(cliAddr);
	while(true)
	{
		// accept a client
		connFd = accept(listenFd, (struct sockaddr*)&cliAddr, &clilen);
		if(connFd < 0)
		{
			perror("ERROR. On accept.\n");
			exit(1);
		}
		// fork a child process
		pId = fork();
		if(pId < 0)
		{
			perror("ERROR. On fork.\n");
			exit(1);
		}
		// run the child process
		if(pId == 0)
		{
			close(listenFd);
			startServer(connFd);
			exit(0);
		}
		else
		{
			close(connFd);
		}
	}
	// closing the socket
	close(connFd);
	close(listenFd);
	bzero(&servAddr, sizeof(servAddr));
	return 0;
}