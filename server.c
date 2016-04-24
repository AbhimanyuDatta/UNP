// server

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<netdb.h>
#include<netinet/in.h>
#include<unistd.h>
#include<mysql.h>
#include<my_global.h>

#define EQUAL    -10
#define LISTENQ  5
#define SLEEP    5
#define PASSWORD 8
#define ID       20
#define MAXLINE  256


/**************************** Helper Functions ****************************/


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
		case 'e':
					perror("ERROR. Client ended connection.");
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
		case 'w':
					perror("ERROR. Writing on socket.");
					break;
		default:
					perror("ERROR. Unknown.");
	}		
	exit(1);
}

int checkTable(MYSQL *sql, char dbTable[])
{
	/**
		Check if the table name entered by the client is a valid one.
	**/
	printf("In checkTable\n");
	char query[] = "SHOW TABLES;";
	int accept = 0;

	// not allowed to look into USER table
	if(strcmp("USER", dbTable) == 0)
	{
		accept = -1;
		return accept;
	}

	printf("Running query.\n");
	if(mysql_query(sql, query))
	{
		fprintf(stderr, "%s\n", mysql_error(sql));
		mysql_close(sql);
		exit(1);
	}
	printf("Query result.\n");
	MYSQL_RES *result = mysql_store_result(sql);
	if(result == NULL)
	{
		fprintf(stderr, "%s\n", mysql_error(sql));
		mysql_close(sql);
		exit(1);
	}

	MYSQL_ROW row;
	printf("Matching result\n");
	while(row = mysql_fetch_row(result))
	{
		if(strcmp(dbTable, row[0]) == 0)
		{
			accept = 1;
			break;
		}
	}
	printf("return %d\n", accept);
	mysql_free_result(result);
	return accept;
}


int checkDate(char data[])
{
	/**
		Checks the date whether it's in the write format
		Return: 1, if the format is correct
				0, otherwise
	**/
	printf("In dateFormat\n");
	char year[5], month[3], date[3];
	int i, j, k;
	int y, m, d, leap;
	printf("%s\n", data);

	for(i = 0; data[i] != '-'; ++i)
	{
		if(data[i] < 48 || data[i] > 57)
			return 0;	
		year[i] = data[i];
	}
	year[i++] = '\0';
	printf("%s\n", year);
	for(j = 0; data[i] != '-'; ++i)
	{
		if(data[i] < 48 || data[i] > 57)
			return 0;
		month[j++] = data[i];
	}
	month[j] = '\0';
	printf("%s\n", month);
	i++;
	for(k = 0; data[i] != '\0'; ++i)
	{
		if(data[i] < 48 || data[i] > 57)
			return 0;
		date[k++] = data[i];
	}
	date[k] = '\0';
	printf("%s\n", date);
	if((strlen(date) != 2) || (strlen(month) != 2) || (strlen(year) != 4))
		return 0;

	y = atoi(year);
	m = atoi(month);
	d = atoi(date);

	printf("date range check\n");
	if(y < 1900 || y > 2016)
		return 0;
	if(m < 1 || m > 12)
		return 0;
	if(d < 1 || d > 31)
		return 0;

	printf("Leap year check\n");
	if(y % 400 == 0)
		leap = 1;
	else
		if(y % 100 == 0)
			leap = 0;
		else
			if(y % 4 == 0)
				leap = 1;
			else
				leap = 0;
	printf("feb check\n");
	if((leap && m == 2 && d >29) || (!leap && m == 2 && d > 28))
		return 0;
	printf("non 31 days check\n");
	if((m == 4 || m == 6 || m == 9 || m == 11) && d > 30)
		return 0;

	return 1;
}

