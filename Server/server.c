// server

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<netdb.h>
#include<netinet/in.h>
#include<unistd.h>
#include<mysql.h>
#include<my_global.h>
#include<time.h>
#include"myheader.h"

#define MAXLINE 500
#define LISTENQ 5
#define ID 20
#define PASSWORD 8
#define EQUAL -10

int loggedIn[LISTENQ]; // contains the generated ids of logged in clients
int log_size;

void get(int connFd, MYSQL *sql)
{
	/**
		GET command from Client.
	**/
	char sendMsg[MAXLINE], recvMsg[MAXLINE];
	int n;


}

void put(int connFd, MYSQL *sql)
{
	/**
		PUT command from Client.
	**/
	char sendMsg[MAXLINE], recvMsg[MAXLINE];
	int n;


}

void del(int connFd, MYSQL *sql)
{
	/**
		DELETE command from Client.
	**/
	char sendMsg[MAXLINE], recvMsg[MAXLINE];
	int n;


}

int verification(MYSQL *sql, char id[], char password[])
{
	/**
		Verifies the id and password in the USER database.
		In case of Register, only id is matched to see if it's
		already taken.
		In case of Log In, password is also matched to check if 
		credentials are correct.
		Return - 1, if the id exists
				 0, otherwise
	**/
	printf("In checkLogIn\n");
	char query[] = "SELECT * FROM USER";
	int exist = 0;
	char tempId[ID], c; 
	c = (password == NULL)? 'r' : 'l';
	
	if(mysql_query(sql, query))
	{
		fprintf(stderr, "%s\n", mysql_error(sql));
		mysql_close(sql);
		exit(1);
	}

	MYSQL_RES *result = mysql_store_result(sql);
	if(result == NULL)
	{
		fprintf(stderr, "%s\n", mysql_error(sql));
		mysql_close(sql);
		exit(1);
	}
	
	MYSQL_ROW row;
	printf("fetching result\n");
	while(row = mysql_fetch_row(result))
	{
		strcpy(tempId, row[0]);
		stringLower(tempId);
		if(strcmp(id, tempId) == 0)
		{
			if(c == 'r')
			{
				exist = 1;
				break;
			}
			else
				if(c == 'l')
					if(strcmp(password, row[1]) == 0)
					{
						exist = 1;
						break;
					}
				else
					printf("Wrong Option.\n");
		}
	}

	mysql_free_result(result);
	return exist;
}

void formatCheck(char recvMsg[], char errorMsg[], char id[], char password[], int *idError, int *passwordError)
{
	/**
		Check whether the entered id and password are in the correct format.
		idError and passwordError are used to indicate if there is an error.
	**/
	int i, k, count = 0;
	for(i = 0; recvMsg[i] != ' '; ++i)
	{
		count++;
		if(count > 20)
		{
			*idError = 1;
			strcat(errorMsg, "ID error. ");
			break;
		}
		id[i] = recvMsg[i];
	}
	if(count < 5 && !(*idError))
	{
		*idError = 1;
		strcat(errorMsg, "ID error. ");
	}
	id[i] = '\0';
	printf("Id : %s\n", id);
	count = 0;
	k = 0;
	printf("i : %d\n", i);
	i++;
	printf("i : %d\n", i);
	for( ; recvMsg[i] != '\n'; ++i)
	{
		count++;
		if(count > 8 || recvMsg[i] == ' ')
		{
			*passwordError = 1;
			strcat(errorMsg, "Password error. Check format. ");
			break;
		}
		password[k++] = recvMsg[i];
	}
	if(count < 4 && !(*passwordError))
	{
		*passwordError = 1;
		strcat(errorMsg, "Password error. ");
	}
	password[k] = '\0';
	printf("Password : %s\n", password);
}


int logIn(int connFd, MYSQL *sql)
{
	/**
		Logs in user.
		Return - 1, when user logs out.
	**/
	printf("In logIn\n.");
	char sendMsg[MAXLINE], recvMsg[MAXLINE], errorMsg[MAXLINE] = ""; 
	char id[ID], password[PASSWORD];
	char tempId[ID];
	char logMsg[] = "Enter ID and Password to log in.\nID<space>Password.";
	char query[] = "SELECT * FROM USER";
	int n, i, k, count, logId, exist;
	int idError, passwordError;
	int logout = 0;

	while(1)
	{
		exist = 0;
		idError = 0;
		passwordError = 0;
		bzero(&recvMsg, MAXLINE);
		bzero(&sendMsg, MAXLINE);
		strcpy(sendMsg, logMsg);
		
		// ask ID and password from user to log in
		printf("ask ID and password from user to log in\n");
		n = write(connFd, sendMsg, MAXLINE);
		if(n < 0)
			printError('w');

		// waiting for credentials
		printf("waiting for credentials.\n");
		n = read(connFd, recvMsg, MAXLINE);
		if(n < 0)
			printError('w');
		printf("%s\n", recvMsg);

		// username and password format verification
		formatCheck(recvMsg, errorMsg, id, password, &idError, &passwordError);
		strcpy(tempId, id);
		stringLower(tempId);

		if(idError == 0 && passwordError == 0)
		{
			printf("AlL OK\n");
			exist = verification(sql, tempId, password);
			if(exist)
			{
				char welcome[MAXLINE] = "Welcome ";
				strcat(welcome, id);
				strcpy(sendMsg, welcome);
				printf("%s\n", sendMsg);
				
				// sending welcome message
				if((n = write(connFd, sendMsg, MAXLINE)) < 0)
					printError('w');
				//srand(time(NULL));
				//loggedIn[++log_size] = rand();
				break;
			}
			else
			{
				strcpy(errorMsg, "You need to register.");
				n = write(connFd, errorMsg, MAXLINE);
				if(n < 0)
					printError('w');
				break;
			}
		}
		else
		{
			strcpy(sendMsg, errorMsg);
			if((n = write(connFd, sendMsg, MAXLINE)) < 0)
				printError('w');
			break;
		}
	}

	while(exist)
	{
		char options[] = "Here are your options.\nGET\nPUT\nDELETE\nLOGOUT.";
		strcpy(sendMsg, options);
		if((n = write(connFd, sendMsg, MAXLINE)) < 0)
			printError('w');

		bzero(&recvMsg, MAXLINE);
		if((n = read(connFd, recvMsg, MAXLINE)) < 0)
			printError('r');

		printf("%s", recvMsg);
		stringLower(recvMsg);

		if(strcmp("get", recvMsg) == EQUAL)
		{
			get(connFd, sql);
		}
		else
			if(strcmp("put", recvMsg) == EQUAL)
			{
				put(connFd, sql);
			}
			else
				if(strcmp("delete", recvMsg) == EQUAL)
				{
					del(connFd, sql);
				}
				else
					if(strcmp("logout", recvMsg) == EQUAL)
					{
						logout = 1;
						printf("Logout successful.\n");
						strcpy(sendMsg, "Logout successful.");
						if((n = write(connFd, sendMsg, MAXLINE)) < 0)
							printError('w');
						return logout;
					}
					else
					{
						printf("Wrong option.\n");
						strcpy(sendMsg, "Wrong option.\n");
						if((n = write(connFd, sendMsg, MAXLINE)) < 0)
							printError('w');
					}
	}
}

