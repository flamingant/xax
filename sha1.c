#ifdef __cplusplus /*Z*/
extern "C" {
#endif

#include <inttypes.h> /* uint8_t */
#include <stdarg.h>

#include <openssl/sha.h>

extern void tr_sha1 (uint8_t * setme, const void * content1, int content1_len, ...)
{
  va_list vl;
  SHA_CTX sha;
  const void * content;

  SHA1_Init (&sha);
  SHA1_Update (&sha, content1, content1_len);

  va_start (vl, content1_len);
  while ((content = va_arg (vl, const void*)))
    SHA1_Update (&sha, content, va_arg (vl, int));
  va_end (vl);

  SHA1_Final (setme, &sha);
}

/* ================================================================ */
static int s_main(int argc,char **argv,char *mode)
{
    uint8_t sha[SHA_DIGEST_LENGTH];
    tr_sha1(sha,"hello world",11,0,0) ;
    
    return(0) ;
}

/* ~# use mainmode ; #~ */
/* ~~mode("s",
   desc		=> "sha1 test",
   )~~ */

#include	".gen/sha1.c"

#ifdef __cplusplus /*Z*/
}
#endif
