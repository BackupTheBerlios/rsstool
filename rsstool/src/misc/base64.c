/*
base64.c - base64 codec

Copyright (c) 2006 NoisyB


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
#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "base64.h"


#define FILLCHAR '='

//                 00000000001111111111222222
//                 01234567890123456789012345
const char *cvt = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
//                 22223333333333444444444455
//                 67890123456789012345678901
  "abcdefghijklmnopqrstuvwxyz"
//                 555555556666
//                 234567890123
  "0123456789+/";

char *
base64_enc (char *src, int add_linefeeds)
{
  static unsigned char
    alphabet[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  unsigned int
    bits;
  int
    i = 0;
  int
    j = 0;
  int
    k;
  int
    len;
  char *
    dst;

  len = strlen (src);
  dst = (char *) malloc (((((len - 1) / 3) + 1) * 4) + 1 + len / 54);

  while (i < len)
    {
      if (add_linefeeds)
        if (i && i % 54 == 0)
          dst[j++] = '\n';

      bits = src[i++];
      for (k = 0; k < 2; k++)
        {
          bits <<= 8;
          if (i < len)
            bits += src[i++];
        }

      dst[j++] = alphabet[bits >> 18];
      dst[j++] = alphabet[(bits >> 12) & 0x3f];
      dst[j++] = alphabet[(bits >> 6) & 0x3f];
      dst[j++] = alphabet[bits & 0x3f];
    }

  switch (len % 3)
    {
    case 1:
      dst[j - 2] = '=';
    case 2:
      dst[j - 1] = '=';
      break;
    }
  dst[j] = 0;

  return dst;
}


static const char cd64[]="|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW$$$$$$XYZ[\\]^_`abcdefghijklmnopq";
void decodeblock( unsigned char in[4], unsigned char out[3] )
{   
  out[ 0 ] = (unsigned char ) (in[0] << 2 | in[1] >> 4);
  out[ 1 ] = (unsigned char ) (in[1] << 4 | in[2] >> 2);
  out[ 2 ] = (unsigned char ) (((in[2] << 6) & 0xc0) | in[3]);
}
            
            
void
base64_dec (char *ret, char *src, int length)
{
  unsigned char in[4], out[3], v;
  int i, len;
  int src_offset = 0;
  int ret_offset = 0;

    while( src_offset < length ) {
          for (len = 0, i = 0; i < 4 && src_offset < length; i++)
        {
          v = 0;
          while (src_offset < length && v == 0)
            {
              v = (unsigned char) *(src + src_offset);
              src_offset++;
              v = (unsigned char) ((v < 43 || v > 122) ? 0 : cd64[v - 43]);
              if (v)
                {
                  v = (unsigned char) ((v == '$') ? 0 : v - 61);
                }
            }
          if (src_offset < length)
            {
              len++;
              if (v)
                {
                  in[i] = (unsigned char) (v - 1);
                }
            }
          else
            {
              in[i] = 0;
            }
        }
      if (len)
        {
          decodeblock (in, out);
          for (i = 0; i < len - 1; i++)
            {
              *(ret + ret_offset) = out[i];
              ret_offset++;
            }
        }
}
}
