#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdint.h>
#include <inttypes.h>

#if 1
#define VER 303
#else
#define VER 307
#endif
#define SEC SEC_NONE

#define PORT 5901
//#define PORT 0

#define W 10
#define H 10
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

#define BE32(v) ((uint32_t)htonl( v))
#define BE16(v) ((uint16_t)htons( v))

#define HE32(v) ((uint32_t)ntohl( v))
#define HE16(v) ((uint16_t)ntohs( v))

enum { SEC_INVALID, SEC_NONE, SEC_VNC };

enum {
	csSetPixelFormat,
	csSetEncodings=2,
	csFramebufferUpdateRequest,
	csKeyEvent,
	csPointerEvent,
	csClientCutText
} cliser_t;

enum {
	scFramebufferUpdate,
	scSetColourMapEntries,
	scBell,
	scServerCutText
} sercli_t;

#pragma pack(1)
typedef struct {
	uint8_t type;
	uint8_t padding0[1];
	uint16_t nrec;
} fbupdate_t;
typedef struct {
	uint16_t xpos;
	uint16_t ypos;
	uint16_t width;
	uint16_t height;
	int32_t type;
} rec_t;
typedef struct {
	uint8_t bpp, depth, big, truecol;
	uint16_t rmax, gmax, bmax;
	uint8_t rshift, gshift, bshift;
	uint8_t padding[3];
} pixel_format_t;
typedef struct {
	uint16_t w, h;
	pixel_format_t fmt;
	uint32_t name_len;
} ServerInit_t;
typedef struct {
	uint8_t padding0[1];
	uint16_t nenc;
} encodings_t;
typedef struct {
	uint8_t incr;
	uint16_t xpos;
	uint16_t ypos;
	uint16_t width;
	uint16_t height;
} fbupdatereq_t;
typedef struct {
	uint8_t padding0[3];
	uint8_t bpp;
	uint8_t depth;
	uint8_t big;
	uint8_t truecol;
	uint16_t rmax;
	uint16_t gmax;
	uint16_t bmax;
	uint8_t rshift;
	uint8_t gshift;
	uint8_t bshift;
	uint8_t padding1[3];
} pixfmt_t;
typedef struct {
	uint8_t down;
	uint8_t padding[2];
	uint32_t key;
} keyev_t;
typedef struct {
	uint8_t padding[3];
	uint32_t len;
} ccuttext_t;
typedef struct {
	uint8_t bmask;
	uint16_t xpos;
	uint16_t ypos;
} pointerev_t;
#pragma pack()

unsigned char screen[BPP / 8 * W * H] = 
{
	0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
};

int FrameBufferUpdate( int fd, int bpp, int xpos, int ypos, int width, int height)
{
	fbupdate_t fbupdate;
	rec_t rec;

//	printf( "%s: %d %d:%d %dx%d\n", __func__, bpp, xpos, ypos, width, height);
	int n;
	int len;
	fbupdate.type = scFramebufferUpdate;
	fbupdate.nrec = BE16(1);
	len = sizeof( fbupdate);
	n = write( fd, &fbupdate, len);
	if (n <= 0)
		return -1;
	int num;
	rec.xpos = BE16(xpos);
	rec.ypos = BE16(ypos);
	rec.width = BE16(width);
	rec.height = BE16(height);
	rec.type = BE16(0);
	len = sizeof( rec);
	n = write( fd, &rec, len);
	if (n <= 0)
		return -1;
	num = width * height * bpp / 8;
	int i, j;
	for (j = 0; j < height; j++)
	{
		for (i = 0; i < width; i++)
		{
			uint32_t pixel;
			
			pixel = !!screen[(ypos + j) * W + (xpos + i)] * 0xFF;
//			pixel <<= 16; // R
//			pixel <<= 8; // G
			pixel <<= 0; // B
			printf( " %x", pixel);
			n = write( fd, &pixel, sizeof( pixel));
			if (n <= 0)
				return -1;
		}
	}
	printf( "\n");

	return 0;
}

