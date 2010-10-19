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
#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  
#include "string.h"
#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef  USE_MYSQL
#include <mysql/mysql.h>
#endif
#ifdef  USE_ODBC
#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>
#endif
#include "sql.h"


#ifdef  MAXBUFSIZE
#undef MAXBUFSIZE
#endif
#define MAXBUFSIZE 32768


#ifdef  USE_MYSQL
//#include "sql_mysql.c"
static int
sql_free_array (st_sql_t *sql)
{
/*
  int r = 0;

  if (!sql->array)
    return 0;

  for (; sql->array[r]; r++)
    {
      free (sql->array[r]);
      sql->array[r] = NULL;
    }

  free (sql->array);
  sql->array = NULL;
*/
  return 0;
}


static int
sql_malloc_array (st_sql_t *sql, int rows, int cols)
{
/*
  int r = 0;

  if (sql->array)
    sql_free_array (sql);

  sql->array = (const char ***) malloc (rows * sizeof (const char **));
  if (!sql->array)
    return -1;

  for (r = 0; r <= rows; r++)
    {
      sql->array[r] = (const char **) malloc (cols * sizeof (const char *));
      if (!sql->array[r])
        return -1;
    }
*/
  return 0;
}


st_sql_t *
sql_mysql_open (st_sql_t *sql,
                const char *host, int port,
                const char *user, const char *password,
                const char *db_name)
{
  mysql_init (&sql->db);
  mysql_options (&sql->db, MYSQL_READ_DEFAULT_GROUP, "random");

  if (!mysql_real_connect (&sql->db, host, user, password, db_name, port, NULL, 0))
    {
      fprintf (stderr, "ERROR: mysql_real_connect(): %s\n", mysql_error (&sql->db));
      return NULL;
    }

#if 1
  if (mysql_select_db (&sql->db, db_name))  // select the database we want to use
    {
      fprintf (stderr, "ERROR: mysql_select_db(): %s\n", mysql_error (&sql->db));
      return NULL;
    }
#endif

  return sql;
}


int
sql_mysql_close (st_sql_t *sql)
{
  if (sql->res)
    {
      mysql_free_result (sql->res);
      sql->res = NULL;
    }

  mysql_close (&sql->db);
  sql_free_array (sql);

  return 0;
}


const char ***
sql_mysql_read (st_sql_t *sql)
{
  int row = 0, col = 0;
  int rows = 0, cols = 0;

  if (!sql->res)
    return NULL;

  rows = mysql_num_rows (sql->res);
  cols = mysql_num_fields (sql->res);
  if (sql_malloc_array (sql, rows, cols))
    {
      mysql_free_result (sql->res);
      sql->res = NULL;
      return NULL;
    }

  // free and re-malloc the result array in st_sql_t
  sql_free_array (sql);
  if (sql_malloc_array (sql, rows, cols) != 0)
    {
      mysql_free_result (sql->res);
      sql->res = NULL;
      return NULL;
    }

  for (row = 0; row < rows; row++)
    if ((sql->row = mysql_fetch_row (sql->res)))
      for (col = 0; col < cols; col++)
        sql->array[row][col] = sql->row[col];

  return (const char ***) sql->array;
}


const char **
sql_mysql_getrow (st_sql_t *sql, int row)
{
  sql_mysql_read (sql);

  if (sql->array)
    if (mysql_num_rows (sql->res) > (unsigned int) row)
      return sql->array[row];

  return NULL;
}


int
sql_mysql_write (st_sql_t *sql, const char *sql_statement)
{
  if (mysql_real_query (&sql->db, sql_statement, strlen (sql_statement)))
    {
      fprintf (stderr, "ERROR: mysql_real_query(): %s\n", mysql_error (&sql->db));
      return -1;
    }

  sql->res = mysql_store_result (&sql->db);  // download result from server

  return 0;
}

#endif
#ifdef  USE_ODBC
#include "sql_odbc.c"
#endif


