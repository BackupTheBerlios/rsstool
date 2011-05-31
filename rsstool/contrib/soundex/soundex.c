// gcc soundex.c -o soundex
#include <stdio.h>
#include <string.h>


const char *
soundex2 (const char *s)
{
  int i;
  const char *k = " 123 12  22455 12623 1 2 2";
  char t = 0;
  int l = strlen (s);
  static char d[32];
  int pos = 0;

  d[pos++] = toupper (s[0]);
  d[pos] = 0;
  for (i = 0; i < l && pos < 5; i++)
    {
      char c = k[toupper (s[i]) - 'A'];
      if (c == ' ')
        continue;
      if (c != t)
        {
          if (i > 0)
            {
              d[pos++] = c;
              d[pos] = 0;
            }   
          t = c;
        }
    }
  strcpy (&d[pos], "000");
  d[4] = 0; 

  // DEBUG
//  printf ("%d %s\n", pos, d);

  return d;
}


int
main (int argc, char *argv)
{
  printf ("%s", soundex2 ("gwmcglmumh"));
  return 0;
}

