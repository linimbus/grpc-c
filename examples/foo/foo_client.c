#include <stdio.h>
#include "foo.grpc-c.h"

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
     */
    grpc_c_client_t *client = grpc_c_client_init("127.0.0.1:3000", NULL, NULL);

    int i;
	int ret;
	grpc_c_status_t status;
	
    for ( i = 0 ; i < 100 ; i++ )
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
        int ret = foo__greeter__say_hello__sync(client, NULL, 0, &h, &r, &status, -1);
        if (r) 
        {
            printf("Got back: %s\n", r->message);
            foo__hello_reply_free(r);
        }

        if ( 0 == ret)
        {
            printf("Finished with %d\n", status.code);
        }
        else
        {
            printf("return failed %d\n", ret);
        }
    }

	printf("Total Count %d\n", i);

	grpc_c_client_stop(client);
	grpc_c_client_wait(client);
    grpc_c_client_free(client);

    grpc_c_shutdown();
}
