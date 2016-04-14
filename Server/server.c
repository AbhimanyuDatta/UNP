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

int checkLogIn(int connFd, MYSQL *sql, char id[], char password[])
{
	char query[] = "SELECT * FROM USER";
	int found = 0, logId;

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
	int num_fields = mysql_num_fields(sql);
	MYSQL_ROW row;
	printf("fetching result\n");
	while(row = mysql_fetch_row(result))
	{
		if(strcmp(id, row[0]) == 0)
		{
			if(strcmp(password, row[1]) == 0)
			{
				found = 1; // found the user
				break;
			}
		}
	}
	return found;
}


void logIn(int connFd, MYSQL *sql)
{
	/**
		Logs in user.
		Generates connection ids.
	**/
	char sendMsg[MAXLINE], recvMsg[MAXLINE], errorMsg[MAXLINE] = ""; 
	char id[ID], password[PASSWORD];
	char logMsg[] = "Enter ID and Password to log in.\nID<space>Password.";
	char query[] = "SELECT * FROM USER";
	int n, i, k, count, logId, exist = 0;
	int idError, passwordError;

	while(1)
	{
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

		// username verification
		count = 0;
		for(i = 0; recvMsg[i] != ' '; ++i)
		{
			count++;
			if(count > 20)
			{
				idError = 1;
				strcpy(errorMsg, "ID Error. ");
				break;
			}
			id[i] = recvMsg[i];
		}
		if(count < 5)
		{
			idError = 1;
			strcpy(errorMsg, "ID Error. ");
		}
		id[i] = '\0';
		stringLower(id);

		// password verification
		i++;
		count = 0;
		k = 0;
		for( ; recvMsg[i] != '\n'; ++i)
		{
			count++;
			if(count > 8)
			{
				passwordError = 1;
				strcpy(errorMsg, "Password Error.");
				break;
			}
			password[k++] = recvMsg[i];
		}
		passwordError[k] = '\0';
		if(count < 4)
		{
			passwordError = 1;
			strcpy(errorMsg, "Password Error. ");
		}
		if(idError == 0 && passwordError == 0)
		{
			printf("AlL OK\n");
			exist = checkLogIn(connFd, sql, id, password);
			if(exist)
			{
				srand(time(NULL));
				loggedIm[++log_size] = rand();
			}
			else
			{
				strcpy(errorMsg, "You need to log in first.");
				n = write(connFd, errorMsg, MAXLINE);
				if(n < 0)
					printError('w');
				break;
			}
		}

		if(mysql_query(sql, query))
		{
			fprintf(stderr, "%s\n", mysql_error(sql));
			mysql_close(sql);
			exit(1);
		}
	}
}

int checkId(int connFd, MYSQL *sql, char id[])
{
	/** Checks if the id already exists. 
		Returns - 0, if it doesn't exist,
				- 1, otherwise.
	**/
	printf("Check ID.\n");
	int exist = 0;
	char query[] = "Select ID from USER";
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
	int num_fields = mysql_num_fields(result);
	MYSQL_ROW row;
	printf("checking row id\n");
	// fetching all the rows
	while(row = mysql_fetch_row(result))
	{
		// comparing the id with the previouslt entered ids
		if(strcmp(id, row[0]) == 0)
		{
			exist = 1;
			break;
		}
	}
	mysql_free_result(result);
	return exist;
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
		printf("\nclient : \n%s\n", recvMsg);

		// username validation
		count = 0;
		for(i = 0; recvMsg[i] != ' '; ++i)
		{
			count++;
			if(count > 20)
			{
				idError = 1;
				strcpy(errorMsg, "ID Error. ");
				break;
			}
			id[i] = recvMsg[i];
		}
		if(count < 5)
		{
			idError = 1;
			strcat(errorMsg, "ID Error. ");
		}
		id[i] = '\0';
		stringLower(id);
		printf("id : %s\n", id);
		i++;
		count = 0;
		k = 0;

		// password validation
		for( ; recvMsg[i]!= '\n'; ++i)
		{
			count++;
			if(count > 8)
			{
				passwordError = 1;
				strcat(errorMsg, "Password Error. ");
				break;
			}
			password[k++] = recvMsg[i];
		}
		if(count < 4)
		{
			passwordError = 1;
			strcat(errorMsg, "Password Error. ");
		}
		password[k] = '\0';
		printf("password : %s\n", password);
		if(idError == 0 && passwordError == 0)
		{
			printf("All OK\n");
			exist = checkId(connFd, sql, id);
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
	printf("Query : \n%s", query);

	if(mysql_query(sql, query))
	{
		fprintf(stderr, "%s\n", mysql_error(sql));
		mysql_close(sql);
		exit(1);
	}

	strcpy(sendMsg, success);
	printf("success");
	n = write(connFd, sendMsg, MAXLINE);
	if(n < 0)
		printError('w');
}

void startServer(int connFd)
{
	/**
		Start the server side operations.
	**/
	char sendMsg[MAXLINE], recvMsg[MAXLINE];
	char start[] = "Do you want to Register or Log In?";
	char regSucc[] = "Registered successfully.";
	char logSucc[] = "Successfully Logged In.";
	char logOut[] = "Successfully Logged Out.";
	int n;
	
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

	strcpy(sendMsg, start);
	n = write(connFd, sendMsg, MAXLINE);
	if(n < 0)
		printError('w');
	
	//while(1)
	{
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
			logIn(connFd, sql);
		}
		else
			if(strcmp("log in", recvMsg) == EQUAL || strcmp("login", recvMsg) == EQUAL)
			{
				//logIn(connFd, sql);
			}
			else
			{
				printf("Wrong option");
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
		printError('b');
	
	// listening on the port
	listen(listenFd, LISTENQ);
	unsigned int clilen = sizeof(cliAddr);
	
	while(1)
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