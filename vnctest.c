#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

//#define PORT 5900
#define PORT 0

int main()
{
	printf( "hello vnctest\n");
	
	struct sockaddr_in sa;
	int s;
	int port = PORT;
	
	s = socket( PF_INET, SOCK_STREAM, 0);
	int on = 1;
	setsockopt( s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof( on));
	memset( &sa, 0, sizeof( sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons( port);
	bind( s, (struct sockaddr *)&sa, sizeof( sa));
	socklen_t len = sizeof( sa);
	getsockname( s, (struct sockaddr *)&sa, &len);
	port = ntohs( sa.sin_port);
	listen( s, 1);
	
	printf( "listening on port %d..\n", port);
	while (1)
	{
		int cs;
		int n, len;
		char buf[1024];
		
		cs = accept( s, NULL, NULL);
		printf( "client accepted.\n");

		printf( "writing version..\n");
		snprintf( buf, sizeof( buf), "RFB 003.007\n");
		len = strlen( buf);
		n = write( cs, buf, len);

		printf( "reading version..\n");
		len = sizeof( buf);
		n = read( cs, buf, len);
		printf( "read version [%s]\n", buf);

		printf( "writing magic..\n");
		snprintf( buf, sizeof( buf), "\x01\x02");
		len = strlen( buf);
		n = write( cs, buf, len);

		printf( "reading magic..\n");
		len = sizeof( buf);
		n = read( cs, buf, len);
		printf( "read magic [%02x]\n", buf[0]);

		while (1)
		{
			printf( "reading..\n");
			n = read( cs, buf, len);
			printf( "read returned %d\n", n);
			if (n <= 0)
				break;
		}
		printf( "client left.\n");
		close( cs);
	}
	printf( "closing server..\n");
	close( s);

	return 0;
}