int checkUser(MYSQL *sql, char id[], char password[])
{
	/**
		Verifies the id and password in the USER database.
		In case of Register, only id is matched to see if it's
		already taken.
		In case of Log In, password is also matched to check if 
		credentials are correct.
		Return: 1, if the id exists
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

void checkFormat(char recvMsg[], char errorMsg[], char id[], char password[], int *idError, int *passwordError)
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



/********************* Main Functionality Procedures **********************/


void get(int connFd, MYSQL *sql, char msg[])
{
	/**
		GET command from Client.
	**/
	printf("In get\n");
	char sendMsg[MAXLINE];
	char dbTable[MAXLINE], id[ID];
	char query[MAXLINE] = "SHOW TABLES;";
	char answer[MAXLINE];
	int n, i, k, accept = 0;

	for(i = 0; msg[i] != ' '; ++i)
		dbTable[i] = msg[i];
	dbTable[i++] = '\0';
	stringUpper(dbTable);
	
	k = 0;
	for( ; msg[i] != '\0'; ++i)
		id[k++] = msg[i];
	id[k] = '\0';
	stringLower(id);
	
	accept = checkTable(sql, dbTable);
	if(accept == -1)
	{
		printf("Permission denied\n");
		strcpy(sendMsg, "PERMISSION DENIED\n");
		if((n = write(connFd, sendMsg, MAXLINE)) < 0)
			printError('w');
		return;
	}
	if(!accept)
	{
		printf("Incorrect table name.\n");
		strcpy(sendMsg, "TABLE NAME ERROR. CHECK TABLE NAME.\n");
		if((n = write(connFd, sendMsg, MAXLINE)) < 0)
			printError('w');
		return;
	}
	// else
	strcpy(query, "SELECT * FROM ");
	strcat(query, dbTable);
	strcat(query, " WHERE ID = \'");
	strcat(query, id);
	strcat(query, "\';");
	printf("Query : %s\n", query);

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
	MYSQL_ROW row = mysql_fetch_row(result);
	if(row == NULL)
		strcpy(answer, "NONEXISTENT\n");
	else
	{
		strcpy(answer, "EXISTS\n");
		strcat(answer, row[1]);
		strcat(answer, "\n");
	}
	printf("Answer : %s\n", answer);
	strcpy(sendMsg, answer);
	if((n = write(connFd, sendMsg, MAXLINE)) < 0)
		printError('w');
	mysql_free_result(result);
	return;
}



void put(int connFd, MYSQL *sql, char msg[])
{
	/**
		PUT command from Client.
	**/
	printf("In put\n");
	char sendMsg[MAXLINE];
	char query[MAXLINE];
	char dbTable[MAXLINE], id[ID], data[MAXLINE];
	char temp[MAXLINE], type, lenOk;
	int n, i, j, k, count, accept = 0, len = 1, exist = 0;

	for(i = 0; msg[i] != ' '; ++i)
		dbTable[i] = msg[i];
	dbTable[i++] = '\0';
	stringUpper(dbTable);
	
	printf("%s\n", dbTable);
	k = 0;
	for( ; msg[i] != ' '; ++i)
		id[k++] = msg[i];
	id[k] = '\0';
	stringLower(id);
	printf("%s\n", id);
	
	k = 0;
	i++;
	for( ; msg[i] != '\0'; ++i)
		data[k++] = msg[i];
	data[k] = '\0';
	printf("%s\n", data);

	accept = checkTable(sql, dbTable);
	if(accept == -1)
	{
		printf("Permission denied\n");
		strcpy(sendMsg, "PERMISSION DENIED\n");
		if((n = write(connFd, sendMsg, MAXLINE)) < 0)
			printError('w');
		return;
	}
	if(!accept)
	{
		printf("Incorrect table name.\n");
		strcpy(sendMsg, "TABLE NAME ERROR. CHECK TABLE NAME.\n");
		if((n = write(connFd, sendMsg, MAXLINE)) < 0)
			printError('w');
		return;
	}

	// else if the id already exists
	strcpy(query, "SELECT * FROM ");
	strcat(query, dbTable);
	strcat(query, " WHERE ID = \'");
	strcat(query, id);
	strcat(query, "\';");

	printf("query : %s\n", query);
	if(mysql_query(sql, query))
	{
		fprintf(stderr, "%s\n", mysql_error(sql));
		mysql_close(sql);
		exit(1);
	}
	printf("query result\n");
	MYSQL_RES *result = mysql_store_result(sql);
	if(result == NULL)
	{
		fprintf(stderr, "%s\n", mysql_error(sql));
		mysql_close(sql);
		exit(1);	
	}

	MYSQL_ROW row;
	printf("matching row\n");
	while(row = mysql_fetch_row(result))
	{
		if(strcmp(id, row[0]) == 0)
		{
			printf("found\n");
			exist = 1;
			break;
		}
	}
	if(exist)
	{
		printf("The value exists\n");
		strcpy(sendMsg, "EXISTS.\nID EXISTS IN THE TABLE.\nDATA MAY DIFFER.");
		if((n = write(connFd, sendMsg, MAXLINE)) < 0)
			printError('w');
		mysql_free_result(result);
		return;
	}

	// else
	strcpy(query, "DESCRIBE ");
	strcat(query, dbTable);
	strcat(query, ";");
	printf("query : %s\n", query);
	if(mysql_query(sql, query))
	{
		fprintf(stderr, "%s\n", mysql_error(sql));
		mysql_close(sql);
		exit(1);
	}
	printf("query result\n");
	result = mysql_store_result(sql);
	if(result == NULL)
	{
		fprintf(stderr, "%s\n", mysql_error(sql));
		mysql_close(sql);
		exit(1);	
	}

	count = 0;
	printf("query matching\n");
	while(row = mysql_fetch_row(result))
	{
		if(count == 0)
		{
			count++;
			continue;
		}
		strcpy(temp, row[1]);
		break;
	} 
	if(temp[0] == 'i' || temp[0] == 'I')
		type = 'i';
	else
		if(temp[0] == 'v' || temp[0] == 'V')
			type = 'v';
		else
			if(temp[0] == 'd' || temp[0] == 'D')
				type = 'd';

	exist = 0;
	lenOk = 0;
	if(type == 'd')
	{
		exist = checkDate(data);
		if(!exist)
		{
			printf("Date format is incorrect.\n");
			strcpy(sendMsg, "DATE FORMAT IS INCORRECT. IT SHOULD BE YYYY-MM-DD");
		}
	}
	else
		if(type != 'd')
		{
			for(j = 0; temp[j] != '\0'; ++j)
				if(temp[j] < 48 || temp[j] > 57)
					temp[j] = ' ';
			len = atoi(temp);
			if(strlen(data) <= 0 && strlen(data) > len)
			{
				lenOk = 0;
				strcpy(sendMsg, "Enter some data upto length ");
				sprintf(temp, "%d", len);
				strcat(sendMsg, temp);
			}
			else
				lenOk = 1;
		}
	
	if(lenOk || exist)
	{
		// INSERT query
		strcpy(query, "INSERT INTO ");
		strcat(query, dbTable);
		strcat(query, " VALUES (\'");
		strcat(query, id);
		strcat(query, "\' , \'");
		strcat(query, data);
		strcat(query, "\');");

		printf("\nquery : %s\n", query);
		printf("writing data\n");
		if(mysql_query(sql, query))
		{
			fprintf(stderr, "%s\n", mysql_error(sql));
			mysql_close(sql);
			exit(1);
		}

		strcpy(sendMsg, "ADDED <");
		strcat(sendMsg, id);
		strcat(sendMsg, ", ");
		strcat(sendMsg, data);
		strcat(sendMsg, "> TO ");
		strcat(sendMsg, dbTable);
		strcat(sendMsg, "\n");
	}

	if((n = write(connFd, sendMsg, MAXLINE)) < 0)
		printError('w');
	mysql_free_result(result);
	return;
}


void del(int connFd, MYSQL *sql, char msg[])
{
	/**
		DELETE command from Client.
	**/
	char sendMsg[MAXLINE];
	char dbTable[MAXLINE], id[ID];
	char query[MAXLINE], table[] = "USER";
	int n, i, k, accept = 0, found = 0;

	bzero(&sendMsg, MAXLINE);
	for(i = 0; msg[i] != ' '; ++i)
		dbTable[i] = msg[i];
	dbTable[i++] = '\0';
	stringUpper(dbTable);
	k = 0;
	for( ;msg[i] != '\0'; ++i)
		id[k++] = msg[i];
	id[k] = '\0';
	stringLower(id);

	if(strcmp(dbTable, table) == 0)
	{
		printf("Not allowed\n");
		strcpy(sendMsg, "PERMISSION DENIED\n");
		if((n = write(connFd, sendMsg, MAXLINE)) < 0)
			printError('w');
		return;
	}

	accept = checkTable(sql, dbTable);
	if(accept == -1)
	{
		printf("Permission denied\n");
		strcpy(sendMsg, "PERMISSION DENIED\n");
		if((n = write(connFd, sendMsg, MAXLINE)) < 0)
			printError('w');
		return;
	}
	if(!accept)
	{
		printf("Incorrect table name\n");
		strcpy(sendMsg, "TABLE NAME ERROR. CHECK TABLE NAME\n");
		if((n = write(connFd, sendMsg, MAXLINE)) < 0)
			printError('w');
		return;
	}

	// else search the data in table
	strcpy(query, "SELECT * FROM ");
	strcat(query, dbTable);
	strcat(query, " WHERE ID = \'");
	strcat(query, id);
	strcat(query, "\';");

	printf("query %s\n", query);

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
	while(row = mysql_fetch_row(result))
	{
		if(strcmp(id, row[0]) == 0)
		{
			found = 1;
			break;
		}
	}

	// if data is not available on the table
	if(!found)
	{
		printf("NONEXISTENT\n");
		strcpy(sendMsg, "NONEXISTENT\n");
		if((n = write(connFd, sendMsg, MAXLINE)) < 0)
			printError('w');
		mysql_free_result(result);
		return;
	}

	// else if data is available
	strcpy(query, "DELETE FROM ");
	strcat(query, dbTable);
	strcat(query, " WHERE ID = \'");
	strcat(query, id);
	strcat(query, "\';");

	printf("query : %s\n", query);

	if(mysql_query(sql, query))
	{
		fprintf(stderr, "%s\n", mysql_error(sql));
		mysql_close(sql);
		exit(1);
	}

	strcpy(sendMsg, "DELETED\n");
	if((n = write(connFd, sendMsg, MAXLINE)) < 0)
		printError('w');

	mysql_free_result(result);
	return;
}


int startCommunication(int connFd, MYSQL *sql, char tempId[])
{
	/**
		Starts communication with the logged in client.
		Return: 1, when clients logs out
	**/
	char options[MAXLINE];
	char command[MAXLINE], operation[ID];
	char tempCommand[MAXLINE];
	char sendMsg[MAXLINE], recvMsg[MAXLINE];
	int n, i, k, logout = 0, countSpace, wrongInput, notReachable, accept;

	strcpy(options, "Here are your options.\nGET  PUT  DELETE  LOGOUT");
	strcpy(sendMsg, options);
	printf("Sending %s\n", sendMsg);
		
	if((n = write(connFd, sendMsg, MAXLINE)) < 0)
		printError('w');

	while(1)
	{	
		bzero(&sendMsg, MAXLINE);
		bzero(&recvMsg, MAXLINE);
		bzero(&command, MAXLINE);
		bzero(&operation, ID);
		
		if((n = read(connFd, recvMsg, MAXLINE)) < 0)
			printError('r');

		printf("%s", recvMsg);
		
		if(strcmp("logout", recvMsg) == EQUAL)
		{
			logout = 1;
			printf("%s logged out successful.\n", tempId);
			strcpy(sendMsg, "Logout successful.");
			if((n = write(connFd, sendMsg, MAXLINE)) < 0)
				printError('w');
			return logout;
		}

		wrongInput = 0;
		countSpace = 0;
		notReachable = 0;

		for(i = 0; recvMsg[i] != ' '; ++i)
		{
			if(recvMsg[i] == '\n')
			{
				notReachable = 1;
				break;
			}
			command[i] = recvMsg[i];
		}
		command[i++] = '\0';
		strcpy(tempCommand, command);
		stringLower(tempCommand);
		printf("%s\n", tempCommand);
		
		if(!notReachable)
		{
			k = 0;
			for( ; recvMsg[i] != '\n'; ++i)
			{	
				if(recvMsg[i] == '\'' || recvMsg[i] == '"')
				{
					wrongInput = 1;
					break;
				}
				operation[k++] = recvMsg[i];
			}
			operation[k] = '\0';
			printf("%s\n", operation);
		}
		if(wrongInput)
		{
			printf("Wrong input\n");
			strcpy(sendMsg, "INCORRECT INPUT FORMAT\n");
			if((n = write(connFd, sendMsg, MAXLINE)) < 0)
				printError('w');
			continue;
		}

		if(strcmp("get", tempCommand) == 0)
		{
			for(i = 0; operation[i] != '\0';++i)
				if(operation[i] == ' ')
					countSpace++;
			if(countSpace == 1)
				get(connFd, sql, operation);
			else
			{
				printf("get wrong format\n");
				strcpy(sendMsg, "WRONG FORMAT : ");
				strcat(sendMsg, command);
				strcat(sendMsg, " ");
				strcat(sendMsg, operation);
				strcat(sendMsg, "\nCORRECT FORMAT : GET TABLENAME ID\n");
				if((n = write(connFd, sendMsg, MAXLINE)) < 0)
					printError('w');
			}
		}
		else
			if(strcmp("put", tempCommand) == 0)
			{
				char *tempTable = (char*)malloc(MAXLINE*sizeof(char));
				for(i = 0; operation[i] != ' '; ++i)
					tempTable[i] = operation[i];
				tempTable[i] = '\0';
				stringUpper(tempTable);
				printf("temptable : %s\n", tempTable);
				accept = checkTable(sql, tempTable);
				if(accept == -1)
				{
					printf("Permission denied\n");
					strcpy(sendMsg, "PERMISSION DENIED\n");
					if((n = write(connFd, sendMsg, MAXLINE)) < 0)
						printError('w');
				}
				else 
					if(!accept)
					{
						printf("Incorrect table name.\n");
						strcpy(sendMsg, "TABLE NAME ERROR. CHECK TABLE NAME.\n");
						if((n = write(connFd, sendMsg, MAXLINE)) < 0)
							printError('w');
					}
					else
					{
						for(i = 0; operation[i] != '\0';++i)
							if(operation[i] == ' ')
								countSpace++;
						if(countSpace == 2 && (strcmp("TELEPHONE", tempTable) == 0 || strcmp("EMAIL", tempTable) == 0 || strcmp("DOB", tempTable) == 0))
							put(connFd, sql, operation);
						else
							if(countSpace >= 2 && strcmp("ADDRESS", tempTable) == 0)
								put(connFd, sql, operation);
							else
							{
								printf("put wrong format\n");
								strcpy(sendMsg, "WRONG FORMAT : ");
								strcat(sendMsg, command);
								strcat(sendMsg, " ");
								strcat(sendMsg, operation);
								strcat(sendMsg, "\nCORRECT FORMAT : PUT TABLENAME ID DATA\n");
								if((n = write(connFd, sendMsg, MAXLINE)) < 0)
									printError('w');
							}
						free(tempTable);
					}
			}
			else
				if(strcmp("delete", tempCommand) == 0)
				{
					for(i = 0; operation[i] != '\0';++i)
						if(operation[i] == ' ')
							countSpace++;
					if(countSpace == 1)
						del(connFd, sql, operation);
					else
					{
						printf("delete wrong format\n");
						strcpy(sendMsg, "WRONG FORMAT : ");
						strcat(sendMsg, command);
						strcat(sendMsg, " ");
						strcat(sendMsg, operation);
						strcat(sendMsg, "\nCORRECT FORMAT : DELETE TABLENAME ID\n");
						if((n = write(connFd, sendMsg, MAXLINE)) < 0)
							printError('w');
					}
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


int logIn(int connFd, MYSQL *sql)
{
	/**
		Logs in the client.
	**/
	printf("In logIn\n.");
	char sendMsg[MAXLINE], recvMsg[MAXLINE], errorMsg[MAXLINE] = ""; 
	char id[ID], password[PASSWORD];
	char tempId[ID];
	char logMsg[] = "Enter ID and Password to log in.\nID<space>Password.";
	char query[] = "SELECT * FROM USER";
	int n, i, logId, exist;
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
		if((n = write(connFd, sendMsg, MAXLINE)) < 0)
			printError('w');

		// waiting for credentials
		printf("waiting for credentials.\n");
		if((n = read(connFd, recvMsg, MAXLINE)) < 0)
			printError('w');
		printf("%s\n", recvMsg);

		// username and password format checking
		checkFormat(recvMsg, errorMsg, id, password, &idError, &passwordError);
		strcpy(tempId, id);
		stringLower(tempId);

		if(idError == 0 && passwordError == 0)
		{
			printf("All OK\n");
			exist = checkUser(sql, tempId, password);
			if(exist)
			{
				char welcome[MAXLINE] = "WELCOME ";
				strcat(welcome, id);
				strcpy(sendMsg, welcome);
				printf("%s\n", sendMsg);
				
				// sending welcome message
				if((n = write(connFd, sendMsg, MAXLINE)) < 0)
					printError('w');

				// start communication with the logged in client
				logout = startCommunication(connFd, sql, tempId);
				return logout;
			}
			// not registered yet
			else
			{
				strcpy(errorMsg, "ERROR. NO SUCH USER. You need to register.");
				if((n = write(connFd, errorMsg, MAXLINE)) < 0)
					printError('w');
				logout = 0;
				return logout;
			}
		}
		// id and/or password error
		else
		{
			strcpy(sendMsg, errorMsg);
			if((n = write(connFd, sendMsg, MAXLINE)) < 0)
				printError('w');
			logout = 0;
			return logout;
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
	char regMsg[] = "Enter the desired ID and Password.\nID should be between 5 and 20 characters. Password should be between 4 and 8 characters.\nEnter as ID<space>Password.\n";
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
		if((n = write(connFd, sendMsg, MAXLINE)) < 0)
			printError('w');
		
		// reading credentials from client
		printf("reading credentials from client\n");
		if((n = read(connFd, recvMsg, MAXLINE)) < 0)
			printError('r');
		printf("\nClient : \n%s\n", recvMsg);

		// username and password format validation
		checkFormat(recvMsg, errorMsg, id, password, &idError, &passwordError);
		stringLower(id);

		if(idError == 0 && passwordError == 0)
		{
			printf("All OK\n");
			exist = checkUser(sql, id, NULL);
			if(!exist)
				break;
			else
			{
				strcpy(errorMsg, "ID already exists. Choose a different one.");
				printf("Same id message sent.");
				if((n = write(connFd, errorMsg, MAXLINE)) < 0)
					printError('w');
			}
		}
		else
		{
			strcat(errorMsg, "Enter credentials again.\n");
			printf("Error message sent.\n");
			if((n = write(connFd, errorMsg, MAXLINE)) < 0)
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
	if((n = write(connFd, sendMsg, MAXLINE)) < 0)
		printError('w');
}


void startServer(int connFd)
{
	/**
		Start the server side operations.
	**/
	printf("In startServer.\n");
	char sendMsg[MAXLINE], recvMsg[MAXLINE], temp[MAXLINE];
	char start[] = "Do you want to Register or Log In?";
	int n, i, logout = 0;
	
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
		//sleep(SLEEP);
		if((n = write(connFd, sendMsg, MAXLINE))<0)
			printError('w');

		bzero(&recvMsg, MAXLINE);
		
		//sleep(SLEEP);
		if((n = read(connFd, recvMsg, MAXLINE)) < 0)
			printError('r');
		//while((n = read(connFd, temp, MAXLINE)) > 0);
		
		printf("%s\n", recvMsg);
		stringLower(recvMsg);
		printf("%s\n", recvMsg);
		
		if(strcmp("register", recvMsg) == EQUAL)
		{
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
		{
			mysql_close(sql);
			return;
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

/*	for(int i = 0; i <LISTENQ; ++i)
		strcpy(client[i], "");
*/	
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