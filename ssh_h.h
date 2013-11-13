#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#define HAVE_SYS_SOCKET_H	1
#define HAVE_ARPA_INET_H	1
#define HAVE_UNISTD_H		1

#include	<libssh2.h>

#ifdef HAVE_WINDOWS_H
#include	 <windows.h>
#endif

#ifdef HAVE_WINSOCK2_H
#include	<winsock2.h>
#endif

#ifdef HAVE_SYS_SOCKET_H
#include	<sys/socket.h>
#endif

#ifdef HAVE_NETINET_IN_H
#include	<netinet/in.h>
#endif

#ifdef HAVE_UNISTD_H
#include	<unistd.h>
#endif

#ifdef HAVE_ARPA_INET_H
#include	<arpa/inet.h>
#endif


#ifdef __cplusplus /*Z*/
}
#endif
