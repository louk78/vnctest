#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdint.h>
#include <inttypes.h>

#include "vnc_proto.h"
#include "vnc.h"

typedef struct {
	vnc_server_init_t *init;
	int ss, cs;
	vnc_pixel_format_t client_fmt;
} vnc_t;

static int FrameBufferUpdate( vnc_t *vnc, int xpos, int ypos, int width, int height)
{
	fbupdate_t fbupdate;
	rec_t rec;

	int bpp = vnc->client_fmt.bpp;
	unsigned char *screen = vnc->init->screen;
	int W = vnc->init->width;

//	printf( "%s: %d %d:%d %dx%d\n", __func__, bpp, xpos, ypos, width, height);
	int n;
	int len;
	fbupdate.type = scFramebufferUpdate;
	fbupdate.nrec = BE16(1);
	len = sizeof( fbupdate);
	n = write( vnc->cs, &fbupdate, len);
	if (n <= 0)
		return -1;
	int num;
	rec.xpos = BE16(xpos);
	rec.ypos = BE16(ypos);
	rec.width = BE16(width);
	rec.height = BE16(height);
	rec.type = BE16(0);
	len = sizeof( rec);
	n = write( vnc->cs, &rec, len);
	if (n <= 0)
		return -1;
	num = width * height * bpp / 8;
	int i, j;
	for (j = 0; j < height; j++)
	{
		for (i = 0; i < width; i++)
		{
			uint32_t pixel;
			
#if 0
			pixel = !!screen[(ypos + j) * W + (xpos + i)] * 0xFF;
//			pixel <<= 16; // R
//			pixel <<= 8; // G
			pixel <<= 0; // B
#else
			pixel = screen[(ypos + j) * W + (xpos + i)];
			pixel = ((!!(pixel & 0xe0) * 0xFF) << 16) + ((!!(pixel & 0x18) * 0xFF) << 8) + ((!!(pixel & 0x07) * 0xFF) << 0);
#endif
//			printf( " %x", pixel);
			n = write( vnc->cs, &pixel, sizeof( pixel));
			if (n <= 0)
				return -1;
		}
	}
//	printf( "\n");

	return 0;
}

void *vnc_init_server( vnc_server_init_t *init)
{
	void *result = NULL;

	printf( "%s\n", __func__);
	vnc_t *vnc = malloc( sizeof( *vnc));
	if (vnc)
	{
		memset( vnc, 0, sizeof( *vnc));
		vnc->ss = vnc->cs = -1;
		vnc->init = init;

		struct sockaddr_in sa;

		vnc->ss = socket( PF_INET, SOCK_STREAM, 0);
		int on = 1;
		setsockopt( vnc->ss, SOL_SOCKET, SO_REUSEADDR, &on, sizeof( on));
		memset( &sa, 0, sizeof( sa));
		sa.sin_family = AF_INET;
		sa.sin_port = htons( vnc->init->port);
		bind( vnc->ss, (struct sockaddr *)&sa, sizeof( sa));
		socklen_t len = sizeof( sa);
		getsockname( vnc->ss, (struct sockaddr *)&sa, &len);
		vnc->init->port = ntohs( sa.sin_port);
		listen( vnc->ss, 1);
	
		printf( "listening : vncviewer 127.0.0.1::%d\n", vnc->init->port);

		result = vnc;
	}

	return result;
}

int vnc_close_server( void *opaque)
{
	vnc_t *vnc = opaque;

	printf( "%s\n", __func__);
	if (vnc)
	{
		if (vnc->ss != -1)
			close( vnc->ss);
		free( vnc);
	}

	return 0;
}

#if 1
#define VER 303
#else
#define VER 307
#endif
#define SEC SEC_NONE

static int ver = VER;
static int sec = SEC;

