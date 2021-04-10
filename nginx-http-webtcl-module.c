#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <tcl.h>

static char * ngx_zstrdup(ngx_pool_t *pool, ngx_str_t *src)
{
  
    char  *dst;
    int zero = 1;

    ngx_uint_t dest_len = src->len; 
    if(zero) dest_len += 1; 

    dst = ngx_pnalloc(pool, dest_len);
    if (dst == NULL) {
        return NULL;
    }

    ngx_memcpy(dst, src->data, src->len);
    if(zero) dst[src->len] = '\0';


    return dst;
}

static u_char * ngx_str_copy(ngx_pool_t *pool, ngx_str_t *dest, ngx_str_t *src, int zero)
{
  
    u_char  *dst;

    ngx_uint_t dest_len = src->len; 
    if(zero) dest_len += 1; 

    dst = ngx_pnalloc(pool, dest_len);
    if (dst == NULL) {
        return NULL;
    }

    ngx_memcpy(dst, src->data, src->len);
    if(zero) dst[src->len] = '\0';

    dest->data = dst;
    dest->len  = dest_len;

    return dst;
}

#if 1
static ngx_str_t * ngx_str_clone(ngx_pool_t *pool, ngx_str_t *src, int zero)
{
  
    ngx_str_t *dest = ngx_pnalloc(pool, sizeof(ngx_str_t));

    u_char  *dst;

    ngx_uint_t dest_len = src->len; 
    if(zero) dest_len += 1; 

    dst = ngx_pnalloc(pool, dest_len);
    if (dst == NULL) {
        return NULL;
    }

    ngx_memcpy(dst, src->data, src->len);
    if(zero) dst[src->len] = '\0';

    dest->data = dst;
    dest->len  = dest_len;

    return dest;
}
#endif

#if 0
static ngx_core_module_t  ngx_webtcl_module_ctx = {
    ngx_string("webtcl"),
    ngx_webtcl_create_conf,
    ngx_webtcl_init_conf
};
#endif

#if 0
static const ngx_str_t ngx_str_webtcl = ngx_string("webtcl");
#endif

static void * ngx_http_webtcl_create_loc_conf(ngx_conf_t *cf);
static char * ngx_http_webtcl_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child);
static ngx_int_t ngx_http_webtcl_postconfig(ngx_conf_t *cf);

static ngx_http_module_t  ngx_http_webtcl_module_ctx = {
    NULL,                                  /* preconfiguration */
    ngx_http_webtcl_postconfig,            /* postconfiguration: ngx_int_t (*module_setup)(ngx_conf_t *cf) */

    NULL,                                  /* create main configuration */
    NULL,                                  /* init main configuration */

    NULL,                                  /* create server configuration */
    NULL,                                  /* merge server configuration */

    ngx_http_webtcl_create_loc_conf,       /* create location configuration */
    ngx_http_webtcl_merge_loc_conf         /* merge location configuration */
};


static char * ngx_http_command_webtcl(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);


//      NGX_HTTP_LOC_CONF|NGX_CONF_BLOCK|NGX_CONF_NOARGS ,

static ngx_command_t  ngx_http_webtcl_commands[] = {

    { ngx_string("webtcl"),
      NGX_HTTP_LOC_CONF|NGX_CONF_1MORE,
      ngx_http_command_webtcl,
      NGX_HTTP_LOC_CONF_OFFSET,            /* ngx_uint_t conf ; e.g. NGX_HTTP_LOC_CONF_OFFSET */
      0,                                   /* ngx_uint_t offset */
      NULL                                 /* void *post */
    },

    ngx_null_command
};

static ngx_int_t init_module(ngx_cycle_t *cycle){
    ngx_log_error(NGX_LOG_NOTICE, cycle->log, 0, "webtcl init_module in %d", ngx_process);
    return NGX_OK;
}
static ngx_int_t init_process(ngx_cycle_t *cycle){
    ngx_log_error(NGX_LOG_NOTICE, cycle->log, 0, "webtcl init_process in %d", ngx_process);
    return NGX_OK;
}


