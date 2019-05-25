/*
 * Copyright (c) 2016, Juniper Networks, Inc.
 * All rights reserved.
 */

#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include "foo.grpc-c.h"


static grpc_c_server_t *test_server;

int test_times = 0;

/*
 * This function gets invoked whenever say_hello RPC gets called
 */
void
foo__greeter__say_hello_cb (grpc_c_context_t *context)
{
	int ret;
    foo__HelloRequest *h;

    /*
     * Read incoming message into h
     */
    if (grpc_c_read(context, (void **)&h, 0, -1)) {
		printf("Failed to read data from client\n");
    }

    if ( h ) {
        foo__hello_request_free(h);
    }

    /*
     * Create a reply
     */
    foo__HelloReply r;
    foo__hello_reply__init(&r);

    char buf[1024];
    buf[0] = '\0';
    snprintf(buf, 1024, "hello, world! from server.");
    r.message = buf;
    
    /*
     * Write reply back to the client
     */

    ret = grpc_c_write(context, &r, 0, -1);
    if ( ret ) {
        printf("Failed to write %d\n", ret);
    }

    grpc_c_status_t status;
    status.code = 0;
	status.message[0] = '\0';

    /*
     * Finish response for RPC
     */
    if (grpc_c_server_finish(context, &status, 0)) {
        printf("Failed to write status\n");
    }

    test_times++;
    if ( test_times >= 100 )
    {
        grpc_c_server_stop(test_server);
    }
}

/*
 * Takes socket path as argument
 */
int foo_server() 
{
    int i = 0;

    /*
     * Initialize grpc-c library to be used with vanilla gRPC
     */
    grpc_c_init();
    
    /*
     * Create server object
     */
    test_server = grpc_c_server_create("127.0.0.1:3000", NULL, NULL);
    if (test_server == NULL) {
		printf("Failed to create server\n");
		exit(1);
    }

    /*
     * Initialize greeter service
     */
    foo__greeter__service_init(test_server);

    /*
     * Start server
     */
    grpc_c_server_start(test_server);

    /*
     * Blocks server to wait to completion
     */
    grpc_c_server_wait(test_server);

    /*
     * Destory server
     */
    grpc_c_server_destroy(test_server);

    /*
     * Destory grpc-c library.
     */
    grpc_c_shutdown();
}
