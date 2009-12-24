/*
net.h - miscellaneous network functions

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
#ifndef MISC_NET_H
#define MISC_NET_H
#ifdef  __cplusplus
extern "C" {
#endif
#if     (defined USE_TCP || defined USE_UDP)
#ifdef  _WIN32
#include <winsock2.h>
#include <io.h>
#else
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#endif


/*
  Network functions

  net_init()     open TCP or UDP socket
  net_quit()     quit

  Flags
    NET_TCP        use TCP
    NET_CLIENT     use to connect to server
    NET_SERVER     use as server
    NET_UDP        use UDP
    NET_DEBUG      print DEBUG output
    NET_LOCALONLY  allow connections to/from localhost only

  Client (and server)
    net_open()     open connection to a server

    net_read()
    net_write()

    net_getc()
    net_putc()
    net_gets()
    net_puts()

    net_close()    close connection

    net_sync() 

  Server
    net_bind()
    net_listen()
    net_accept()
    net_server()   starts TCP server and runs callback function on connection/request
*/
#define NET_TCP        1
#define NET_CLIENT     (1<<1)
#define NET_SERVER     (1<<2)
#define NET_UDP        (1<<4)
#define NET_DEBUG      (1<<5)
#define NET_LOCALONLY  (1<<6)


typedef struct
{
  int flags;
  int timeout;

  int sock0;
  int socket;
  int port;

  struct sockaddr_in addr;
  struct sockaddr_in udp_addr;
} st_net_t;


extern st_net_t *net_init (int flags, int timeout);
extern int net_quit (st_net_t *n);

// client
extern int net_open (st_net_t *n, const char *address, int port);
extern int net_close (st_net_t *n);

extern int net_read (st_net_t *n, void *buffer, int buffer_len);
extern int net_write (st_net_t *n, void *buffer, int buffer_len);
extern int net_getc (st_net_t *n);
extern int net_putc (st_net_t *n, int c);
extern char *net_gets (st_net_t *n, char *buffer, int buffer_len);
extern int net_puts (st_net_t *n, char *buffer);

//extern int net_sync (st_net_t *n);
extern int net_bind (st_net_t *n, int port);
extern int net_listen (st_net_t *n);
extern st_net_t *net_accept (st_net_t *n);

// server with callback
extern int net_server (st_net_t *n, int port, int (* callback_func) (const void *, int, void *, int *), int max_content_len);


/*
  HTTP header build and read/parse functions

  net_build_http_request()  build http header (for client request)
  net_build_http_response() build http header (for server response) 

  net_get_http_header()     reads http header (from client or server)

  net_parse_http_request()  http header parser (of client request)
  net_parse_http_response() http header parser (of server response)
*/
#define NET_MAXBUFSIZE 1024  
#define NET_MAXHTTPHEADERSIZE (NET_MAXBUFSIZE*16)
enum {
  NET_METHOD_GET = 0,
  NET_METHOD_POST
};
typedef struct
{
  char header[NET_MAXHTTPHEADERSIZE];   // the whole header

//  int method;                         // the method
//  char method_s[NET_MAXBUFSIZE];      // the method as string "GET", "POST", ...

  char host[NET_MAXBUFSIZE];          // "localhost", ...
  char request[NET_MAXBUFSIZE];

  char user_agent[NET_MAXBUFSIZE];    // or "server:"

//  char connection[NET_MAXBUFSIZE];    // "close", "keep-alive"
//  int keep_alive;

  int gzip;                           // compression enabled

  char content_type[NET_MAXBUFSIZE];
//  int content_length;
} st_http_header_t;
extern int net_build_http_request (char *http_header, const char *url_s, const char *user_agent, int keep_alive, int method, int gzip);
extern int net_build_http_response (char *http_header, const char *user_agent, int keep_alive, unsigned int content_len, int gzip);
extern int net_get_http_header (char *http_header, st_net_t *n);
extern int net_parse_http_request (st_http_header_t *h, const char *http_header);
extern int net_parse_http_response (st_http_header_t *h, const char *http_header);


/*
  net_http_get_to_temp()    decide if url_or_fname is a url or fname
                              it will eventually download the file and
                              return the name of a temporary file
                              OR the fname when it was a local file
  Flags
    GET_USE_GZIP  use gzip compression (if compiled)
*/
#define GET_USE_GZIP (1<<1)
// curl is the default (if available) 
#define GET_NO_CURL  (1<<2) 
#define GET_VERBOSE  (1<<3)
extern const char *net_http_get_to_temp (const char *url_s, const char *user_agent, int flags);
                                              

#endif  // (defined USE_TCP || defined USE_UDP)
#ifdef  __cplusplus
}
#endif
#endif  // MISC_NET_H
