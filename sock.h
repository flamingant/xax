#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#ifndef __sock_h
#define __sock_h

typedef int socket_t ;

#define SOCKET_BAD	-1

#if defined(WIN32) || defined(TPF)
#define VERIFY_SOCK(x)  /* sockets are not in range [0..FD_SETSIZE] */
#else
#define VALID_SOCK(s) (((s) >= 0) && ((s) < FD_SETSIZE))
#define VERIFY_SOCK(x) do { \
  if(!VALID_SOCK(x)) { \
    errno = EINVAL; \
    return -1; \
  } \
} while(0)
#endif

extern int socket_errno(void) ;
extern int socket_close(int sock) ;

#endif

#ifdef __cplusplus /*Z*/
}
#endif
