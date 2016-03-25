//client 

#include<stdio.h>
#include<stdlib.h>
#include<netdb.h>
#include<netinet/in.h>
#include<string.h>
#include<unistd.h>

#define MAXLINE 500

void startClient(int sockFd)
{
	char buffer[MAXLINE], sendMsg[MAXLINE], recvMsg[MAXLINE];
	int n;
	bool entry = false;
	while(!entry)
	{
		n = read(sockFd, recvMsg, MAXLINE);
		if(n < 0)
		{
			perror("ERROR. Reading from socket.\n");
			exit(1);
		}
		printf("Server : %s\n", recvMsg);
		fgets(sendMsg, MAXLINE, stdin);
		n = write(sockFd, sendMsg, MAXLINE);
		if(n < 0)
		{
			perror("ERROR. Writing on socket.\n");
			exit(1);
		}
		
		if(strcmp("register", tolower(sendMsg)) == 0)
		{
			n = read(sockFd, recvMsg, MAXLINE);
			if(n < 0)
			{
				perror("ERROR. Reading from socket.\n");
				exit(1);
			}
			bzero(sendMsg, MAXLINE);
			fgets(sendMsg, MAXLINE, stdin);
			

		}
		if(strcmp("registered successfully", tolower(recvMsg)) == 0)
		{
			entry = true;
		}
	}
	// successfull log in
	while(true)
	{
		break;
	}
}

int main(int argc, char const *argv[])
{
	int sockFd, port = 9876, n;
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
	{
		perror("ERROR opening socket on client.\n");
		exit(1);
	}
	server = gethostbyname(argv[1]);
	if(server == NULL)
	{
		fprintf(stderr, "ERROR, no such host\n");
		exit(0);
	}
	// setting the socket address structure
	bzero(&servAddr, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	bcopy(server->h_addr, &servAddr.sin_addr.s_addr, server->h_length);
	servAddr.sin_port = htons(port);
	// connecting to the server
	if(connect(sockFd, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0)
	{
		perror("ERROR connecting client to server.\n");
		exit(1);
	}
	startClient(sockFd);
	close(sockFd);
	return 0;
}