static int client_init( vnc_t *vnc)
{
	char buf[1024];
	int len;
	int n;

	printf( "%s: cs=%d ss=%d\n", __func__, vnc->cs, vnc->ss);
	printf( "writing version..\n");
//	getchar();
	if (ver >= 308)
		snprintf( buf, sizeof( buf), "RFB 003.008\n");
	else if (ver >= 307)
		snprintf( buf, sizeof( buf), "RFB 003.007\n");
	else
		snprintf( buf, sizeof( buf), "RFB 003.003\n");
	len = strlen( buf);
	n = write( vnc->cs, buf, len);
	printf( "wrote %d bytes\n", n);

	printf( "reading version..\n");
	len = sizeof( buf);
	memset( buf, 0xDA, sizeof( buf));
	n = read( vnc->cs, buf, len);
	printf( "read %d bytes\n", n);
	if (n < 0)
	{
		perror( "read");
		return -1;
	}
	printf( "read version [%s]\n", buf);
	int vermaj, vermin;
	sscanf( buf, "RFB %d.%d\n", &vermaj, &vermin);
	ver = vermaj * 100 + vermin;
	printf( "ver=%d\n", ver);

	printf( "writing security..\n");
//	getchar();
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
	n = write( vnc->cs, buf, len);
	printf( "wrote %d bytes\n", n);

	if (sec != SEC_NONE)
	{
		if (ver >= 307)
		{
			printf( "reading security..\n");
			len = 1;
			n = read( vnc->cs, buf, len);
			printf( "read %d bytes\n", n);
			printf( "read security [%02x]\n", buf[0]);
		}

		printf( "writing pass..\n");
		getchar();
		snprintf( buf, sizeof( buf), "123456789abcdef0");
		len = strlen( buf);
		n = write( vnc->cs, buf, len);
		printf( "wrote %d bytes\n", n);

		printf( "reading pass..\n");
		len = sizeof( buf);
		n = read( vnc->cs, buf, len);
		printf( "read %d bytes\n", n);
		printf( "read pass [%s]\n", buf);

		if (ver >= 308)
		{
			printf( "writing security..\n");
			getchar();
			len = 4;
			memset( buf, 0, len);
			n = write( vnc->cs, buf, len);
			printf( "wrote %d bytes\n", n);
		}
	}

	printf( "reading ClientInit..\n");
	len = sizeof( buf);
	n = read( vnc->cs, buf, len);
	printf( "read %d bytes\n", n);
	printf( "read ClientInit [%02x]\n", buf[0]);

	ServerInit_t ServerInit;
	printf( "writing ServerInit..\n");
	getchar();
	len = sizeof( ServerInit);
	memset( &ServerInit, 0, len);
	snprintf( buf, sizeof( buf), "hello vnc");
	int len2 = strlen( buf);
	ServerInit.w = BE16( vnc->init->width);
	ServerInit.h = BE16( vnc->init->height);
	ServerInit.fmt.bpp = vnc->init->fmt.bpp;
	ServerInit.fmt.depth = vnc->init->fmt.depth;
	ServerInit.fmt.big = vnc->init->fmt.big;
	ServerInit.fmt.truecol = vnc->init->fmt.truecol;
	ServerInit.fmt.rmax = BE16( vnc->init->fmt.rmax);
	ServerInit.fmt.gmax = BE16( vnc->init->fmt.gmax);
	ServerInit.fmt.bmax = BE16( vnc->init->fmt.bmax);
	ServerInit.fmt.rshift = vnc->init->fmt.rshift;
	ServerInit.fmt.gshift = vnc->init->fmt.gshift;
	ServerInit.fmt.bshift = vnc->init->fmt.bshift;
	ServerInit.name_len = BE32(len2);
	n = write( vnc->cs, &ServerInit, len);
	printf( "wrote %d bytes\n", n);
	n = write( vnc->cs, buf, len2);
	printf( "wrote %d bytes\n", n);

	return 0;
}

