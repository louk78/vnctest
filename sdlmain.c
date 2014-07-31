#include <stdio.h>

#include <SDL.h>

#include "vnc_proto.h"
#include "vnc.h"

typedef uint32_t pixel_t;
#define TRUECOL	1
#define RMAX	((1 << 8) - 1)
#define GMAX	((1 << 8) - 1)
#define BMAX	((1 << 8) - 1)
#define RSHIFT	16
#define GSHIFT	8
#define BSHIFT	0

#define BIG 0
#define W	200
#define H	200
#define BPP	(sizeof( pixel_t) * 8)

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
//	printf( "%s: opaque=%p len=%d text=[%s]\n", __func__, opaque, len, text);
	printf( "%s: opaque=%p len=%d text=%p\n", __func__, opaque, len, text);

	return 0;
}

vnc_server_init_t init = {
	.port = VNC_DEFAULT_PORT,
	.screen = 0,
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
	printf( "hello sdl+vnc\n");
	SDL_Surface *screen;
	int w = W;
	int h = H;
	int bpp = BPP;
	int flags = SDL_RESIZABLE;

	screen = SDL_SetVideoMode( w, h, bpp, flags);

	init.screen = malloc( w * h * bpp / 8);
	void *p = vnc_init_server( &init);

	int end = 0;
	while (1)
	{
		SDL_Event event;
		while (SDL_PollEvent( &event))
		{
			switch (event.type)
			{
				case SDL_QUIT:
					end = 1;
					break;
				case SDL_KEYUP:
					end = 1;
					break;
				case SDL_VIDEORESIZE:
					w = event.resize.w;
					h = event.resize.h;
					init.width = w;
					init.height = h;
					free( init.screen);
					init.screen = malloc( w * h * bpp / 8);
					screen = SDL_SetVideoMode( w, h, bpp, flags);
					break;
				default:
					break;
			}
		}
		if (end)
			break;
		SDL_UpdateRect( screen, 0, 0, 0, 0);
		vnc_sync( p);
		SDL_Delay( 100);
	}
	vnc_close_server( p);

	return 0;
}
