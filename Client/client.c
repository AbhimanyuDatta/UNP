//client 

#include<stdio.h>
#include<stdlib.h>
#include<netdb.h>
#include<netinet/in.h>
#include<string.h>
#include<unistd.h>

#define MAXLINE 500

int main(int argc, char const *argv[])
{
	int sockfd, port = 9876, n;
	struct sockaddr_in servaddr;
	struct hostent *server;
	char buffer[MAXLINE];
	if(argc != 2)
	{
		fprintf(stderr, "Not the right number of parameters required after %s.\n", argv[0]);
		exit(0);
	}
	port = atoi(argv[2]);
	// opening the socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
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
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	bcopy(server->h_addr, &servaddr.sin_addr.s_addr, server->h_length);
	servaddr.sin_port = htons(port);
	// connecting to the server
	if(connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
	{
		perror("ERROR connecting client to server.\n");
		exit(1);
	}
	while(true)
	{
		printf("Please enter the message : ");
		bzero(buffer, MAXLINE);
		fgets(buffer, MAXLINE, stdin);
		// writing on the socket
		n = write(sockfd, buffer, sizeof(buffer));
		if(n < 0)
		{
			perror("ERROR. Writing on socket.\n");
			exit(1);
		}
		// reading from the socket
		n = read(sockfd, buffer, MAXLINE);
		if(n < 0)
		{
			perror("ERROR. Reading from socket.\n");
			exit(1);
		}
		printf("Server says : %s\n", buffer);
	}
	close(sockfd);
	return 0;
}