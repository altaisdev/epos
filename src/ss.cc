/* Please ignore. Stupid client only. */

#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <errno.h>

main(int argc, char **argv)
{
	int s;
	sockaddr_in addr;

	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == -1) printf("No socket\n");
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(8778);
	addr.sin_addr.s_addr = htonl(0x7f000001);
	if (connect(s, (sockaddr *)&addr, sizeof(addr))) {
		printf("Could not connect %s\n", sys_errlist[errno]);
		printf("Make sure ssd is running!\n");
		exit(1);
	}
	char buffer [4096]="";
	for (int i=1; i<argc; i++) {
		strcat(buffer, argv[i]);
		strcat(buffer, "\034");
	}
	write(s, buffer, 3200);
	read(s, buffer, 1);
	close(s);
}