static int client_manage( vnc_t *vnc)
{
	int result = 0;
	pixfmt_t pixfmt;
	int type;
	int n;
	char buf[1024];

	int cs = vnc->cs;
	int len;
	int end = 0;

//	printf( "reading type..\n");
	n = read( cs, buf, 1);
	if (n <= 0)
	{
		end = 1;
	}
//	printf( "read returned %d\n", n);
	type = buf[0];
	switch (type)
	{
		case csFramebufferUpdateRequest:
		{
			fbupdatereq_t fbupdatereq;
//			printf( "reading framebufferupdaterequest event..\n");
			len = sizeof( fbupdatereq);
			n = read( cs, &fbupdatereq, len);
			if (n <= 0)
			{
				end = 1;
				break;
			}
//			printf( "read returned %d\n", n);
			int x, y, w, h;
			x = HE16(fbupdatereq.xpos);
			y = HE16(fbupdatereq.ypos);
			w = HE16(fbupdatereq.width);
			h = HE16(fbupdatereq.height);
//			printf( "framebufferupdaterequest event : incr=%d xpos=%" PRId16 " ypos=%" PRId16 " width=%" PRId16 " height=%" PRId16 "\n", fbupdatereq.incr, x, y, w, h);

			if (FrameBufferUpdate( vnc, x, y, w, h) < 0)
			{
				end = 1;
				break;
			}
		}
			break;
		case csSetEncodings:
		{
			encodings_t encodings;
//			printf( "reading setencodings event..\n");
			len = sizeof( encodings);
			n = read( cs, &encodings, len);
			if (n <= 0)
			{
				end = 1;
				break;
			}
//			printf( "read returned %d\n", n);
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
//				printf( "read returned %d\n", n);
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
//			printf( "reading pointer event..\n");
			len = sizeof( pointer);
			n = read( cs, &pointer, len);
			if (n <= 0)
				end = 1;
//				printf( "read returned %d\n", n);
//				printf( "pointer event : bmask=%d xpos=%" PRId16 "ypos=%" PRId16 "\n", pointer.bmask, HE16(pointer.xpos), HE16(pointer.ypos));
		}
			break;
		case csKeyEvent:
		{
			keyev_t key;
//			printf( "reading key event..\n");
			len = sizeof( key);
			n = read( cs, &key, len);
			if (n <= 0)
				end = 1;
//			printf( "read returned %d\n", n);
			printf( "key event : down=%d key=%" PRIx32 "\n", key.down, HE32(key.key));
			if (HE32(key.key) == 0xff1b)
				end = 1;
		}
			break;
		case csClientCutText:
		{
			ccuttext_t cut;
//			printf( "reading cut event..\n");
			len = sizeof( cut);
			n = read( cs, &cut, len);
			if (n <= 0)
				end = 1;
//			printf( "read returned %d\n", n);
			len = HE32(cut.len);
			printf( "cut event : len=%" PRId32 "\n", len);
			n = read( cs, buf, len);
			if (n <= 0)
				end = 1;
//			printf( "read returned %d\n", n);
			printf( "cut event : len=%" PRId32 "\n", HE32(cut.len));
		}
			break;
#if 0
		case 1:
			printf( "WARNING !!! SILENTLY SKIPPING UNKNOWN MESSAGE TYPE %d (end=%d)\n", type, end);
			break;
#endif
		default:
			printf( "unknown cs message type %d\n", type);
			end = 1;
			break;
	}
	if (end)
		result = -1;

	return result;
}

int vnc_sync( void *opaque)
{
	int result = 0;
	vnc_t *vnc = opaque;

//	printf( "%s\n", __func__);
#if 0
	static int count = 0;
	if (count++ > 5)
		result = -1;
#else
	if (vnc)
	{
		int n;
		int max = 0;
		fd_set rfds;
		struct timeval tv;
		
		FD_ZERO( &rfds);
		if (vnc->ss != -1)
		{
			FD_SET( vnc->ss, &rfds);
			if (vnc->ss > max)
				max = vnc->ss;
		}
		if (vnc->cs != -1)
		{
			FD_SET( vnc->cs, &rfds);
			if (vnc->cs > max)
				max = vnc->cs;
		}
		if (max > 0)
		{
			max++;
			tv.tv_usec = 1000;
			tv.tv_sec = 0;
			n = select( max, &rfds, NULL, NULL, &tv);
			if (n < 0)
				result = -1;
			else if (n == 0);
			else
			{
				if (vnc->ss != -1)
				{
					if (FD_ISSET( vnc->ss, &rfds))
					{
						int cs;
						cs = accept( vnc->ss, NULL, NULL);
						if (vnc->cs == -1)
						{
							vnc->cs = cs;
							printf( "accepted client\n");
							if (client_init( vnc) < 0)
								return -1;
						}
						else
							close( cs);
					}
				}
				if (vnc->cs != -1)
				{
					if (FD_ISSET( vnc->cs, &rfds))
					{
						if (client_manage( vnc) < 0)
						{
							printf( "client has disappeared ?\n");
							close( vnc->cs);
							vnc->cs = -1;
						}
					}
				}
			}
		}
	}
#endif

	return result;
}