#ifdef  USE_MYSQL
char *
sql_stresc (char *s)
{
  return mysql_escape_string (s);
}
#if 0
char *
sql_strrealesc (st_sql_t *sql, char *s)
{
  return mysql_real_escape_string(MYSQL *mysql, d, s, strlen (s));
}
#endif
#elif   defined USE_ODBC
char *
sql_stresc (char *s)
{
#warning TODO: sql_stresc for unixODBC
  return s;
}
#else
char *
sql_stresc (char *s)
{
#if 1
 char *bak = strdup (s);
 char *p = bak;
 char *d = s;

 if (!p)
   return NULL;

 for (; *p; p++)
   switch (*p)
     {
       case 10:  // \n
         strcpy (d, "\\n");
         d = strchr (d, 0);
         break;

       case 13:  // \r
         strcpy (d, "\\r");
         d = strchr (d, 0);
         break;

       case 39:  // single quote is escaped with another single quote
         sprintf (d, "'%c", *p);
         d = strchr (d, 0);
         break;

       case 34:  // quotes
       case 92:  // backslash
         sprintf (d, "\\%c", *p);
         d = strchr (d, 0);
         break;

       default:
//         if (*p > 31 && *p < 127)
           {
             *d = *p;
             *(++d) = 0;
           }
         break;
     }

  free (bak);
#else
  strrep (s, "\n", "\\n");
  strrep (s, "\r", "\\r");
  strrep (s, "\"", "\\\"");
  strrep (s, "\'", "\\\'");
  strrep (s, "\\", "\\");
#endif

  return s;
}
#endif


#if     (defined USE_ODBC || defined USE_MYSQL)
#ifdef  USE_MYSQL
#include "sql_mysql.c"
#endif
#ifdef  USE_ODBC
#include "sql_odbc.c"
#endif


st_sql_t *
sql_open (const char *host, int port,
          const char *user, const char *password,
          const char *db_name, int flags)
{
  static st_sql_t sql;

#ifndef  USE_MYSQL
  if (flags == SQL_MYSQL)
    {
      fprintf (stderr, "ERROR: sql_open(): libmysql support missing\n");
      return NULL;
    }
#endif
#ifndef  USE_ODBC
  if (flags == SQL_ODBC)
    {
      fprintf (stderr, "ERROR: sql_open(): unixODBC support missing\n");
      return NULL;
    } 
#endif

  memset (&sql, 0, sizeof (st_sql_t));
  sql.flags = flags;

#ifdef  USE_MYSQL
  if (flags == SQL_MYSQL)
    return sql_mysql_open (&sql, host, port, user, password, db_name);
#endif
#ifdef  USE_ODBC
  if (flags == SQL_ODBC)
    return sql_odbc_open (&sql, host, port, user, password, db_name);
#endif

  return NULL;
}


const char ***
sql_read (st_sql_t *sql)
{
#ifdef  USE_MYSQL
  if (sql->flags == SQL_MYSQL)
    return sql_mysql_read (sql);
#endif
#ifdef  USE_ODBC
  if (sql->flags == SQL_ODBC)
    return sql_odbc_read (sql);
#endif

  return 0;
}


const char **
sql_getrow (st_sql_t *sql, int row)
{
#ifdef  USE_MYSQL
  if (sql->flags == SQL_MYSQL)
    return sql_mysql_getrow (sql, row);
#endif
#ifdef  USE_ODBC
  if (sql->flags == SQL_ODBC)
    return sql_odbc_getrow (sql, row);
#endif

  return 0;
}


int
sql_write (st_sql_t *sql, const char *sql_statement)
{
#ifdef  USE_MYSQL
  if (sql->flags == SQL_MYSQL)
    return sql_mysql_write (sql, sql_statement);
#endif
#ifdef  USE_ODBC
  if (sql->flags == SQL_ODBC)
    return sql_odbc_write (sql, sql_statement);
#endif

  return 0;
}


