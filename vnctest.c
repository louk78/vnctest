#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

//#define VER 303
#define VER 307
#define SEC SEC_NONE

#define PORT 5901
//#define PORT 0

#define W 20
#define H 20
#define BPP 8
#define DEPTH 8
#define BIG 0
#define TRUECOL 0
#define RMAX 3
#define GMAX 2
#define BMAX 3
#define RSHIFT 5
#define GSHIFT 3
#define BSHIFT 0

enum { SEC_INVALID, SEC_NONE, SEC_VNC };

enum {
	SetPixelFormat,
	SetEncodings=2,
	FramebufferUpdateRequest,
	KeyEvent,
	PointerEvent,
	ClientCutText
} cliser_t;

enum {
	FramebufferUpdate,
	SetColourMapEntries,
	Bell,
	ServerCutText
} sercli_t;

int main()
{
	printf( "hello vnctest\n");
	
	struct sockaddr_in sa;
	int s;
	int port = PORT;
	
	int ver = VER;
	int sec = SEC;
	
	int w, h, bpp, depth, big, truecol;
	int rmax, gmax, bmax;
	int rshift, gshift, bshift;
	
	w = W; h = H; bpp = BPP; depth = DEPTH; big = BIG; truecol = TRUECOL;
	rmax = RMAX; gmax = GMAX; bmax = BMAX;
	rshift = RSHIFT; gshift = GSHIFT; bshift = bshift;
	
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
	
	printf( "listening : vncviewer 127.0.0.1::%d\n", port);
	while (1)
	{
		int cs;
		int n, len;
		char buf[1024];
		
		cs = accept( s, NULL, NULL);
		printf( "client accepted.\n");

		printf( "writing version..\n");
		if (ver >= 308)
			snprintf( buf, sizeof( buf), "RFB 003.008\n");
		else if (ver >= 307)
			snprintf( buf, sizeof( buf), "RFB 003.007\n");
		else
			snprintf( buf, sizeof( buf), "RFB 003.003\n");
		len = strlen( buf);
		n = write( cs, buf, len);

		printf( "reading version..\n");
		len = sizeof( buf);
		n = read( cs, buf, len);
		printf( "read version [%s]\n", buf);
		int vermaj, vermin;
		sscanf( buf, "RFB %d.%d\n", &vermaj, &vermin);
		ver = vermaj * 100 + vermin;

		printf( "writing security..\n");
		if (ver >= 307)
			snprintf( buf, sizeof( buf), "\x01\x01");
		else
			snprintf( buf, sizeof( buf), "\x01");
		len = strlen( buf);
		n = write( cs, buf, len);

		if (sec != SEC_NONE)
		{
			if (ver >= 307)
			{
				printf( "reading security..\n");
				len = 1;
				n = read( cs, buf, len);
				printf( "read security [%02x]\n", buf[0]);
			}

			printf( "writing pass..\n");
			snprintf( buf, sizeof( buf), "123456789abcdef0");
			len = strlen( buf);
			n = write( cs, buf, len);

			printf( "reading pass..\n");
			len = sizeof( buf);
			n = read( cs, buf, len);
			printf( "read pass [%s]\n", buf);

			if (ver >= 308)
			{
				printf( "writing security..\n");
				len = 4;
				memset( buf, 0, len);
				n = write( cs, buf, len);
			}
		}

		printf( "reading ClientInit..\n");
		len = sizeof( buf);
		n = read( cs, buf, len);
		printf( "read ClientInit [%02x]\n", buf[0]);

		typedef struct {
			uint8_t bpp, depth, big, truecol;
			uint16_t rmax, gmax, bmax;
			uint8_t rshift, gshift, bshift;
			uint8_t padding[3];
		} pixel_format_t;
		struct {
			uint16_t w, h;
			pixel_format_t fmt;
			uint32_t name_len;
		} ServerInit;
		printf( "writing ServerInit..\n");
		len = sizeof( ServerInit);
		memset( &ServerInit, 0, len);
		snprintf( buf, sizeof( buf), "hello vnc");
		int len2 = strlen( buf);
		ServerInit.w = w;
		ServerInit.w = h;
		ServerInit.fmt.bpp = bpp;
		ServerInit.fmt.depth = depth;
		ServerInit.fmt.big = big;
		ServerInit.fmt.truecol = truecol;
		ServerInit.fmt.rmax = rmax;
		ServerInit.fmt.gmax = gmax;
		ServerInit.fmt.bmax = bmax;
		ServerInit.fmt.rshift = rshift;
		ServerInit.fmt.gshift = gshift;
		ServerInit.fmt.bshift = bshift;
		ServerInit.name_len = len2;
		n = write( cs, &ServerInit, len);
		n = write( cs, buf, len2);

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
