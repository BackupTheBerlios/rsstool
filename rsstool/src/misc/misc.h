/*
misc.h - miscellaneous functions

Copyright (c) 1999 - 2008 NoisyB
Copyright (c) 2001 - 2005 dbjh


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
#ifndef MISC_H
#define MISC_H


#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif


#if     (defined __unix__ && !defined __MSDOS__) || defined __BEOS__ || \
        defined AMIGA || defined __APPLE__      // Mac OS X actually
// GNU/Linux, Solaris, FreeBSD, OpenBSD, Cygwin, BeOS, Amiga, Mac (OS X)
#define FILE_SEPARATOR '/'  
#define FILE_SEPARATOR_S "/"
#else // DJGPP, Win32
#define FILE_SEPARATOR '\\'  
#define FILE_SEPARATOR_S "\\"
#endif


/*
  tmpnam3()   replacement for tmpnam() temp must have the size of FILENAME_MAX
  bytes_per_second() returns bytes per second (useful in combination with
                    gauge())
  misc_percent()  returns percentage of progress (useful in combination with
                    gauge())
  wait2()         wait (sleep) a specified number of milliseconds
  getenv2()       getenv() clone for enviroments w/o HOME, TMP or TEMP variables
  strptime2()     parse dates with different formats to time_t
*/
extern char *tmpnam3 (char *temp, int dir);
extern int bytes_per_second (time_t start_time, int nbytes);
extern int misc_percent (unsigned long pos, unsigned long len);
extern void wait2 (int nmillis);
extern char *getenv2 (const char *variable);
extern int misc_digits (unsigned long value);
extern time_t strptime2 (const char *s);


/*
  gauge()  progressbar/gauge
             width in characters
             if one color is NULL no color will be used
             color == e.g. ANSI color codes
*/
extern void gauge (int percent, int width, char char_done, char char_todo, const char *color_done,
                   const char *color_todo);


#ifdef __sun
#ifdef __SVR4
#define __solaris__
#endif
#endif


#ifdef  __MSDOS__                               // __MSDOS__ must come before __unix__,
  #define CURRENT_OS_S "MSDOS"                  //  because DJGPP defines both
#elif   defined __unix__
  #ifdef  __CYGWIN__
    #define CURRENT_OS_S "Win32 (Cygwin)"
  #elif   defined __FreeBSD__
    #define CURRENT_OS_S "Unix (FreeBSD)"
  #elif   defined __OpenBSD__
    #define CURRENT_OS_S "Unix (OpenBSD)"
  #elif   defined __linux__
    #define CURRENT_OS_S "Unix (Linux)"
  #elif   defined __solaris__
    #ifdef __sparc__
      #define CURRENT_OS_S "Unix (Solaris/Sparc)"
    #else
      #define CURRENT_OS_S "Unix (Solaris/i386)"
    #endif
  #else
    #define CURRENT_OS_S "Unix"
  #endif
#elif   defined _WIN32
  #ifdef  __MINGW32__
    #define CURRENT_OS_S "Win32 (MinGW)"
  #else
    #define CURRENT_OS_S "Win32 (Visual C++)"
  #endif
#elif   defined __APPLE__
  #if   defined __POWERPC__ || defined __ppc__
    #define CURRENT_OS_S "Apple (PPC)"
  #else
    #define CURRENT_OS_S "Apple"
  #endif
#elif   defined __BEOS__
  #define CURRENT_OS_S "BeOS"
#elif   defined AMIGA
  #if defined __PPC__
    #define CURRENT_OS_S "Amiga (PPC)"
  #else
    #define CURRENT_OS_S "Amiga (68K)"
  #endif
#else
  #define CURRENT_OS_S ""
#endif


#if 0
#ifndef LIB_VERSION
#define LIB_VERSION(major, minor, step) (((major) << 16) | ((minor) << 8) | (step))
#endif
#define RANDOM(min, max) ((rand () % ((max + 1) - min)) + min)
#define OFFSET(a, offset) ((((unsigned char *) &(a)) + (offset))[0])
#define SIZEOF_ARRAY(a) (sizeof(a)/sizeof(a[0]))
#endif


#endif // MISC_H