int
sql_close (st_sql_t *sql)
{
#ifdef  USE_MYSQL
  if (sql->flags == SQL_MYSQL)
    return sql_mysql_close (sql);
#endif
#ifdef  USE_ODBC
  if (sql->flags == SQL_ODBC)
    return sql_odbc_close (sql);
#endif

  return 0;
}


//#if 1
#ifdef  TEST
int
main (int argc, char *argv[])
{
  int i = 0, j = 0;
  st_sql_t *sql = NULL;
  const char **row = NULL;

  if (!(sql = sql_open ("localhost", 3306, "root", "nb", "mysql", SQL_MYSQL)))
    return -1;

  sql_write (sql, "SELECT * FROM user");
  for (i = 0; (row = (const char **) sql_getrow (sql, i)); i++)
    {
      for (j = 0; row[j]; j++)
        printf ("\"%s\" ", row[j]);
      printf ("\n");
    }
  
  sql_write (sql, "SELECT * FROM user");
  row = (const char **) sql_getrow (sql, 2);
  if (row)
    {
      for (j = 0; row[j]; j++)
        printf ("\"%s\" ", row[j]);
      printf ("\n");
    }
  
  sql_write (sql, "SELECT * FROM user WHERE user = 'root'");
  for (i = 0; (row = (const char **) sql_getrow (sql, i)); i++)
    {
      for (j = 0; row[j]; j++)
        printf ("\"%s\" ", row[j]);
      printf ("\n");
    }
  
  sql_close (sql);

  return 0;
}
#endif  // TEST


#if 0
#if 0
#if     (defined USE_MYSQL || defined USE_ODBC)
static int
rsstool_db_url_validate (st_strurl_t *url)
{
  if (!(*url->request))
    {
      fputs ("You have to specify a database (URL syntax: user:passwd@host:port/database)\n", stderr);
      return -1;
    }

  fputs ("Connecting to ", stderr);

  if (!(*url->user))
    strcpy (url->user, "admin");
  fputs (url->user, stderr);

  if (*url->pass)
    fprintf (stderr, ":%s", url->pass);

  if (!(*url->host))
    strcpy (url->host, "localhost");
  fprintf (stderr, "@%s", url->host);

  if (url->port < 1)
    url->port = 3306; // default
  fprintf (stderr, ":%d%s", url->port, url->request);
 
  fputs (" ... ", stderr);

  return 0;
}
#endif


#ifdef  USE_MYSQL
int
rsstool_write_mysql (st_rsstool_t *rt)
{
  st_sql_t *sql = NULL;
  st_strurl_t url;
  char buf[MAXBUFSIZE];

  strurl (&url, rt->dburl);

  if (rsstool_db_url_validate (&url) == -1)
    return -1;

  strtrim_s (url.request, "/", NULL);

  if (!(sql = sql_open (url.host, url.port, url.request, url.user, url.pass, SQL_MYSQL)))
    {
      fputs ("FAILED\n", stderr);
      return -1;
    }

  fputs ("OK\n", stderr);

  sql_query (sql, "DROP TABLE IF EXISTS rsstool_table");

  while (sql_gets (sql, buf, MAXBUFSIZE))
    {
      fputs (buf, stdout);
      fputc ('\n', stdout);
    }

  sql_close (sql);

  return 0;
}
#endif


#ifdef  USE_ODBC
int
rsstool_write_odbc (st_rsstool_t *rt)
{
  st_sql_t *sql = NULL;
  st_strurl_t url;

  strurl (&url, rt->dburl);

  if (rsstool_db_url_validate (&url) == -1)
    return -1;

  strtrim_s (url.request, "/", NULL);

  if (!(sql = sql_open (url.host, url.port, url.request, url.user, url.pass, SQL_ODBC)))
    {
      fputs ("FAILED\n", stderr);
      return -1;
    }

  fputs ("OK\n", stderr);

  sql_query (sql, "SELECT * FROM user");

  sql_close (sql);

  return 0;
}
#endif
#endif
#endif


#endif  // #if     (defined USE_ODBC || defined USE_MYSQL)
