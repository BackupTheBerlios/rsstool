/*
file.h - miscellaneous file functions

Copyright (c) 1999 - 2004 NoisyB
Copyright (c) 2001 - 2004 dbjh
Copyright (c) 2002 - 2004 Jan-Erik Karlsson (Amiga)


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
#ifndef MISC_FILE_H
#define MISC_FILE_H


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
  Miscellaneous file operations

  isfname()   test if char could be used for filenames
  tofname()   replaces char that can not be used for filenames
  set_suffix() set/replace suffix of filename with suffix
                suffix means in this case the suffix INCLUDING the dot '.'
  get_suffix() get suffix of filename
  filesize()   returns size of a file in bytes

  basename2() basename() replacement
  dirname2()  dirname() replacement
  realpath2() realpath() replacement
  same_file() returns 1 if two filenames refer to one file, otherwise it
                returns 0
  same_fs() returns 1 if two filenames refer to files on one file-system,
              otherwise it returns 0

  rename2()   renames oldname to newname even if oldname and newname are not
                on one file system
  truncate2() don't use truncate() to enlarge files, because the result is
                undefined (by POSIX) use truncate2() instead which does both
  fcopy()     copy src from start for len to dest with mode

  file_get_contents()    does fopen(), malloc(), fread() in one step and returns pointer to
                data (must be free()'d) or NULL if anything failed
  mkdir2()    like mkdir but creates directories recursively
*/
extern int isfname (int c);
extern int tofname (int c);
extern const char *get_suffix (const char *filename);
extern char *set_suffix (char *filename, const char *suffix);
extern int filesize (const char *filename);

extern char *realpath2 (const char *path, char *full_path);
extern char *dirname2 (const char *path, char *dir);
extern const char *basename2 (const char *path);
extern int same_file (const char *filename1, const char *filename2);
extern int same_fs (const char *filename1, const char *filename2);

extern int rename2 (const char *oldname, const char *newname);
extern int truncate2 (const char *filename, unsigned long size);
extern int fcopy (const char *src, size_t start, size_t len, const char *dest,
                  const char *dest_mode);

extern unsigned char *file_get_contents (const char *filename, int maxlength);
extern int mkdir2 (const char *path, int mode);


#endif // MISC_FILE_H
