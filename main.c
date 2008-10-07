#include <stdio.h>
#include <unistd.h>

#include "vnc_proto.h"
#include "vnc.h"

#if 1					/* 32/24 bit rgb888 */
typedef uint32_t pixel_t;
#define TRUECOL	1
#define RMAX	((1 << 8) - 1)
#define GMAX	((1 << 8) - 1)
#define BMAX	((1 << 8) - 1)
#define RSHIFT	16
#define GSHIFT	8
#define BSHIFT	0
#elif 0					/* 16 bit 6:5:5 */
typedef uint16_t pixel_t;
#else					/* 8 bit rgb323 */
typedef uint8_t pixel_t;
#define TRUECOL	0
#define RMAX	((1 << 3) - 1)
#define GMAX	((1 << 2) - 1)
#define BMAX	((1 << 3) - 1)
#define RSHIFT	5
#define GSHIFT	3
#define BSHIFT	0
#endif
//#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MIN(a,b) ((a) ? (b) : 0)
#define _(r,g,b) ((MIN(r,RMAX) << RSHIFT) + (MIN(g,GMAX) << GSHIFT) + (MIN(b,BMAX) << BSHIFT)),

#define BIG 0
#define W	10
#define H	10
#define BPP	(sizeof( pixel_t) * 8)

pixel_t screen[W * H * BPP / 8] = {
#if 0 /* mire */
_(0,0,0)_(1,0,0)_(1,0,0)_(1,0,0)_(1,0,0)_(1,0,0)_(1,0,0)_(1,0,0)_(1,0,0)_(0,0,0)
_(1,0,0)_(0,0,0)_(0,0,0)_(0,0,0)_(0,0,0)_(0,0,0)_(0,0,0)_(0,0,0)_(0,0,0)_(1,0,0)
_(1,0,0)_(0,0,0)_(0,1,0)_(0,1,0)_(0,1,0)_(0,1,0)_(0,1,0)_(0,1,0)_(0,0,0)_(1,0,0)
_(1,0,0)_(0,0,0)_(0,1,0)_(0,0,0)_(0,0,0)_(0,0,0)_(0,0,0)_(0,1,0)_(0,0,0)_(1,0,0)
_(1,0,0)_(0,0,0)_(0,1,0)_(0,0,0)_(0,0,1)_(0,0,1)_(0,0,0)_(0,1,0)_(0,0,0)_(1,0,0)
_(1,0,0)_(0,0,0)_(0,1,0)_(0,0,0)_(0,0,1)_(0,0,1)_(0,0,0)_(0,1,0)_(0,0,0)_(1,0,0)
_(1,0,0)_(0,0,0)_(0,1,0)_(0,0,0)_(0,0,0)_(0,0,0)_(0,0,0)_(0,1,0)_(0,0,0)_(1,0,0)
_(1,0,0)_(0,0,0)_(0,1,0)_(0,1,0)_(0,1,0)_(0,1,0)_(0,1,0)_(0,1,0)_(0,0,0)_(1,0,0)
_(1,0,0)_(0,0,0)_(0,0,0)_(0,0,0)_(0,0,0)_(0,0,0)_(0,0,0)_(0,0,0)_(0,0,0)_(1,0,0)
_(0,0,0)_(1,0,0)_(1,0,0)_(1,0,0)_(1,0,0)_(1,0,0)_(1,0,0)_(1,0,0)_(1,0,0)_(0,0,0)
#else /* flag */
_(1,0,0)_(1,0,0)_(1,0,0)_(0,1,0)_(0,1,0)_(0,1,0)_(0,1,0)_(0,0,1)_(0,0,1)_(0,0,1)
_(1,0,0)_(1,0,0)_(1,0,0)_(0,1,0)_(0,1,0)_(0,1,0)_(0,1,0)_(0,0,1)_(0,0,1)_(0,0,1)
_(1,0,0)_(1,0,0)_(1,0,0)_(0,1,0)_(0,1,0)_(0,1,0)_(0,1,0)_(0,0,1)_(0,0,1)_(0,0,1)
_(1,0,0)_(1,0,0)_(1,0,0)_(0,1,0)_(0,1,0)_(0,1,0)_(0,1,0)_(0,0,1)_(0,0,1)_(0,0,1)
_(1,0,0)_(1,0,0)_(1,0,0)_(0,1,0)_(0,1,0)_(0,1,0)_(0,1,0)_(0,0,1)_(0,0,1)_(0,0,1)
_(1,0,0)_(1,0,0)_(1,0,0)_(0,1,0)_(0,1,0)_(0,1,0)_(0,1,0)_(0,0,1)_(0,0,1)_(0,0,1)
_(1,0,0)_(1,0,0)_(1,0,0)_(0,1,0)_(0,1,0)_(0,1,0)_(0,1,0)_(0,0,1)_(0,0,1)_(0,0,1)
_(1,0,0)_(1,0,0)_(1,0,0)_(0,1,0)_(0,1,0)_(0,1,0)_(0,1,0)_(0,0,1)_(0,0,1)_(0,0,1)
_(1,0,0)_(1,0,0)_(1,0,0)_(0,1,0)_(0,1,0)_(0,1,0)_(0,1,0)_(0,0,1)_(0,0,1)_(0,0,1)
_(1,0,0)_(1,0,0)_(1,0,0)_(0,1,0)_(0,1,0)_(0,1,0)_(0,1,0)_(0,0,1)_(0,0,1)_(0,0,1)
#endif
};

int vnc_pointer_event( void *opaque, int bmask, int xpos, int ypos)
{
	printf( "%s: opaque=%p bmask=%d xpos=%d ypos=%d\n", __func__, opaque, bmask, xpos, ypos);

	return 0;
}

int vnc_key_event( void *opaque, int down, int key)
{
	printf( "%s: opaque=%p down=%d key=%d\n", __func__, opaque, down, key);

	return 0;
}

int vnc_client_cut_text( void *opaque, int len, char *text)
{
	printf( "%s: opaque=%p len=%d text=[%s]\n", __func__, opaque, len, text);

	return 0;
}

vnc_server_init_t init = {
	.port = VNC_DEFAULT_PORT,
	.screen = &screen,
	.name = "hello vnc",
	.width = W,
	.height = H,
	.fmt = {
		.bpp = BPP,
		.depth = BPP,
		.big = BIG,
		.truecol = TRUECOL,
		.rmax = RMAX,
		.gmax = GMAX,
		.bmax = BMAX,
		.rshift = RSHIFT,
		.gshift = GSHIFT,
		.bshift = BSHIFT,
	},
	.pointer_event = vnc_pointer_event,
	.key_event = vnc_key_event,
	.client_cut_text = vnc_client_cut_text,
};

int main( int argc, char *argv[])
{
	int arg = 1;
	
	while (arg < argc)
	{
		sscanf( argv[arg], "%d", &init.port);
		break;
	}

	void *p = vnc_init_server( &init);
#ifdef VNC_HAVE_ASYNC
	vnc_async( p);
#endif
	while (1)
	{
#ifdef VNC_HAVE_ASYNC
		if (vnc_lock( p) < 0)
			break;
#else
		if (vnc_sync( p) < 0)
			break;
#endif
		/* update screen here */
		static int count = 0;
		*((unsigned char *)screen + count) = 0xFF;
		count++;
		usleep( 1);
#ifdef VNC_HAVE_ASYNC
		if (vnc_unlock( p) < 0)
			break;
#endif
	}

	vnc_close_server( p);

	return 0;
}
