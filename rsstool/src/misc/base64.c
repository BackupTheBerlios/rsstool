/*
codec_base64.h - base64 codec

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

#if 0
char *
encode (char *data, int data_len)
{
  int i;
  char c;
  int len = data_len;
  char *ret;

  for (i = 0; i < len; ++i)
    {
      c = (data[i] >> 2) & 0x3f;
      ret += cvt[c];
      c = (data[i] << 4) & 0x3f;
      if (++i < len)
        c |= (data[i] >> 4) & 0x0f;

      ret += cvt[c];
      if (i < len)
        {
          c = (data[i] << 2) & 0x3f;
          if (++i < len)
            c |= (data[i] >> 6) & 0x03;

          ret += cvt[c];
        }
      else
        {
          ++i;
          ret += FILLCHAR;
        }

      if (i < len)
        {
          c = data[i] & 0x3f;
          ret += cvt[c];
        }
      else
        {
          ret += FILLCHAR;
        }
    }

  return (ret);
}

CString
Base64::decode (CString data)
{
  auto int i;
  auto char c;
  auto char c1;
  auto int len = data.length ();
  auto CString ret;

  for (i = 0; i < len; ++i)
    {
      c = (char) cvt.find (data[i]);
      ++i;
      c1 = (char) cvt.find (data[i]);
      c = (c << 2) | ((c1 >> 4) & 0x3);
      ret += c;
      if (++i < len)
        {
          c = data[i];
          if (FILLCHAR == c)
            break;

          c = (char) cvt.find (c);
          c1 = ((c1 << 4) & 0xf0) | ((c >> 2) & 0xf);
          ret += c1;
        }

      if (++i < len)
        {
          c1 = data[i];
          if (FILLCHAR == c1)
            break;

          c1 = (char) cvt.find (c1);
          c = ((c << 6) & 0xc0) | c1;
          ret += c;
        }
    }

  return (ret);
}
#endif


char *
base64_enc (char *src)
{
  static unsigned char alphabet[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  unsigned int bits;
  int i = 0;
  int j = 0;
  int k;
  int len;
  char *dst;

  len = strlen (src);
  dst = malloc (((((len - 1) / 3) + 1) * 4) + 1 + len / 54);

  while (i < len)
    {
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