ngx_module_t  ngx_http_webtcl_module = {
    NGX_MODULE_V1,
    &ngx_http_webtcl_module_ctx,           /* module context */
    ngx_http_webtcl_commands,              /* module directives */
    NGX_HTTP_MODULE,                       /* module type */
    NULL,                                  /* init master  (*init_master)(ngx_log_t *log); */
    init_module,                           /* init module  (*init_module)(ngx_cycle_t *cycle); */
    init_process,                          /* init process (*init_process)(ngx_cycle_t *cycle); */
    NULL,                                  /* init thread  (*init_thread)(ngx_cycle_t *cycle); */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process (*exit_process)(ngx_cycle_t *cycle); */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};

typedef struct {
    ngx_uint_t  foo;
    Tcl_Interp *interp;
    ngx_array_t webtcl_cmds;
    ngx_array_t access_cmds;
    ngx_array_t content_cmds;
    ngx_array_t complex_values;
} ngx_http_webtcl_loc_conf_t;

static void * ngx_http_webtcl_create_loc_conf(ngx_conf_t *cf)
{
    ngx_log_error(NGX_LOG_NOTICE, cf->log, 0, "webtcl module create location config in %d", ngx_process);

    ngx_http_webtcl_loc_conf_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_webtcl_loc_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    conf->foo = NGX_CONF_UNSET_UINT;
    conf->interp = Tcl_CreateInterp();
    // Tcl_Init(conf->interp);

    ngx_array_init(&conf->webtcl_cmds, cf->pool, 1, sizeof(ngx_array_t));
    ngx_array_init(&conf->access_cmds, cf->pool, 1, sizeof(ngx_array_t));
    ngx_array_init(&conf->content_cmds, cf->pool, 1, sizeof(ngx_array_t));
    ngx_array_init(&conf->complex_values, cf->pool, 1, sizeof(ngx_http_complex_value_t));

    return conf;
}


static char * ngx_http_webtcl_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_log_error(NGX_LOG_NOTICE, cf->log, 0, "webtcl module merge location config in %d", ngx_process);

    ngx_http_webtcl_loc_conf_t *prev = parent;
    ngx_http_webtcl_loc_conf_t *conf = child;

    ngx_conf_merge_uint_value(conf->foo, prev->foo, 1);

    return NGX_CONF_OK;
}

typedef struct {
   Tcl_Interp *interp;
   const char *script;
} webtcl_variable_ctx;

typedef struct {
   int type;
   ngx_array_t argv;
} webtcl_command_t ;

static inline int ngx_str_equal(const char *s1, ngx_str_t *s2){
  int s1_len = strlen(s1);
  int s2_len = s2->len;
  if(s2->data[s2->len-1]=='\0') s2_len--;
  if(s1_len != s2_len) return 0;

  return strncmp(s1, (const char *)s2->data, s1_len<s2_len?s1_len:s2_len)==0;
}

static ngx_int_t
ngx_http_webtcl_get_variable(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data){

  webtcl_variable_ctx *vctx = (webtcl_variable_ctx *) data;

  Tcl_Interp *interp = vctx->interp;

  Tcl_Obj *objPtr = Tcl_NewStringObj(vctx->script, -1);
  Tcl_Obj *objResult = Tcl_SubstObj(interp, objPtr, TCL_SUBST_ALL);

  int size;
  const char *result = Tcl_GetStringFromObj(objResult, &size);
  v->data = ngx_pnalloc(r->pool, size);

  v->len  = ngx_sprintf(v->data, "%s", result) - v->data;
  v->valid = 1;
  v->no_cacheable = 0;
  v->not_found = 0;

  Tcl_DecrRefCount(objPtr);
  Tcl_DecrRefCount(objResult);

  return NGX_OK;
}



