// client 

#include<stdio.h>
#include<stdlib.h>
#include<netdb.h>
#include<netinet/in.h>
#include<string.h>
#include<unistd.h>
#include<poll.h>
#include<time.h>

#define EQUAL       -10
#define RETRIES     2
#define SERVERCRASH 5
#define MAXLINE     256
#define TIMEOUT     3000

struct pollfd fd;
int rndm, r;

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
	char sendMsg[MAXLINE], recvMsg[MAXLINE], discard[MAXLINE], temp[MAXLINE];
	int n, logout = 0, retVal = 0, i = 0, crash;

	// server providing options
	if((n = read(sockFd, recvMsg, MAXLINE)) < 0)
		printError('r');
	printf("\nServer :\n%s\n", recvMsg);

	r = rand()%10;		// these values are generated only once, otherwise
	crash = rand()%10;  // they disrupt the normal flow of the communication

	while(1)
	{
		// choosing an option
		printf("%d %d %d ", rndm, r, crash);
		fputs("\nClient : \n", stdout);
		fgets(sendMsg, MAXLINE, stdin);
		if((n = write(sockFd, sendMsg, MAXLINE)) < 0)
			printError('w');
		if(n == 0)
			printError('t');
		stringLower(sendMsg);
		
		if(strcmp("unexpected response", sendMsg) == EQUAL)
		{
			while(n = read(sockFd, recvMsg, MAXLINE))
			{
				printf("\nServer : \n%s\n", recvMsg);
			}
			if(n == 0)
				printError('t');
			return 1;
		}

		if(crash == SERVERCRASH) // simulate server crash : done
		{
			bzero(&temp, MAXLINE);
			bzero(&discard, MAXLINE);
			sprintf(discard, "%d", r);
			strcat(discard, sendMsg);
			retVal = poll(&fd, 1, TIMEOUT);
			if(retVal == 0)
			{
				fputs("Timeout\n", stdout);
				sleep(TIMEOUT/1000);
				i = 0;
				while(i++ < RETRIES)
				{
					fputs("Retrying. Sending request again.\n", stdout);
					n = write(sockFd, discard, MAXLINE);
					retVal = poll(&fd, 1, TIMEOUT);
					if(retVal)
						if((n = read(sockFd, temp, MAXLINE)) == 0)
							printError('t');
					sleep(TIMEOUT/1000);
				}
				if(strlen(temp) > 0)
					fputs(temp, stdout);
				fputs("Server seems unreachable.\n", stdout);
				sleep(1);
				fputs("Logging out\n", stdout);
				return 1;
			}
		}

		if(rndm == r) // simulate timeout and server and client out of sync
		{
			r = 11;
			retVal = poll(&fd, 1, TIMEOUT);
			if(retVal == 0)
			{
				fputs("Timeout\n", stdout);
				sleep(1);
				i = 0;
				while(i++ < RETRIES)
				{
					fputs("Retrying. Sending again.\n", stdout);
					n = write(sockFd, sendMsg, MAXLINE);
					retVal = poll(&fd, 1, TIMEOUT);
					if(retVal)
						if((n = read(sockFd, recvMsg, MAXLINE)) == 0)
							printError('t');
					sleep(1);
				}
			}
			else
				if((n = read(sockFd, recvMsg, MAXLINE)) < 0)
					printError('r');
				if(n == 0)
					printError('t');
		}
		else
		{
			if((n = read(sockFd, recvMsg, MAXLINE)) < 0)
				printError('r');
			if(n == 0)
				printError('t');
		}
		printf("\nServer : \n%s", recvMsg);
		stringLower(recvMsg);
		if(strcmp("logout successful.", recvMsg) == 0)
		{
			logout = 1;
			return logout;
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
		//printf("providing credentials to server\n");
		if((n = write(sockFd, sendMsg, MAXLINE)) < 0)
			printError('w');
		if(n == 0)
			printError('t');

		// response from the server
		//printf("response from the server\n");
		if((n = read(sockFd, recvMsg, MAXLINE)) < 0)
			printError('r');
		if(n == 0)
			printError('t');
		
		stringLower(recvMsg);
		printf("\nServer : \n%s\n", recvMsg);
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

	fd.fd = 0;
	fd.events = POLLIN | POLLRDNORM;

	// a random number to simulate timeout
	srand(time(NULL));
	rndm = rand()%10;

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