//client 

#include<stdio.h>
#include<stdlib.h>
#include<netdb.h>
#include<netinet/in.h>
#include<string.h>
#include<unistd.h>
#include"myheader.h"

#define MAXLINE 500
#define EQUAL -10

int logId(int sockFd)
{
	char sendMsg[MAXLINE], recvMsg[MAXLINE];
	int n;

	bzero(&recvMsg, MAXLINE);
	bzero(&sendMsg, MAXLINE);
	
	// server asking for credentials to log in
	printf("server asking for credentials to log in\n");
	n = read(sockFd, recvMsg, MAXLINE);
	if(n < 0)
		printError('w');

	printf("\nServer :\n%s\n", recvMsg);
	fgets(sendMsg, MAXLINE, stdin);

	// providing log in credentials
	n = write(sockFd, sendMsg, MAXLINE);
	if(n < 0)
		printError('w');
	

}

int regis(int sockFd)
{
	char sendMsg[MAXLINE], recvMsg[MAXLINE];
	int n;
	int accept = 0;
	while(!accept)
	{// server asking for credentials
		printf("In regis\n");
			printf("server asking for credentials\n");
			n = read(sockFd, recvMsg, MAXLINE);
			if(n < 0)
				printError('r');
			printf("\nServer : \n%s\n", recvMsg);
			fgets(sendMsg, MAXLINE, stdin);

			// providing credentials to server
			printf("providing credentials to server\n");
			n = write(sockFd, sendMsg, MAXLINE);
			if(n < 0)
				printError('w');

			// response from the server
			printf("response from the server\n");
			n = read(sockFd, recvMsg, MAXLINE);
			if(n < 0)
				printError('r');
			stringLower(recvMsg);
			printf("server : \n%s\n", recvMsg);
			printf("%d", strcmp("success", recvMsg));
			if(strcmp("registered successfully", recvMsg) == 0)
			{
				printf("Returning to start client\n");
				return 1;
			}
	}

}

void startClient(int sockFd)
{
	char sendMsg[MAXLINE], recvMsg[MAXLINE];
	int n;
	int entry = 0, logId;
	while(!entry)
	{
		printf("In startClient\n");
		bzero(&recvMsg, MAXLINE);
		bzero(&sendMsg, MAXLINE);

		// first message from the server
		printf("first message from the server\n");
		n = read(sockFd, recvMsg, MAXLINE);
		if(n < 0)
			printError('r');
		printf("\nServer : \n%s\n", recvMsg);
		fgets(sendMsg, MAXLINE, stdin);

		// sending response to server
		printf("sending response to server\n");
		n = write(sockFd, sendMsg, MAXLINE);
		if(n < 0)
			printError('w');
		if(strcmp("register", sendMsg) == EQUAL)
		{
			entry = regis(sockFd);
			printf("returned to start client");
		}
		else
			if(strcmp("log in", sendMsg) == EQUAL || strcmp("login", sendMsg) == EQUAL)
			{
				logId = logIn(sockFd);
			}
	}
}

int main(int argc, char const *argv[])
{
	int sockFd, port, n;
	struct sockaddr_in servAddr;
	struct hostent *server;

	if(argc < 3)
	{
		fprintf(stderr, "Not the right number of parameters required after %s.\n", argv[0]);
		exit(0);
	}
	port = atoi(argv[2]);
	
	// opening the socket
	sockFd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockFd < 0)
		printError('o');
	server = gethostbyname(argv[1]);
	if(server == NULL)
		printError('h');
	
	// setting the socket address structure
	bzero(&servAddr, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	bcopy(server->h_addr, &servAddr.sin_addr.s_addr, server->h_length);
	servAddr.sin_port = htons(port);
	
	// connecting to the server
	if(connect(sockFd, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0)
		printError('c');
	startClient(sockFd);
	close(sockFd);
	return 0;
}