static char * ngx_http_command_webtcl(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_webtcl_loc_conf_t *webtcl_conf;

    if(0){
      webtcl_conf = conf;
    }else{
      webtcl_conf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_webtcl_module);
    }




    ngx_str_t *argv = cf->args->elts;
    ngx_uint_t argc = cf->args->nelts;

    ngx_str_t subcmd = argv[1];

    /* TODO:
     *   [+] "set"
     *   [+] "content"
     */

    /*
     * webtcl variable foo $uri
     * webtcl set      bar "[expr 3+4]"
     */

    if( ngx_str_equal("set", &subcmd) ){
      /* webtcl set name value */
      ngx_http_variable_t *var;
      var = ngx_http_add_variable(cf, &argv[2], NGX_HTTP_VAR_CHANGEABLE); 

      webtcl_variable_ctx *vctx;
      vctx = ngx_pcalloc(cf->pool, sizeof(webtcl_variable_ctx));
      vctx->interp = webtcl_conf->interp;
      vctx->script = (const char *) ngx_pstrdup(cf->pool, &argv[3]);

      var->get_handler = ngx_http_webtcl_get_variable;
      var->data = (uintptr_t) vctx;
    }else if( ngx_str_equal("variable", &subcmd) ){
      if(argc!=4){
          fprintf(stderr, "webtcl command argc = %ld\n", argc);
          return NGX_CONF_ERROR;
      }

      #if 1
      ngx_http_complex_value_t           *cv;
      ngx_http_compile_complex_value_t   ccv;
      
      cv = ngx_array_push(&webtcl_conf->complex_values);

      ngx_memzero(&ccv, sizeof(ngx_http_compile_complex_value_t));
      
      ccv.cf = cf;
      ccv.value = ngx_str_clone(cf->pool, &argv[3], 0);
      ccv.complex_value = cv;
      ccv.zero = 1;
      ccv.conf_prefix = 0;
      
      fprintf(stderr, "webtcl variable compile %.*s\n", (int) argv[3].len, argv[3].data);
      if (ngx_http_compile_complex_value(&ccv) != NGX_OK) {
      fprintf(stderr, "webtcl variable compile fail %.*s\n", (int) argv[3].len, argv[3].data);
          return NGX_CONF_ERROR;
      }
     
      // ngx_log_error(NGX_LOG_ERR, cf->log, 0, "webtcl variable %.*s", argv[3].len, argv[3].data);
      fprintf(stderr, "webtcl variable %.*s\n", (int) argv[3].len, argv[3].data);

      ngx_array_t *cmd_argv;
      cmd_argv = ngx_array_push(&webtcl_conf->content_cmds);
      ngx_array_init(cmd_argv, cf->pool, argc, sizeof(ngx_str_t));

      for(ngx_uint_t i=0; i<argc; i++){
         ngx_str_t *dst_arg = ngx_array_push(cmd_argv);
         ngx_str_copy(cf->pool, dst_arg, &argv[i], 0);
      }
      fprintf(stderr, "webtcl variable %.*s\n", (int) argv[3].len, argv[3].data);
      #endif
    }else if(
         ngx_str_equal("echo", &subcmd)
      || ngx_str_equal("puts", &subcmd)
      || ngx_str_equal("content", &subcmd)
      || ngx_str_equal("header", &subcmd)
      || ngx_str_equal("return", &subcmd)
    ){
       ngx_array_t *cmd_argv;
       cmd_argv = ngx_array_push(&webtcl_conf->content_cmds);
       ngx_array_init(cmd_argv, cf->pool, argc, sizeof(ngx_str_t));

       for(ngx_uint_t i=0; i<argc; i++){
          ngx_str_t *dst_arg = ngx_array_push(cmd_argv);
          ngx_str_copy(cf->pool, dst_arg, &argv[i], 0);
       }
    }else if(
         ngx_str_equal("allow", &subcmd)
      || ngx_str_equal("deny", &subcmd)
    ){
       ngx_array_t *cmd_argv;
       cmd_argv = ngx_array_push(&webtcl_conf->access_cmds);
       ngx_array_init(cmd_argv, cf->pool, argc, sizeof(ngx_str_t));

       for(ngx_uint_t i=0; i<argc; i++){
          ngx_str_t *dst_arg = ngx_array_push(cmd_argv);
          ngx_str_copy(cf->pool, dst_arg, &argv[i], 0);
       }
    } else {

      ngx_array_t *cmd_argv = ngx_array_push(&webtcl_conf->webtcl_cmds);
      ngx_array_init(cmd_argv, cf->pool, argc, sizeof(ngx_str_t));

      for(ngx_uint_t i=0; i<argc; i++){
         ngx_str_t *dst_arg = ngx_array_push(cmd_argv);
         dst_arg->data = ngx_pstrdup(cf->pool, &argv[i]);
         dst_arg->len  = argv[i].len;
      }
    }
      fprintf(stderr, "webtcl postconfig ok\n");

   /*

    ngx_array_init(&b, pool, 10, sizeof(ngx_http_complex_value_t));

    ngx_array_t *cvalues = ngx_array_create(pool, 10, sizeof(ngx_http_complex_value_t));

    ngx_http_complex_value_t  *cv;
    ngx_http_compile_complex_value_t   ccv;

    ngx_memzero(&ccv, sizeof(ngx_http_compile_complex_value_t));
    cv = (ngx_http_complex_value_t *) (cvalues->elts + i*cvalues->size);
    ccv.cf = cf;
    ccv.value = &argv[i];
    ccv.complex_value = cv;
    ccv.zero = 1;
    ccv.conf_prefix = 0;

    if (ngx_http_compile_complex_value(&ccv) != NGX_OK) {
       return NGX_CONF_ERROR;
    }

    ngx_str_t  res;

    if (ngx_http_complex_value(r, &cv, &res) != NGX_OK) {
      return NGX_ERROR;
    }
    */


   
    /*
    ngx_http_core_loc_conf_t  *clcf;

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    */

    // TODO:
    // clcf->handler = ngx_http_bar_handler;

    return NGX_CONF_OK;
}

