#include "mysql.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

/* initiator is the argument. target will contain result of the lookup */

int tableLookup (char* target, u_int* tgtPort, char* initiator, u_int initPort, u_int* nextHop)
{

   MYSQL *conn;
   MYSQL_RES *res;
   MYSQL_ROW row;
   char *server = "localhost";
   char *user = "root";
   char *password = "nu5cdl32"; /* set me first */
   char *database = "router";
   char Query[120];

   conn = mysql_init(NULL);
   /* Connect to database */
   if (!mysql_real_connect(conn, server,
         user, password, database, 0, NULL, 0)) {
      fprintf(stderr, "%s\n", mysql_error(conn));
      return -1;
   }
   /* send SQL query */

   sprintf(Query, "select dst_ip, dst_port, next_hop from lookup where src_ip = '%s' and src_port = '%d'", initiator, initPort);
   printf("%s\n", Query);
   if (mysql_query(conn, Query)) 
   {
      fprintf(stderr, "%s\n", mysql_error(conn));
      return -1;
   }
   res = mysql_use_result(conn);
   while ((row = mysql_fetch_row(res)) != NULL)
   {
      printf("Aici 1\n");
      sprintf(target, "%s", row[0]);
      *tgtPort = atoi((char*)row[1]);
      *nextHop = atoi((char*)row[2]);
      mysql_free_result(res);
      mysql_close(conn);
      return 1;
    }

    sprintf(Query, "select dst_ip, dst_port, next_hop from lookup where src_ip = '%s'", initiator);
    printf("%s\n", Query);
    if (mysql_query(conn, Query))
    {
       fprintf(stderr, "%s\n", mysql_error(conn));
       return -1;
    }
    res = mysql_use_result(conn);
    while ((row = mysql_fetch_row(res)) != NULL)
    {
       printf("Aici 2\n");
       sprintf(target, "%s", row[0]);
       *tgtPort = atoi((char*)row[1]);
       *nextHop = atoi((char*)row[2]);
       mysql_free_result(res);
       mysql_close(conn);
       return 2;
    }  

    mysql_free_result(res);
    mysql_close(conn);
    return 0;
    
}

int main()
{
	//int tableLookup (char* target, u_int* tgtPort, char* initiator, u_int initPort, u_int* nextHop)
	char targetAddr[16];
	u_int tgtPort, nextHop;
	//printf("Sizeof u_init: %d\n", sizeof(u_int));
	tableLookup(targetAddr, &tgtPort, "10.55.0.1", 1, &nextHop);
	printf("tgt: %s tgtPort: %d localPort: %d\n", targetAddr, tgtPort, nextHop);

}
