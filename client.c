// client 

#include<stdio.h>
#include<stdlib.h>
#include<netdb.h>
#include<netinet/in.h>
#include<string.h>
#include<unistd.h>
#include<poll.h>
#include<sys/select.h>

#define EQUAL   -10
#define RETRIES 4
#define MAXLINE 256
#define TIMEOUT 1000


struct pollfd fd;
/*fd_set fds;
struct timeval tv;
int maxfdp1;*/


/**************************** Helper Functions ****************************/


int maximum(int x, int y)
{
	return x >= y? x : y;
}

void stringLower(char *str)
{
	/**Convert a string with uppercase letters to one with all lowercase.**/
	while(*str)
	{
		if(*str >= 65 && *str <= 90)
			*str += 32;
		str++;
	}
}

void stringUpper(char *str)
{
	/**Convert a string with lowercase letters to one with all uppercase.**/
	while(*str)
	{
		if(*str >= 97 && *str <= 122)
			*str -= 32;
		str++;
	}
}

void printError(char c)
{
	/**Printing error messages for socket functions.**/
	switch(c)
	{
		case 'a':
					perror("ERROR. Accepting socket.");
					break;
		case 'b':
					perror("ERROR. Binding socket.");
					break;
		case 'c':
					perror("ERROR. Connecting socket.");
					break;
		case 'f':
					perror("ERROR. Fork a child.");
					break;
		case 'h':
					perror("ERROR. No such host.");
					break;
		case 'o':
					perror("ERROR. Opening socket.");
					break;
		case 'r':
					perror("ERROR. Reading from socket.");
					break;
		case 't':
					perror("ERROR. Server terminated connection.");
					break;
		case 'w':
					perror("ERROR. Writing on socket.");
					break;
		default:
					perror("ERROR. Unknown.");
	}		
	exit(1);
}


/********************* Main Functionality Procedures **********************/


int startCommunication(int sockFd)
{
	/**
		Start the client communication with the server.
		Return: 1, when the user has successfully logged out.
	**/
	//printf("In startCommunication.\n");
	char sendMsg[MAXLINE], recvMsg[MAXLINE];
	int n, logout = 0, retVal = 0;

	// server providing options
	if((n = read(sockFd, recvMsg, MAXLINE)) < 0)
		printError('r');
	printf("\nServer :\n%s\n\nClient :\n", recvMsg);
	
	while(1)
	{
		/*FD_SET(fileno(stdin), &fds);
		FD_SET(sockFd, &fds);
		maxfdp1 = maximum(fileno(stdin), sockFd) + 1;
		retVal = select(maxfdp1, &fds, NULL, NULL, &tv);*/

		retVal = poll(&fd, 1, TIMEOUT);
		if(retVal == 0)
		{
			fputs("Timeout.\n", stdout);
			return -1;
		}
		else
		{
		/*	if(FD_ISSET(fileno(stdin), &fds))
			{
*/
				fgets(sendMsg, MAXLINE, stdin);

				// choosing an option
				if((n = write(sockFd, sendMsg, MAXLINE)) < 0)
					printError('w');
				if(n == 0)
					printError('t');
		/*	}
			
			if(FD_ISSET(sockFd, &fds))
			{
		*/		// server's reply
				if((n = read(sockFd, recvMsg, MAXLINE)) < 0)
					printError('r');
				if(n == 0)
					printError('t');
				
				printf("\nServer : \n%s\nClient : \n ", recvMsg);
				stringLower(recvMsg);
				if(strcmp("logout successful.", recvMsg) == 0)
				{
					logout = 1;
					return logout;
				}
			//}
		}
	}
}


