#include "simplehttpd.h"

// Closes socket before closing
void catch_ctrlc(int sig)
{
	printf("Server terminating\n");
	close(socket_conn);
	exit(0);
}