#if 1
static ngx_int_t ngx_http_webtcl_handler_preaccess(ngx_http_request_t *r);
static ngx_int_t ngx_http_webtcl_handler_content(ngx_http_request_t *r);

static ngx_int_t ngx_http_webtcl_postconfig(ngx_conf_t *cf)
{
    ngx_log_error(NGX_LOG_NOTICE, cf->log, 0, "webtcl setup in %d", ngx_process);

    ngx_http_core_main_conf_t  *cmcf;
    ngx_http_webtcl_loc_conf_t *webtcl_conf;

    cmcf        = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);
    webtcl_conf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_webtcl_module);

    if( 1 || webtcl_conf->access_cmds.nelts > 0 ){
      ngx_http_handler_pt *h;

      h    = ngx_array_push(&cmcf->phases[NGX_HTTP_PREACCESS_PHASE].handlers);
      if (h == NULL) {
          ngx_log_error(NGX_LOG_NOTICE, cf->log, 0, "webtcl skip install preaccess handler in %d", ngx_process);
          return NGX_ERROR;
      }

      ngx_log_error(NGX_LOG_NOTICE, cf->log, 0, "webtcl install preaccess handler in %d", ngx_process);
      *h = ngx_http_webtcl_handler_preaccess;
    }

    if( 1 || webtcl_conf->content_cmds.nelts > 0 ){
      ngx_http_handler_pt *h;

      h    = ngx_array_push(&cmcf->phases[NGX_HTTP_CONTENT_PHASE].handlers);
      if (h == NULL) {
          return NGX_ERROR;
      }

      ngx_log_error(NGX_LOG_NOTICE, cf->log, 0, "webtcl install content handler in %d", ngx_process);
      *h = ngx_http_webtcl_handler_content;
    }

    return NGX_OK;
}

#define USE_MODULE_LOC_CONF(name) \
    ngx_http_webtcl_loc_conf_t *name = ngx_http_get_module_loc_conf(r, ngx_http_webtcl_module);
  
static ngx_int_t ngx_http_webtcl_handler_preaccess(ngx_http_request_t *r)
{
    USE_MODULE_LOC_CONF(webtcl_conf);

    if( webtcl_conf->access_cmds.nelts == 0 ){
      return NGX_DECLINED;
    }

    ngx_connection_t *c = r->connection;
    ngx_log_error(NGX_LOG_NOTICE, c->log, 0, "webtcl preaccess in %d", ngx_process);

    ngx_array_t *webtcl_cmds = webtcl_conf->webtcl_cmds.elts;

    fprintf(stderr, "see N webtcl %ld\n", webtcl_conf->webtcl_cmds.nelts);
    
    for(ngx_uint_t i=0, n=webtcl_conf->webtcl_cmds.nelts; i<n; i++){
       ngx_str_t *argv = webtcl_cmds[i].elts;
       ngx_uint_t argc = webtcl_cmds[i].nelts;
       fprintf(stderr, "see argc = %ld\n", argc);
    
       for(ngx_uint_t j=0; j<argc; j++){
          fprintf(stderr, "see arg = %.*s\n", (int) argv[j].len, argv[j].data);
       }
    }


    return NGX_DECLINED;
    // return NGX_HTTP_FORBIDDEN;


    // return NGX_DECLINED;
}

