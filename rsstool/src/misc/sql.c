/*
  simple wrapper for ODBC or libmysql
  
  Copyright (c) 2006 Dirk
                            
                            
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
#include "string.h"
#ifdef  USE_MYSQL
#include "sql_mysql.h"
#endif
#ifdef  USE_ODBC
#include "sql_odbc.h"
#endif
#ifdef  USE_PGSQL 
#include <libpq-fe.h>
#endif
#include "sql.h"


#if     (defined USE_ODBC || defined USE_MYSQL)
int
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


int
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
#endif


char *
sql_stresc (char *s)
{
//#ifdef  USE_MYSQL
//  return mysql_escape_string (s);
////  return mysql_real_escape_string(MYSQL *mysql, d, s, strlen (s));
////#elif   define USE_ODBC
//#else
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

       case 34:  // quotes
       case 39:  // single quotes
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
//#endif

  return s;
}


#if     (defined USE_ODBC || defined USE_MYSQL)
st_sql_t *
sql_open (const char *host, int port,
          const char *user, const char *password,
          const char *db_name, int flags)
{
  static st_sql_t sql;

  if (!(flags & SQL_MYSQL) &&
      !(flags & SQL_ODBC))
    return NULL;

  memset (&sql, 0, sizeof (st_sql_t));

  sql.flags = flags;

#ifdef  USE_MYSQL
  if (flags & SQL_MYSQL)
    return sql_mysql_open (&sql, host, port, user, password, db_name);
#endif
#ifdef  USE_ODBC
  if (flags & SQL_ODBC)
    return sql_odbc_open (&sql, host, port, user, password, db_name);
#endif

  return NULL;
}


const char ***
sql_read (st_sql_t *sql)
{
#ifdef  USE_MYSQL
  if (sql->flags & SQL_MYSQL)
    return sql_mysql_read (sql);
#endif
#ifdef  USE_ODBC
  if (sql->flags & SQL_ODBC)
    return sql_odbc_read (sql);
#endif

  return 0;
}


const char **
sql_getrow (st_sql_t *sql, int row)
{
#ifdef  USE_MYSQL
  if (sql->flags & SQL_MYSQL)
    return sql_mysql_getrow (sql, row);
#endif
#ifdef  USE_ODBC
  if (sql->flags & SQL_ODBC)
    return sql_odbc_getrow (sql, row);
#endif

  return 0;
}


int
sql_write (st_sql_t *sql, const char *sql_statement)
{
#ifdef  USE_MYSQL
  if (sql->flags & SQL_MYSQL)
    return sql_mysql_write (sql, sql_statement);
#endif
#ifdef  USE_ODBC
  if (sql->flags & SQL_ODBC)
    return sql_odbc_write (sql, sql_statement);
#endif

  return 0;
}


int
sql_close (st_sql_t *sql)
{
#ifdef  USE_MYSQL
  if (sql->flags & SQL_MYSQL)
    return sql_mysql_close (sql);
#endif
#ifdef  USE_ODBC
  if (sql->flags & SQL_ODBC)
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
#endif  // #if     (defined USE_ODBC || defined USE_MYSQL)
