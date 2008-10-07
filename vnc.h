#ifndef __VNC_H__
#define __VNC_H__

//#define VNC_HAVE_ASYNC
#define VNC_DEFAULT_PORT 5901

typedef int vnc_pointer_event_cb_t( void *opaque, int bmask, int xpos, int ypos);
typedef int vnc_key_event_cb_t( void *opaque, int down, int key);
typedef int vnc_client_cut_text_cb_t( void *opaque, int len, char *text);

typedef struct {
	void *opaque;
	int port;					// use VNC_DEFAULT_PORT or 0 for dynamic value
	char *name;
	void *screen;
	int width, height;
	vnc_pixel_format_t fmt;		// warning : big endian contents !!!!

	vnc_pointer_event_cb_t *pointer_event;
	vnc_key_event_cb_t *key_event;
	vnc_client_cut_text_cb_t *client_cut_text;
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

