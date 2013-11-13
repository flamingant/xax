/* ~= use arg ; =~ */

/* ~~argset(name => "FOO",help => "argset 1")~~ */

/* ~~arg(help => "Set server document to directory")~~ */
static int argf__http_document_root(char *name,char *value,void *a0)
{
    g_hsg.document_root = strdup(value) ;
    return(ASF_ARGACCEPTED) ;
}

/* ~~arg(help => "foo")~~ */
static int argf__http_foo(char *name,char *value,void *a0)
{
    g_hsg.document_root = strdup(value) ;
    return(ASF_ARGACCEPTED) ;
/*~+(value => "override")+~*/
}

/* ~~argset(name => "BAR",help => "argset 2")~~ */

/* ~~arg(value => "prelude")~~ */
static int argf__bubbo(char *name,char *value,void *a0)
{
    return(ASF_ARGACCEPTED) ;
}
