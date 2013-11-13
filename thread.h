#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#ifndef __thread_h
#define __thread_h

#ifndef WIN32
#include <pthread.h>
#define THREAD_CC
#define THREAD_TYPE pthread_t
#define THREAD_CREATE(tid, entry, arg) pthread_create(&(tid), NULL, (entry), (arg))
#else
#include <windows.h>
#define THREAD_CC _ _cdecl
#define THREAD_TYPE DWORD
#define THREAD_CREATE(tid, entry, arg) \
    do { _beginthread((entry), 0,(arg)); \
	(tid) = GetCurrentThreadId();	 \
    } while (0)	
#endif
#endif

#ifdef __cplusplus /*Z*/
}
#endif
