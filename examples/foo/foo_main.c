#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include "foo.grpc-c.h"

int foo_server();
int foo_client();

int main(int argv, char ** args) {
	
	if ( argv < 2) {
		printf("usage: <server/client>\n");
		return -1;
	}
	
	if ( 0 == strcmp(args[1],"server") ) {
		foo_server();
	}else {
		foo_client();
	}

    sleep(1);
	
	return 0;
}