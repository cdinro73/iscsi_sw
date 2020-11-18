#include "mysql.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

/* initiator is the argument. target will contain result of the lookup */

int tableLookup (char* target, u_int* tgtPort, char* initiator, u_int initPort, char* nexthop_ip, u_int* nexthop_port)
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

   sprintf(Query, "select dst_ip, dst_port, nexthop_ip, nexthop_port from lookup where src_ip = '%s' and src_port = '%d'", initiator, initPort);
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
      sprintf(nexthop_ip, "%s", row[2]);
      *nexthop_port = atoi((char*)row[3]);
      mysql_free_result(res);
      mysql_close(conn);
      return 1;
    }

    sprintf(Query, "select dst_ip, dst_port, nexthop_ip, nexthop_port from lookup where src_ip = '%s'", initiator);
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
       sprintf(nexthop_ip, "%s", row[2]);
       *nexthop_port = atoi((char*)row[3]);
       mysql_free_result(res);
       mysql_close(conn);
       return 2;
    }  

    mysql_free_result(res);
    mysql_close(conn);
    return 0;
    
}

