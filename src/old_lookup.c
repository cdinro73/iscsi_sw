#include "mysql.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

/* initiator is the argument. target will contain result of the lookup */
int tableLookup (char* target, char* initiator)
{

   MYSQL *conn;
   MYSQL_RES *res;
   MYSQL_ROW row;
   char *server = "localhost";
   char *user = "root";
   char *password = "nu5cdl32"; /* set me first */
   char *database = "router";
   int retval = 0;
   u_int ip, net, mask;

   conn = mysql_init(NULL);
   /* Connect to database */
   if (!mysql_real_connect(conn, server,
         user, password, database, 0, NULL, 0)) {
      fprintf(stderr, "%s\n", mysql_error(conn));
      return -1;
   }
   /* send SQL query */


   if (mysql_query(conn, "select * from lookup order by mask DESC")) {
      fprintf(stderr, "%s\n", mysql_error(conn));
      return -1;
   }
   res = mysql_use_result(conn);
   while ((row = mysql_fetch_row(res)) != NULL)
   {
      inet_pton(AF_INET, initiator, &ip);
      inet_pton(AF_INET, row[0], &net);
      inet_pton(AF_INET, row[1], &mask);
      //printf("%s %s %s %s %d %d %d\n", initiator, row[0], row[1], row[2], ip, net, mask);
      if ((ip & mask) == (net & mask))
      {
	printf("From mysql: %s\n", row[2]);
      	sprintf(target, "%s", row[2]);
      	retval = 1;
	break;
      }
   }
   mysql_free_result(res);
   mysql_close(conn);
   return retval;
}