void regis(int connFd, MYSQL *sql)
{
	/**
		Registers a new User.
	**/
	printf("Entered into register.\n");
	char sendMsg[MAXLINE], recvMsg[MAXLINE], errorMsg[MAXLINE] = "";
	char id[ID], password[PASSWORD];
	char regMsg[] = "Enter the desired ID and Password.\nID should be between 5 and 20 characters.\nPassword should be between 4 and 8 characters.\nEnter as ID<space>Password.\n";
	char success[] = "Registered successfully";
	char query[MAXLINE] = "INSERT INTO USER VALUES (\'";
	int n, i, k, count;
	int idError, passwordError, exist = 0;

	while(1)
	{
		idError = 0;
		passwordError = 0;
		bzero(&sendMsg, MAXLINE);
		bzero(&recvMsg, MAXLINE);
		strcpy(sendMsg, regMsg);
		
		// ask for credentials
		printf("ask for credentials\n");
		n = write(connFd, sendMsg, MAXLINE);
		if(n < 0)
			printError('w');
		
		// reading credentials from client
		printf("reading credentials from client\n");
		n = read(connFd, recvMsg, MAXLINE);
		if(n < 0)
			printError('r');
		printf("\nClient : \n%s\n", recvMsg);

		// username and password format validation
		formatCheck(recvMsg, errorMsg, id, password, &idError, &passwordError);
		stringLower(id);

		if(idError == 0 && passwordError == 0)
		{
			printf("All OK\n");
			exist = verification(sql, id, NULL);
			if(!exist)
				break;
			else
			{
				strcpy(errorMsg, "ID already exists. Choose a different one.");
				printf("Same id message sent.");
				n = write(connFd, errorMsg, MAXLINE);
				if(n < 0)
					printError('w');
			}
		}
		else
		{
			strcat(errorMsg, "Enter credentials again.\n");
			printf("Error message sent.\n");
			n = write(connFd, errorMsg, MAXLINE);
			if(n < 0)
				printError('w');
		}
	}

	strcat(query, id);
	strcat(query, "\', \'");
	strcat(query, password);
	strcat(query, "\')");
	printf("Query : %s\n", query);

	if(mysql_query(sql, query))
	{
		fprintf(stderr, "%s\n", mysql_error(sql));
		mysql_close(sql);
		exit(1);
	}

	strcpy(sendMsg, success);
	printf("success\n");
	n = write(connFd, sendMsg, MAXLINE);
	if(n < 0)
		printError('w');
}

void startServer(int connFd)
{
	/**
		Start the server side operations.
	**/
	printf("In startServer.\n");
	char sendMsg[MAXLINE], recvMsg[MAXLINE];
	char start[] = "Do you want to Register or Log In?";
	char regSucc[] = "Registered successfully.";
	char logSucc[] = "Successfully Logged In.";
	char logOut[] = "Successfully Logged Out.";
	int n, logout = 0;
	
	// connect mysql
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
	if(mysql_query(sql, "USE SERVER"))
	{
		fprintf(stderr, "%s\n", mysql_error(sql));
		mysql_close(sql);
		exit(1);
	}

	while(1)
	{
		// start server
		strcpy(sendMsg, start);
		n = write(connFd, sendMsg, MAXLINE);
		if(n < 0)
			printError('w');

		bzero(&recvMsg, MAXLINE);
		n = read(connFd, recvMsg, MAXLINE);
		if(n < 0)
			printError('r');
		
		printf("%s\n", recvMsg);
		stringLower(recvMsg);
		printf("%s\n", recvMsg);
		
		if(strcmp("register", recvMsg) == EQUAL)
		{
			printf("Client %d wants to register.\n", connFd);
			regis(connFd, sql);
			logout = logIn(connFd, sql);
		}
		else
			if(strcmp("log in", recvMsg) == EQUAL || strcmp("login", recvMsg) == EQUAL)
			{
				printf("Client %d wants to login.\n", connFd);
				logout = logIn(connFd, sql);
			}
			else
			{
				printf("Wrong option.\n");
			}
		if(logout)
			return;
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
		printError('b');
	
	// listening on the port
	listen(listenFd, LISTENQ);
	
	while(1)
	{
		// accept a client
		unsigned int clilen = sizeof(cliAddr);
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