/*
  simple libmysql (and unixODBC) wrapper           
  
  Copyright (c) 2006 - 2009 NoisyB
                            
                            
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef MISC_SQL_H
#define MISC_SQL_H
#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#if     (defined USE_ODBC || defined USE_MYSQL)
#ifdef  USE_ODBC
#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>
#endif  // USE_ODBC
#ifdef  USE_MYSQL
#include <mysql/mysql.h>
#endif  // USE_MYSQL


typedef struct
{
  int flags;

#ifdef  USE_ODBC
  SQLHENV env;
  SQLHDBC conn;
  SQLHSTMT hstmt;
  SQLINTEGER err;
  SQLINTEGER id;
  unsigned char buffer[200];
#endif
#ifdef  USE_MYSQL
  MYSQL db;
  MYSQL_RES *res;               /* To be used to fetch information into */
  MYSQL_ROW row;
#endif  // USE_MYSQL

  const char *array[1024][1024];
} st_sql_t;
#endif  // #if     (defined USE_ODBC || defined USE_MYSQL)


/*
  sql_open()   open connection to db

  Flags
    SQL_MYSQL
    SQL_ODBC

  sql_close()  close connection

  sql_read()   read array of rows from db
  sql_getrow() get a single row from db
  sql_write()  send query to db

  sql_stresc() remove possible SQL Injection attacks
                 from a string
*/
#if     (defined USE_ODBC || defined USE_MYSQL)
#define SQL_MYSQL 0
#define SQL_ODBC  1


extern st_sql_t * sql_open (const char *host, int port,
                            const char *user, const char *password,
                            const char *db_name, int flags);

extern const char ***sql_read (st_sql_t *sql, int assoc, int debug);
extern const char **sql_getrow (st_sql_t *sql, int row, int assoc, int debug);
extern int sql_write (st_sql_t *sql, const char *sql_query, int unbuffered, int debug);

extern int sql_close (st_sql_t *sql);
//extern int sql_seek (st_sql_t *sql, int row);
//extern char **sql_get_result (st_sql_t *sql);
//extern int sql_get_rows (st_sql_t *sql);

#endif  // #if     (defined USE_ODBC || defined USE_MYSQL)


extern char *sql_stresc (char *s);


#endif  // MISC_SQL_H