static ngx_int_t ngx_http_webtcl_handler_content(ngx_http_request_t *r)
{
    USE_MODULE_LOC_CONF(webtcl_conf);

    if( webtcl_conf->content_cmds.nelts == 0 ){
      return NGX_DECLINED;
    }

    Tcl_Interp *interp = webtcl_conf->interp;

    ngx_buf_t *buf; // ngx_calloc_buf(r->pool);

    if(0){
      buf = ngx_calloc_buf(r->pool);
    }else{
      buf = ngx_create_temp_buf(r->pool, 1024);
    }

    ngx_int_t header_status = NGX_HTTP_OK;

    ngx_http_complex_value_t *webtcl_complex_value = webtcl_conf->complex_values.elts;

    ngx_array_t *content_cmds = webtcl_conf->content_cmds.elts;
    for(ngx_uint_t i=0, n=webtcl_conf->content_cmds.nelts; i<n; i++){
       ngx_str_t *argv = content_cmds[i].elts;
       ngx_uint_t argc = content_cmds[i].nelts;

       ngx_str_t *subcmd = argv+1;

       if( ngx_str_equal("echo", subcmd) ){
         for(ngx_uint_t j=2; j<argc; j++){
            buf->last = ngx_sprintf(buf->last, "%V ", &argv[j]);
         }
         buf->last = ngx_sprintf(buf->last-1, "\n");
       }else if( ngx_str_equal("puts", subcmd) ){
          ngx_str_t *arg_text = argv+2;
	  Tcl_Obj *objPtr = Tcl_NewStringObj((const char *)arg_text->data, arg_text->len);
	  Tcl_Obj *objResult = Tcl_SubstObj(interp, objPtr, TCL_SUBST_ALL);

	  int size;
	  const char *result = Tcl_GetStringFromObj(objResult, &size);
          buf->last = ngx_sprintf(buf->last, "%*s\n", size, result);
       }else if( ngx_str_equal("content", subcmd) ){
          ngx_str_t *arg_script = argv+2;
	  Tcl_Preserve(interp);
	  int ret = Tcl_EvalEx(interp, (const char *) arg_script->data, arg_script->len, 0);
          if( 1 || ret==TCL_OK ){
	    const char *tcl_result = Tcl_GetStringResult(interp);
	    buf->last = ngx_sprintf(buf->last, "%s", tcl_result);
          }
	  Tcl_Release(interp);
       }else if( ngx_str_equal("header", subcmd) ){
         ngx_str_t *arg_name  = argv+2;
         ngx_str_t *arg_value = argv+3;
         ngx_table_elt_t *header = ngx_list_push(&r->headers_out.headers);
         header->hash  = 1;
         header->key   = *arg_name;
         header->value = *arg_value;
       }else if( ngx_str_equal("return", subcmd) ){
         ngx_str_t *arg_code = argv+2;
         // TODO:
         if(ngx_strncmp(arg_code->data, "201", 3)==0){
           header_status = 201;
         }
       }else if( ngx_str_equal("variable", subcmd) ){
         #if 1
         ngx_str_t  res;

         if (ngx_http_complex_value(r, webtcl_complex_value++, &res) != NGX_OK) {
             ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "fail variable %s = %s", argv[2].data, res.data);
             return NGX_ERROR;
         }

         Tcl_SetVar(interp, ngx_zstrdup(r->pool, &argv[2]), ngx_zstrdup(r->pool, &res), 0);
         #endif
       }else{
         for(ngx_uint_t j=0; j<argc; j++){
            buf->last = ngx_sprintf(buf->last, "see arg %V\n", &argv[j]);
         }
       }
    }


    buf->last_buf = (r==r->main)?1:0;
    buf->last_in_chain = 1;

    /*
    buf->memory = 1;
    */

    ngx_chain_t out;
    out.buf = buf;
    out.next = NULL;


    r->headers_out.status = header_status;
    r->headers_out.content_length_n = buf->last - buf->pos ;
    ngx_http_send_header(r);

    ngx_http_output_filter(r, &out);

    ngx_http_finalize_request(r, NGX_DONE);

    return NGX_OK;
}
#endif