int main()
{
//	memset( &screen, 0, sizeof( screen));
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
		{
			snprintf( buf, sizeof( buf), "\x01\x01");
			len = strlen( buf);
		}
		else
		{
			uint32_t sec_type = BE32(sec);
			len = sizeof( sec_type);
			memcpy( buf, &sec_type, len);
		}
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

		ServerInit_t ServerInit;
		printf( "writing ServerInit..\n");
		len = sizeof( ServerInit);
		memset( &ServerInit, 0, len);
		snprintf( buf, sizeof( buf), "hello vnc");
		int len2 = strlen( buf);
		ServerInit.w = BE16(w);
		ServerInit.h = BE16(h);
		ServerInit.fmt.bpp = bpp;
		ServerInit.fmt.depth = depth;
		ServerInit.fmt.big = big;
		ServerInit.fmt.truecol = truecol;
		ServerInit.fmt.rmax = BE16(rmax);
		ServerInit.fmt.gmax = BE16(gmax);
		ServerInit.fmt.bmax = BE16(bmax);
		ServerInit.fmt.rshift = rshift;
		ServerInit.fmt.gshift = gshift;
		ServerInit.fmt.bshift = bshift;
		ServerInit.name_len = BE32(len2);
		n = write( cs, &ServerInit, len);
		n = write( cs, buf, len2);

		int end = 0;
		while (!end)
		{
			pixfmt_t pixfmt;
			int type;

//			printf( "reading type..\n");
			n = read( cs, buf, 1);
			if (n <= 0)
				break;
//			printf( "read returned %d\n", n);
			type = buf[0];
			switch (type)
			{
				case csFramebufferUpdateRequest:
				{
					fbupdatereq_t fbupdatereq;
//					printf( "reading framebufferupdaterequest event..\n");
					len = sizeof( fbupdatereq);
					n = read( cs, &fbupdatereq, len);
					if (n <= 0)
					{
						end = 1;
						break;
					}
//					printf( "read returned %d\n", n);
					int x, y, w, h;
					x = HE16(fbupdatereq.xpos);
					y = HE16(fbupdatereq.ypos);
					w = HE16(fbupdatereq.width);
					h = HE16(fbupdatereq.height);
//					printf( "framebufferupdaterequest event : incr=%d xpos=%" PRId16 " ypos=%" PRId16 " width=%" PRId16 " height=%" PRId16 "\n", fbupdatereq.incr, x, y, w, h);
					
					if (FrameBufferUpdate( cs, pixfmt.bpp, x, y, w, h) < 0)
					{
						end = 1;
						break;
					}
				}
					break;
				case csSetEncodings:
				{
					encodings_t encodings;
//					printf( "reading setencodings event..\n");
					len = sizeof( encodings);
					n = read( cs, &encodings, len);
					if (n <= 0)
					{
						end = 1;
						break;
					}
//					printf( "read returned %d\n", n);
					printf( "setencodings event : nenc=%d\n", HE16(encodings.nenc));
					printf( "encodings :");
					int i;
					for (i = 0; i < HE16(encodings.nenc); i++)
					{
						int32_t type;
						len = sizeof( type);
						n = read( cs, &type, len);
						if (n <= 0)
						{
							end = 1;
							break;
						}
//						printf( "read returned %d\n", n);
						printf( " %" PRId32, HE32(type));
					}
					printf( "\n");
				}
					break;
				case csSetPixelFormat:
				{
					printf( "reading setpixelformat event..\n");
					len = sizeof( pixfmt);
					n = read( cs, &pixfmt, len);
					if (n <= 0)
						end = 1;
					printf( "read returned %d\n", n);
					printf( "setpixelformat event : bpp=%d depth=%d big=%d truecol=%d rmax=%" PRIx16 " gmax=%" PRIx16 " bmax=%" PRIx16 " rshift=%d gshift=%d bshift=%d\n",
						pixfmt.bpp, pixfmt.depth, pixfmt.big, pixfmt.truecol, HE16(pixfmt.rmax), HE16(pixfmt.gmax), HE16(pixfmt.bmax), pixfmt.rshift, pixfmt.gshift, pixfmt.bshift);
				}
					break;
				case csPointerEvent:
				{
					pointerev_t pointer;
//					printf( "reading pointer event..\n");
					len = sizeof( pointer);
					n = read( cs, &pointer, len);
					if (n <= 0)
						end = 1;
//					printf( "read returned %d\n", n);
//					printf( "pointer event : bmask=%d xpos=%" PRId16 "ypos=%" PRId16 "\n", pointer.bmask, HE16(pointer.xpos), HE16(pointer.ypos));
				}
					break;
				case csKeyEvent:
				{
					keyev_t key;
//					printf( "reading key event..\n");
					len = sizeof( key);
					n = read( cs, &key, len);
					if (n <= 0)
						end = 1;
//					printf( "read returned %d\n", n);
					printf( "key event : down=%d key=%" PRIx32 "\n", key.down, HE32(key.key));
				}
					break;
				case csClientCutText:
				{
					ccuttext_t cut;
//					printf( "reading cut event..\n");
					len = sizeof( cut);
					n = read( cs, &cut, len);
					if (n <= 0)
						end = 1;
//					printf( "read returned %d\n", n);
					len = HE32(cut.len);
					printf( "cut event : len=%" PRId32 "\n", len);
					n = read( cs, buf, len);
					if (n <= 0)
						end = 1;
//					printf( "read returned %d\n", n);
					printf( "cut event : len=%" PRId32 "\n", HE32(cut.len));
				}
					break;
				default:
					printf( "unknown cs message type %02x\n", type);
					end = 1;
					break;
			}
		}
		printf( "client left.\n");
		close( cs);
//		break;
	}
	printf( "closing server..\n");
	close( s);

	return 0;
}
