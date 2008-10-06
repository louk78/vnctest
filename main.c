#include <stdio.h>

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
};

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

	while (1)
	{
		if (vnc_sync( p) < 0)
			break;
	}

	vnc_close_server( p);

	return 0;
}
