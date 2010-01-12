/*
getopt2.h - getopt() extension
              new structure for defining options, build-in CGI support

Copyright (c) 2004 - 2009 NoisyB


This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef MISC_GETOPT2_H
#define MISC_GETOPT2_H
#ifdef  __cplusplus
extern "C" {
#endif
#include "getopt.h"  // struct option


// NEVER change these
#define OPTION '-'
#define OPTION_S "-"
#define OPTION_LONG_S "--"
#define OPTARG '='
#define OPTARG_S "="
#define QUERY_START '?'
#define QUERY_SEPARATOR '&'


/*
  getopt2_get_option()  find st_getopt2_t in array by val (option integer id)
  getopt2_usage()       render usage output from st_getopt2_t array to stdout
TODO:  getopt2_cgi()         explode QUERY_STRING into argc and argv[]
                          returns argc
TODO:  getopt2_cgi_ui()      render HTML form from st_getopt2_t


  getopt2_long()       turn st_getopt2_t into struct option for getopt_long()
  getopt2_long_only()  turn st_getopt2_t into struct option for getopt_long_only()
  getopt2_short()      turn st_getopt2_t into short options string for getopt_*()


  OPTION           option marker (default: '-')
  OPTION_S         option marker as string (default: "-")
  OPTION_LONG_S    long option marker as string (default: "--")
  OPTARG           optarg separator (default: '=')
  OPTARG_S         optarg separator as string (default: "=")
  QUERY_START      for getopt2_cgi()
  QUERY_SEPARATOR  for getopt2_cgi()
*/
typedef struct
{
  char *name;           // option name
  int has_arg;                // has_arg == 0 no arg
                              // has_arg == 1 reqired
                              // has_arg == 2 optional
  int *flag;
  int val;                    // option integer id
  char *arg_name;       // name of the options arg as it should be
                              // displayed in the --help output
                              // "--name=arg_name" if has_arg == 1
                              // "--name[=arg_name]" if has_arg == 2
  char *help;           // --help, -h, -? output/description for the current option
} st_getopt2_t;


extern const st_getopt2_t *getopt2_get_option (const st_getopt2_t *option, int val);
extern void getopt2_usage (const st_getopt2_t *option);
//extern int getopt2_cgi (char **argv, const st_getopt2_t *option, int args_max);
//extern const char *getopt_cgi_ui (const st_getopt2_t *option);

extern int getopt2_long (struct option *long_option, const st_getopt2_t *option, int args_max);
extern int getopt2_long_only (struct option *long_option, const st_getopt2_t *option, int args_max);
extern int getopt2_short (char *short_option, const st_getopt2_t *option, int args_max);


#ifdef  __cplusplus
}
#endif
#endif // MISC_GETOPT2_H