int logIn(int sockFd)
{
	/**
		Asks client to provide credentials to log in with.
		Return: 1, if log in is successful
				-1, if there's some error
	**/
	//printf("In logIn\n");
	char sendMsg[MAXLINE], recvMsg[MAXLINE];
	char welcome[MAXLINE] = "WELCOME ";
	int n, i, k, count = 0, wrong = 0;
	int logout;

	bzero(&recvMsg, MAXLINE);
	bzero(&sendMsg, MAXLINE);
	
	// server asking for credentials to log in
	//printf("server asking for credentials to log in\n");
	if((n = read(sockFd, recvMsg, MAXLINE)) < 0)
		printError('w');
	if(n == 0)
		printError('t');

	if(strlen(recvMsg) == 0)
	{
		fputs("Server not responding. Closing connection.", stdout);
		exit(1);
	}
	printf("\nServer :\n%s\n", recvMsg);
	printf("\nClient : \n");
	fgets(sendMsg, MAXLINE, stdin);
	
	// providing log in credentials
	if((n = write(sockFd, sendMsg, MAXLINE)) < 0)
		printError('w');
	if(n == 0)
		printError('t');
	
	bzero(&recvMsg, MAXLINE);
	if((n = read(sockFd, recvMsg, MAXLINE)) < 0)
		printError('r');
	if(n == 0)
		printError('t');
	printf("\n%s\n\n", recvMsg);

	// matching the welcome message and id
	k = strlen(welcome);
	for(i = 0; sendMsg[i] != ' '; ++i, ++k)
		welcome[k] = sendMsg[i];
	welcome[k] = '\0';

	if(strcmp(welcome, recvMsg) != 0)
		wrong = 1;

	// begin operation
	if(!wrong)
		logout = startCommunication(sockFd);
	else
		logout = -1;
	return logout;
}


void regis(int sockFd)
{
	/**
		Asks client for id and password to register with.
	**/
	//printf("In regis.\n");
	char sendMsg[MAXLINE], recvMsg[MAXLINE];
	int n;

	while(1)
	{
		// server asking for credentials
		//printf("In regis\n");
		//printf("server asking for credentials\n");
		
		if((n = read(sockFd, recvMsg, MAXLINE)) < 0)
			printError('r');
		if(n == 0)
			printError('t');
		if(strlen(recvMsg) == 0)
		{
			fputs("Server not responding. Closing connection.", stdout);
			exit(1);
		}
		printf("\nServer : \n%s\n", recvMsg);
		
		printf("\nClient : \n");
		fgets(sendMsg, MAXLINE, stdin);

		// providing credentials to server
		printf("providing credentials to server\n");
		if((n = write(sockFd, sendMsg, MAXLINE)) < 0)
			printError('w');
		if(n == 0)
			printError('t');

		// response from the server
		printf("response from the server\n");
		if((n = read(sockFd, recvMsg, MAXLINE)) < 0)
			printError('r');
		if(n == 0)
			printError('t');
		
		stringLower(recvMsg);
		printf("server : \n%s\n", recvMsg);
		//printf("%d", strcmp("success", recvMsg));
		if(strcmp("registered successfully", recvMsg) == 0)
		{
		//	printf("Returning to start client\n");
			return;
		}
	}
}


void startClient(int sockFd)
{
	/**
		Start the client side operations.
	**/
	//printf("In startClient.\n");
	char sendMsg[MAXLINE], recvMsg[MAXLINE];
	int n, retVal;
	int logout = 0, retry = 0;
	
	while(1)
	{
		//printf("In startClient\n");
		bzero(&recvMsg, MAXLINE);
		bzero(&sendMsg, MAXLINE);

		// message from the server
		if((n = read(sockFd, recvMsg, MAXLINE)) < 0)
			printError('r');
		if(n == 0)
			printError('t');
		printf("\nServer : \n%s\n", recvMsg);
		
		//printf("sending response to server\n");
		printf("\nClient : \n");
		
		// sending response to server
		fgets(sendMsg, MAXLINE, stdin);
		if((n = write(sockFd, sendMsg, MAXLINE)) < 0)
			printError('w');
		if(n == 0)
			printError('t');
		
		// register
		if(strcmp("register", sendMsg) == EQUAL)
		{
			regis(sockFd);
			//printf("returned to start client\n");
			logout = logIn(sockFd);
			if(logout)
			{
				fputs("\nWrite ./client 127.0.0.1 9876 to reconnect.\n\n", stdout);
				return;
			}
		}
		else
			// log in
			if(strcmp("log in", sendMsg) == EQUAL || strcmp("login", sendMsg) == EQUAL)
			{
				logout = logIn(sockFd);
				if(logout == 1)
				{
					fputs("\nWrite ./client 127.0.0.1 9876 to reconnect.\n\n", stdout);
					return;
				}
			}
		
		if(logout == -1)
		{
			fputs("Something went wrong.\n", stdout);
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
	if((sockFd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		printError('o');

	/*FD_ZERO(&fds);
	FD_SET(sockFd, &fds);
	FD_SET(fileno(stdin), &fds);
	maxfdp1 = maximum(fileno(stdin), sockFd) + 1;
	tv.tv_sec = TIMEOUT;
	tv.tv_usec = 0;
*/
	fd.fd = sockFd | fileno(stdin);
	fd.events = POLLIN | POLLRDNORM;

	if((server = gethostbyname(argv[1])) < 0)
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