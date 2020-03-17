#include <stdio.h>
#include "foo.grpc-c.h"


void foo_client_call_0( grpc_c_client_t *client ) {
    
    int i;
    int ret;
    grpc_c_status_t status;
    
    for ( i = 0 ; i < 10000 ; i++ )
    {
        /*
         * Create a hello request message and call RPC
         */
        foo__HelloRequest h;
        foo__hello_request__init(&h);
        foo__HelloReply *r;
        
        char str[BUFSIZ];
        snprintf(str, BUFSIZ, "world");
        h.name = str;
        
        /*
         * This will invoke a blocking RPC
         */
        ret = foo__greeter__say_hello__sync(client, NULL, 0, &h, &r, &status, -1);
        if ( ret )
        {
            printf("call failed! %d\n", ret);
            break;
        }
        
        foo__hello_reply_free(r);
    }

    printf("Total Count %d\n", i);
}


void foo_client_call_1( grpc_c_client_t *client ) 
{
    int i;
    int ret;
    grpc_c_status_t   status;
    grpc_c_context_t *context;

    ret = foo__greeter__say_hello1__stream( client, NULL, 0, &context, -1);
    if ( ret )
    {
        printf("stream connect failed! %d\n", ret);
        return;
    }

    /*
     * Create a hello request message and call RPC
     */
    foo__HelloRequest h;
    foo__hello_request__init(&h);

    char str[1024];
    
    snprintf(str, 1024, "world");
    h.name = str;

    for ( i = 0 ; i < 10000 ; i++ )
    {
        ret = grpc_c_write(context, &h, 0, -1);
        if ( ret )
        {
            printf("stream write failed! %d\n", ret);
            break;
        }
    }

    ret = grpc_c_write_done(context, 0, -1);
    if ( ret )
    {
        printf("stream write done failed! %d\n", ret);
    }

    foo__HelloReply *r;

    ret = grpc_c_read(context, (void **)&r, 0, -1);
    if ( ret )
    {
        printf("stream read failed! %d\n", ret);
    }
    else
    {
        foo__hello_reply_free(r);
    }

    ret = grpc_c_finish(context, &status, 0);
    if ( ret )
    {
        printf("stream finish failed! %d\n", ret);
    }    

    printf("Total Count %d\n", i);
}

void foo_client_call_2( grpc_c_client_t *client ) 
{
    int i;
    int ret;
    grpc_c_status_t   status;
    grpc_c_context_t *context;

    ret = foo__greeter__say_hello2__stream( client, NULL, 0, &context, -1);
    if ( ret )
    {
        printf("stream connect failed! %d\n", ret);
        return;
    }

    /*
     * Create a hello request message and call RPC
     */
    foo__HelloRequest h;
    foo__hello_request__init(&h);

    char str[1024];
    
    snprintf(str, 1024, "world");
    h.name = str;

    ret = grpc_c_write(context, &h, 0, -1);
    if ( ret )
    {
        printf("stream write failed! %d\n", ret);
    }

    ret = grpc_c_write_done(context, 0, -1);
    if ( ret )
    {
        printf("stream write done failed! %d\n", ret);
    }

    for ( i = 0 ;; i++ )
    {
        foo__HelloReply *r;
        
        ret = grpc_c_read(context, (void **)&r, 0, -1);
        if ( ret )
        {
            break;
        }

        foo__hello_reply_free(r);
    }
    
    ret = grpc_c_finish(context, &status, 0);
    if ( ret )
    {
        printf("stream finish failed! %d\n", ret);
    }    

    printf("Total Count %d\n", i);
}


void foo_client_call_3( grpc_c_client_t *client ) 
{
    int i;
    int ret;
    grpc_c_status_t   status;
    grpc_c_context_t *context;

    ret = foo__greeter__say_hello3__stream( client, NULL, 0, &context, -1);
    if ( ret )
    {
        printf("stream connect failed! %d\n", ret);
        return;
    }

    /*
     * Create a hello request message and call RPC
     */
    foo__HelloRequest h;
    foo__hello_request__init(&h);

    char str[1024];
    
    snprintf(str, 1024, "world");
    h.name = str;

    for ( i = 0 ; i < 10000 ; i++ )
    {
        ret = grpc_c_write(context, &h, 0, -1);
        if ( ret )
        {
            printf("stream write failed! %d\n", ret);
            break;
        }

        foo__HelloReply *r;

        ret = grpc_c_read(context, (void **)&r, 0, -1);
        if ( ret )
        {
            printf("stream read failed! %d\n", ret);
            break;
        }

        foo__hello_reply_free(r);
    }

    ret = grpc_c_write_done(context, 0, -1);
    if ( ret )
    {
        printf("stream write done failed! %d\n", ret);
    }

    ret = grpc_c_finish(context, &status, 0);
    if ( ret )
    {
        printf("stream finish failed! %d\n", ret);
    }    

    printf("Total Count %d\n", i);
}


/*
 * Takes as argument the socket name
 */
int foo_client() 
{
    /*
     * Initialize grpc-c library to be used with vanilla grpc
     */
    grpc_c_init();

    /*
     * Create a client object with client name as foo client to be talking to
     * a insecure server
     * if thread_nums <= 0, grpc_c will create the number of the cup core threads, otherwise it will create thread_nums threads
     */
    grpc_c_client_t *client = grpc_c_client_init("127.0.0.1:3000", NULL, NULL, 0);

    foo_client_call_0(client);
    foo_client_call_1(client);
    foo_client_call_2(client);
    foo_client_call_3(client);

    grpc_c_client_stop(client);
    grpc_c_client_wait(client);
    grpc_c_client_free(client);

    grpc_c_shutdown();
}
