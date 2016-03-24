// database connect

#include<mysql.h>
#include<my_global.h>

int main(int argc, char const *argv[])
{
	MYSQL *conn = mysql_init(NULL);
	if(conn == NULL)
	{
		fprintf(stderr, "%s\n", mysql_error(conn));
		exit(1);
	}
	if(mysql_real_connect(conn, "localhost", "root", "warmachineROXXX", NULL, 0, NULL, 0) == NULL)
	{
		fprintf(stderr, "%s\n", mysql_error(conn));
		mysql_close(conn);
		exit(1);
	}
	if(mysql_query(conn, "CREATE DATABASE testdb"))
	{
		fprintf(stderr, "%s\n", mysql_error(conn));
		mysql_close(conn);
		exit(1);
	}
	printf("Connection successfully done.");
	mysql_close(conn);
	return 0;
}
