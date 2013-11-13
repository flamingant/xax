#ifdef __cplusplus /*Z*/
extern "C" {
#endif

/*
 * C implementation of a bencode decoder.
 * This is the format defined by BitTorrent:
 *  http://wiki.theory.org/BitTorrentSpecification#bencoding
 *
 * The only external requirements are a few [standard] function calls and
 * the long long type.  Any sane system should provide all of these things.
 *
 * See the bencode.h header file for usage information.
 *
 * This is released into the public domain:
 *  http://en.wikipedia.org/wiki/Public_Domain
 *
 * Written by:
 *   Mike Frysinger <vapier@gmail.com>
 * And improvements from:
 *   Gilles Chanteperdrix <gilles.chanteperdrix@xenomai.org>
 */

/*
 * This implementation isn't optimized at all as I wrote it to support a bogus
 * system.  I have no real interest in this format.  Feel free to send me
 * patches (so long as you don't copyright them and you release your changes
 * into the public domain as well).
 */

#include <stdlib.h> /* malloc() realloc() free() strtoll() */
#include <string.h> /* memset() */

#include "bencode.h"

static be_node *be_alloc(be_type type)
{
	be_node *ret = malloc(sizeof(*ret));
	if (ret) {
	    ret->type = type;
	    ret->val.s = 0 ;
	}
	return ret;
}

static long long _be_decode_int(const char **data, long long *data_len)
{
	char *endp;
	long long ret = strtoll(*data, &endp, 10);
	*data_len -= (endp - *data);
	*data = endp;
	return ret;
}

long long be_str_len(be_node *node)
{
	long long ret = 0;
	if (node->val.s)
		memcpy(&ret, node->val.s - sizeof(ret), sizeof(ret));
	return ret;
}

static char *_be_decode_str(const char **data, long long *data_len)
{
	long long sllen = _be_decode_int(data, data_len);
	long slen = sllen;
	unsigned long len;
	char *ret = NULL;

	/* slen is signed, so negative values get rejected */
	if (sllen < 0)
		return ret;

	/* reject attempts to allocate large values that overflow the
	 * size_t type which is used with malloc()
	 */
	if (sizeof(long long) != sizeof(long))
		if (sllen != slen)
			return ret;

	/* make sure we have enough data left */
	if (sllen > *data_len - 1)
		return ret;

	/* switch from signed to unsigned so we don't overflow below */
	len = slen;

	if (**data == ':') {
		char *_ret = malloc(sizeof(sllen) + len + 1);
		memcpy(_ret, &sllen, sizeof(sllen));
		ret = _ret + sizeof(sllen);
		memcpy(ret, *data + 1, len);
		ret[len] = '\0';
		*data += len + 1;
		*data_len -= len + 1;
	}
	return ret;
}

static be_node *_be_decode(const char **data, long long *data_len)
{
	be_node *ret = NULL;

	if (!*data_len)
		return ret;

	switch (**data) {
		/* lists */
		case 'l': {
			ret = be_alloc(BE_LIST);

			--(*data_len);
			++(*data);
			while (**data != 'e') {
			    be_node *e = _be_decode(data, data_len);
			    ret->val.l = rcons_cons(e,ret->val.l) ;
			}
			--(*data_len);
			++(*data);

			ret->val.l = rcons_reverse(ret->val.l) ;

			return ret;
		}

		/* dictionaries */
		case 'd': {
			ret = be_alloc(BE_DICT);

			--(*data_len);
			++(*data);
			while (**data != 'e') {
			    be_dict *d = malloc(sizeof(be_dict)) ;
			    d->key = _be_decode_str(data, data_len);
			    d->val = _be_decode(data, data_len) ;
			    ret->val.d = rcons_cons(d,ret->val.d) ;
			}
			--(*data_len);
			++(*data);

			ret->val.d = rcons_reverse(ret->val.d) ;

			return ret;
		}

		/* integers */
		case 'i': {
			ret = be_alloc(BE_INT);

			--(*data_len);
			++(*data);
			ret->val.i = _be_decode_int(data, data_len);
			if (**data != 'e')
				return NULL;
			--(*data_len);
			++(*data);

			return ret;
		}

		/* byte strings */
		case '0'...'9': {
			ret = be_alloc(BE_STR);

			ret->val.s = _be_decode_str(data, data_len);

			return ret;
		}

		/* invalid */
		default:
			break;
	}

	return ret;
}

be_node *be_decoden(const char *data, long long len)
{
	return _be_decode(&data, &len);
}

be_node *be_decode(const char *data)
{
	return be_decoden(data, strlen(data));
}

static inline void _be_free_str(char *str)
{
	if (str)
		free(str - sizeof(long long));
}
void be_free(be_node *node)
{
    RCONS *r ;
	switch (node->type) {
		case BE_STR:
			_be_free_str(node->val.s);
			break;

		case BE_INT:
			break;

		case BE_LIST: {
			for (r = node->val.l ; r ; r = rcons_free(r)) {
			    be_node *e = r->car ;
			    be_free(e) ;
			    }
			break;
		}

		case BE_DICT: {
			for (r = node->val.d ; r ; r = rcons_free(r)) {
			    be_dict *d = r->car ;
			    _be_free_str(d->key);
			    be_free(d->val);
			    }
			break;
		}
	}
	free(node);
}

/* ================================================================ */
extern be_node *be_dict_assoc(be_node *node,char *name)
{
    RCONS	*r ;
    for (r = node->val.d ; r ; r = r->cdr) {
	be_dict *d = (be_dict *) r->car ;
	if (!strcmp(d->key,name))
	    return(d->val) ;
	}
    return(0) ;
    }

extern be_node *be_list_aref(be_node *node,int i)
{
    RCONS	*r ;
    for (r = node->val.d ; r ; r = r->cdr) {
	if (i-- <= 0) return((be_node *) r->car) ;
	}
    return(0) ;
    }

/* ================================================================ */
#define BE_DEBUG

#ifdef BE_DEBUG
#include <stdio.h>
#include <stdint.h>

static void _be_dump_indent(ssize_t indent)
{
	while (indent-- > 0)
		printf("    ");
}
static void _be_dump(be_node *node, ssize_t indent)
{
    RCONS *r ;

	_be_dump_indent(indent);
	indent = abs(indent);

	switch (node->type) {
		case BE_STR:
			printf("str = %s (len = %lli)\n", node->val.s, be_str_len(node));
			break;

		case BE_INT:
			printf("int = %lli\n", node->val.i);
			break;

		case BE_LIST:
			puts("list [");

			for (r = node->val.l ; r ; r = r->cdr) {
				_be_dump((be_node *) r->car, indent + 1);
				}

			_be_dump_indent(indent);
			puts("]");
			break;

		case BE_DICT:
			puts("dict {");

			for (r = node->val.l ; r ; r = r->cdr) {
			    be_dict *d = r->car ;
				_be_dump_indent(indent + 1);
				printf("%s => ", d->key);
				_be_dump(d->val, -(indent + 1));
			}

			_be_dump_indent(indent);
			puts("}");
			break;
	}
}
void be_dump(be_node *node)
{
	_be_dump(node, 0);
}

#include	"arg.h"
#include	"common.h"

static int ben_main(int argc,char **argv,char *mode)
{
    char	*file = argv[1] ;
{
    MT		mt[1] ;
    mt_file_contents(mt,file,0) ;
    be_node *b = be_decoden(mt->s,MTFillSize(mt)) ;
{
    be_node *e = be_dict_assoc(b,"info") ;
    e = be_dict_assoc(e,"name") ;
    be_dump(e) ;
    e = be_list_aref(be_dict_assoc(be_list_aref(be_dict_assoc(be_dict_assoc(b,"info"),"files"),1),"path"),0) ;
    printf("path=%s\n",e->val.s) ;
    }
    
    be_free(b) ;
    }
    return 0 ;
    }

/* ~# use mainmode ; #~ */
/* ~~mode("ben",
   desc => "bencode test mode",
   )~~ */

#include	".gen/bencode.c"

#endif

#ifdef __cplusplus /*Z*/
}
#endif
