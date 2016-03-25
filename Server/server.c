// server

#include<stdio.h>
#include<stdlib.h>
#include<netdb.h>
#include<netinet/in.h>
#include<string.h>
#include<unistd.h>
#include<mysql.h>
#include<my_global.h>

#define MAXLINE 500
#define LISTENQ 5

void printError(char c)
{
	/**Printing error messages for socket functions.**/
	switch(c)
	{
		case 'o':
					perror("ERROR. Opening socket.");
					break;
		case 'b':
					perror("ERROR. Binding socket.");
					break;
		case 'a':
					perror("ERROR. Accepting socket.");
					break;
		case 'f':
					perror("ERROR. Fork a child.");
					break;
		case 'r':
					perror("ERROR. Reading from socket.");
					break;
		case 'w':
					perror("ERROR. Writing on socket.");
					break;
		default:
					perror("ERROR. Unkown.");
	}		
	exit(1);
}

void regis(int connFd, MYSQL *sql)
{
	char sendMsg[MAXLINE], recvMsg[MAXLINE], errorMsg[MAXLINE], id[20], password[8];
	char regMsg[] = "\nEnter desired ID and Password. ID should be of atleast 5 and atmost 20 characters\n" 
					"and password should be atleast 4 and atmost 8 characaters.\n" 
					"Enter as ID<space>Password";
	int n, i, count;
	int idError = 0, passwordError = 1;

	strcpy(sendMsg, regMsg);
	n = write(connFd, sendMsg, MAXLINE);
	if(n < 0)
		printError('w');
	while(1)
	{
		bzero(errorMsg, MAXLINE);
		bzero(sendMsg, MAXLINE);
		bzero(recvMsg, MAXLINE);

		n = read(connFd, recvMsg, MAXLINE);
		if(n < 0)
			printError('r');
		
		count = 0;
		for(i = 0; recvMsg[i] != '\0' && recvMsg[i] != ' '; ++i)
		{
			count++;
			if(count > 20)
			{
				strcat(errorMsg, "ID is too long. ");
				idError = 1;
				break;
			}
			id[i] = recvMsg[i];
		}
		if(count < 5)
		{
			strcat(errorMsg, "ID is too short. ");
			idError = 1;
		}
		i++;
		count = 0;
		for( ; recvMsg[i] != '\0'; ++i)
		{
			count++;
			if(count > 8)
			{
				strcat(errorMsg, "Password is too long. ");
				passwordError = 1;
				break;
			}
			password[i] = recvMsg[i];
		}
		if(count < 4)
		{
			strcat(errorMsg, "Password is too short. ");
			passwordError = 1;
		}
		if(idError == 0 && passwordError == 0)
			break;
		else
		{
			strcat(errorMsg, "Enter credentials again.");
			strcpy(sendMsg, errorMsg);
			n = write(connFd, sendMsg, MAXLINE);
			if(n < 0)
				printError('w');
		}
	}
	char *query = "INSERT INTO USER VALUES (" + id + "," + password + ")";
	if(mysql_query(sql, query))
	{
		fprintf(stderr, "%s\n", mysql_error(sql);
		mysql_close(sql);
		exit(1);
	}


//	logIn(connFd);
}


void logIn(int connFd, MYSQL *sql)
{
	char sendMsg[MAXLINE], recvMsg[MAXLINE];
	char logMsg[] = "Enter your ID and Password for Log In.(ID -- Password)";
	int n;
	
	strcpy(sendMsg, logMsg);
	n = write(connFd, sendMsg, MAXLINE);
	if(n < 0)
		printError('w');
	
	n = read(connFd, recvMsg, MAXLINE);
	if(n < 0)
		printError('r');



}

void startServer(int connFd)
{
	char buffer[MAXLINE], sendMsg[MAXLINE], recvMsg[MAXLINE];
	char start[] = "Do you want to Register or Log In.";
	char regSucc[] = "Registered successfully.";
	char logSucc[] = "Successfully Logged In.";
	char logOut[] = "Successfully Logged Out.";
	int n;
	
	MYSQL *sql = mysql_init(NULL);
	if(sql == NULL)
	{
		fprintf(stderr, "%s\n", mysql_error(sql));
		exit(1);
	}
	if(mysql_real_connect(sql, "localhost", "root", "warmachineROXXX", NULL, 0, NULL, 0) == NULL)
	{
		fprintf(stderr, "%s\n", mysql_error(sql));
		mysql_close(sql);
		exit(1);
	}
	if(mysql_query(sql, "USE DATABASE USER"))
	{
		fprintf(stderr, "%s\n", mysql_error(sql));
		mysql_close(sql);
		exit(1);	
	}

	strcpy(sendMsg, start);
	n = write(connFd, sendMsg, MAXLINE);
	if(n < 0)
		printError('w');
	
	while(1)
	{
		n = read(connFd, recvMsg, MAXLINE);
		if(n < 0)
			printError('r');
		if(strcmp("register", tolower(recvMsg)) == 0)
		{
			regis(connFd, sql);
		}
		else
			if(strcmp("log in", tolower(recvMsg)) == 0 || strcmp("login", tolower(recvMsg)) == 0)
			{
				logIn(connFd, sql);
			}
	}
	mysql_close(sql);
}

int main(int argc, char const *argv[])
{
	int listenFd, connFd, port = 9876, pId;
	struct sockaddr_in servAddr, cliAddr;
	printf("Server is up and running ...\n");
	
	// opening the socket
	if((listenFd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		printError('s');
	
	// setting socket address structure
	bzero(&servAddr, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = INADDR_ANY;
	servAddr.sin_port = htons(port);
	
	// binding the server to the port
	if(bind(listenFd, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0)
		printError('b')
	
	// listening on the port
	listen(listenFd, LISTENQ);
	unsigned int clilen = sizeof(cliAddr);
	
	while(true)
	{
		// accept a client
		connFd = accept(listenFd, (struct sockaddr*)&cliAddr, &clilen);
		if(connFd < 0)
			printError('a');
		
		// fork a child process
		pId = fork();
		if(pId < 0)
			printError('f');
		
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