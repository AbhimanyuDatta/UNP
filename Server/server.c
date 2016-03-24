// server

#include<stdio.h>
#include<stdlib.h>
#include<netdb.h>
#include<netinet/in.h>
#include<string.h>
#include<unistd.h>

#define MAXLINE 500
#define LISTENQ 5

void startServer(int connfd)
{
	char buffer[MAXLINE], sendMsg[MAXLINE], recvMsg[MAXLINE];
	char start[] = "Do you want to Register or Log In.";
	char regMsg[] = "Enter desired ID and Password.";
	char regSucc[] = "Registered successfully.";
	char logMsg[] = "Enter your ID and Password for Log In.";
	char logSucc[] = "Successfully Logged In.";
	int n;
	bzero(sendMsg, MAXLINE);
	strcpy(sendMsg, start);
	n = write(connfd, sendMsg, MAXLINE);
	if(n < 0)
	{
		perror("ERROR. Server. Writing on socket.");
		exit(1);
	}
	while(true)
	{
		bzero(buffer, MAXLINE);
		n = read(connfd, buffer, MAXLINE);
		if(n < 0)
		{
			perror("ERROR Server. Reading from socket.");
			exit(1);
		}
		if(strcmp(buffer, "bye") != 0)
		{
			printf("Here is what is written on the socket : %s\n", buffer);
			// writing on the socket
			char msg[] = "Lunch ke baad aana.\n";
			n = write(connfd, msg, sizeof(msg));
			if(n < 0)
			{
				perror("ERROR. Writing on socket.");
				exit(1);
			}
		}
		else
		{
			printf("Client ended the connection.");
			break;
		}
	}
}

int main(int argc, char const *argv[])
{
	int listenfd, connfd, port = 9876, pid;
	struct sockaddr_in servaddr, cliaddr;
	printf("Server is up and running ...\n");
	// opening the socket
	if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("ERROR. In opening socket.");
		exit(1);
	}
	// setting socket address structure
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(port);
	// binding the server to the port
	if(bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
	{
		perror("ERROR. On binding server.");
		exit(1);
	}
	// listening on the port
	listen(listenfd, LISTENQ);
	unsigned int clilen = sizeof(cliaddr);
	while(true)
	{
		// accept a client
		connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &clilen);
		if(connfd < 0)
		{
			perror("ERROR. On accept.");
			exit(1);
		}
		// fork a child process
		pid = fork();
		if(pid < 0)
		{
			perror("ERROR. On fork.");
			exit(1);
		}
		// run the child process
		if(pid == 0)
		{
			close(listenfd);
			startServer(connfd);
			exit(0);
		}
		else
		{
			close(connfd);
		}
	}
	// closing the socket
	close(connfd);
	close(listenfd);
	bzero(&servaddr, sizeof(servaddr));
	return 0;
}