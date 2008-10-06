#ifndef __VNC_H__
#define __VNC_H__

//#define VNC_HAVE_ASYNC
#define VNC_DEFAULT_PORT 5901


typedef struct {
	int port;					// use VNC_DEFAULT_PORT or 0 for dynamic value
	char *name;
	void *screen;
	int width, height;
	vnc_pixel_format_t fmt;		// warning : big endian contents !!!!
} vnc_server_init_t;

void *vnc_init_server( vnc_server_init_t *init);
int vnc_close_server( void *opaque);
// async api
#ifdef VNC_HAVE_ASYNC
int vnc_async( void *opaque);
int vnc_lock( void *opaque);
int vnc_unlock( void * opaque);
#endif
// sync api
int vnc_sync( void *opaque);

#endif/*__VNC_H__*/

