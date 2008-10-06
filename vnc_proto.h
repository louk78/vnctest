#ifndef __VNC_PROTO_H__
#define __VNC_PROTO_H__

#include <stdlib.h>
#include <stdint.h>

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
	uint8_t bpp, depth, big, truecol;
	uint16_t rmax, gmax, bmax;
	uint8_t rshift, gshift, bshift;
	uint8_t padding[3];
} vnc_pixel_format_t;
typedef struct {
	uint16_t w, h;
	vnc_pixel_format_t fmt;
	uint32_t name_len;
} ServerInit_t;
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

#endif/*__VNC_H__*/
