/*
 * Copyright (c) 2016, Juniper Networks, Inc.
 * All rights reserved.
 */

#include <stdio.h>
#include "foo.grpc-c.h"


extern int grpc_api_trace;


/*
 * Takes as argument the socket name
 */
int 
main (int argc, char **argv) 
{
    if (argc < 2)
    {
	    fprintf(stderr, "Too few arguments\n");
	    exit(1);
    }

    /*
     * Initialize grpc-c library to be used with vanilla grpc
     */
    grpc_c_init(GRPC_THREADS, NULL);


    grpc_api_trace = 1;
    gpr_set_log_verbosity(1);


    int i;
    for ( i = 0 ; i < 2 ; i++ )
    {

        /*
         * Create a client object with client name as foo client to be talking to
         * a insecure server
         */
        grpc_c_client_t *client = grpc_c_client_init_by_host("127.0.0.1:3000", "foo client", NULL, NULL);

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
        int status = foo__greeter__say_hello(client, NULL, 0, &h, &r, NULL, -1);
        if (r) 
        {
            //printf("Got back: %s\n", r->message);
        }

        //printf("Finished with %d\n", status);

        grpc_c_client_free(client);
    }

}
