#ifndef VIDEO_H
#define VIDEO_H
#ifdef __cplusplus
#define PROTOHEADER extern "C"
#else
#define PROTOHEADER
#endif
#ifdef __cplusplus
#define SDL_reinterpret_cast(type, expression) reinterpret_cast<type>(expression)
#define SDL_static_cast(type, expression) static_cast<type>(expression)
#else
#define SDL_reinterpret_cast(type, expression) ((type)(expression))
#define SDL_static_cast(type, expression) ((type)(expression))
#endif
#define SDL_VERSION_ATLEAST(X, Y, Z) false
#define Uint64    uint64_t
#define Uint32    uint32_t
#define Uint16    uint16_t
#define Uint8    uint8_t
#define Sint32    int32_t
#define Sint16    int16_t
#define Sint8    int8_t

#include <stdint.h>
#include <graphics/gfx.h>

typedef enum {
	SDL_FALSE = 0,
	SDL_TRUE = 1
} SDL_bool;

typedef struct SDL_Surface {
//	Uint32 flags;				/**< Read-only */
//	SDL_PixelFormat *format;		/**< Read-only */
	int w, h;                /**< Read-only */
//	Uint16 pitch;				/**< Read-only */
	void *pixels;                /**< Read-write */
	int offset;                /**< Private */

	/** Hardware-specific surface info */
//	struct private_hwdata *hwdata;

	/** clipping information */
//	SDL_Rect clip_rect;			/**< Read-only */
//	Uint32 unused1;				/**< for binary compatibility */

	/** Allow recursive locks */
//	Uint32 locked;				/**< Private */

	/** info for fast blit mapping to other surfaces */
//	struct SDL_BlitMap *map;		/**< Private */

	/** format version, bumped at every change to invalidate blit maps */
//	unsigned int format_version;		/**< Private */

	/** Reference count -- used when freeing surface */
//	int refcount;				/**< Read-mostly */} SDL_Surface;
} SDL_Surface;

typedef enum {
	KMOD_NONE = 0x0000,
	KMOD_LSHIFT = 0x0001,
	KMOD_RSHIFT = 0x0002,
	KMOD_LCTRL = 0x0040,
	KMOD_RCTRL = 0x0080,
	KMOD_LALT = 0x0100,
	KMOD_RALT = 0x0200,
	KMOD_LMETA = 0x0400,
	KMOD_RMETA = 0x0800,
	KMOD_NUM = 0x1000,
	KMOD_CAPS = 0x2000,
	KMOD_MODE = 0x4000,
	KMOD_RESERVED = 0x8000
} SDLMod;

typedef struct SDL_Color {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t unused;
} SDL_Color;
#define SDL_Colour SDL_Color

typedef struct SDL_Palette {
	int ncolors;
	SDL_Color *colors;
} SDL_Palette;

typedef struct SDL_Rect {
	int16_t x, y;
	uint16_t w, h;
} SDL_Rect;

PROTOHEADER void waitFrame();
PROTOHEADER void waitFrame2();
PROTOHEADER int test();

PROTOHEADER int SDL_Flip(SDL_Surface *);

PROTOHEADER int SDL_SetColors(SDL_Surface *surface, SDL_Color *colors, int firstcolor, int ncolors);

PROTOHEADER void SDL_Delay(uint32_t ms);

PROTOHEADER void putpixel(unsigned x, unsigned y, int color, SDL_Surface *screen);

PROTOHEADER int SDL_Init(uint32_t flags);

PROTOHEADER char *SDL_GetError(void);

PROTOHEADER int SDL_ShowCursor(int toggle);

PROTOHEADER void SDL_Quit(void);

PROTOHEADER SDL_Surface *SDL_DisplayFormat(SDL_Surface *surface);

#define SDL_SRCCOLORKEY    0x00001000    /**< Blit uses a source color key */

#define SDL_LOGPAL 0x01
#define SDL_PHYSPAL 0x02

#define    SDL_INIT_TIMER        0x00000001
#define SDL_INIT_AUDIO        0x00000010
#define SDL_INIT_VIDEO        0x00000020
#define SDL_INIT_CDROM        0x00000100
#define SDL_INIT_JOYSTICK    0x00000200
#define SDL_INIT_NOPARACHUTE    0x00100000    /**< Don't catch fatal signals */
#define SDL_INIT_EVENTTHREAD    0x01000000    /**< Not supported on all OS's */
#define SDL_INIT_EVERYTHING    0x0000FFFF

/** Available for SDL_CreateRGBSurface() or SDL_SetVideoMode() */
/*@{*/
#define SDL_SWSURFACE    0x00000000    /**< Surface is in system memory */
#define SDL_HWSURFACE    0x00000001    /**< Surface is in video memory */
#define SDL_ASYNCBLIT    0x00000004    /**< Use asynchronous blits if possible */
/*@}*/

/** Available for SDL_SetVideoMode() */
/*@{*/
#define SDL_ANYFORMAT    0x10000000    /**< Allow any video depth/pixel-format */
#define SDL_HWPALETTE    0x20000000    /**< Surface has exclusive palette */
#define SDL_DOUBLEBUF    0x40000000    /**< Set up double-buffered video mode */
#define SDL_FULLSCREEN    0x80000000    /**< Surface is a full screen display */
#define SDL_OPENGL      0x00000002      /**< Create an OpenGL rendering context */
#define SDL_OPENGLBLIT    0x0000000A    /**< Create an OpenGL rendering context and use it for blitting */
#define SDL_RESIZABLE    0x00000010    /**< This video mode may be resized */
#define SDL_NOFRAME    0x00000020    /**< No window caption or edge frame */
/*@}*/

PROTOHEADER void SDL_FreeSurface(SDL_Surface *surface);
PROTOHEADER void c2p(SDL_Surface *src, struct BitMap *output);
PROTOHEADER void c2p2(SDL_Rect *srcrect, void *pixels, struct BitMap *output);
PROTOHEADER void Print(const char *text);
PROTOHEADER int SDL_SetColorKey(SDL_Surface *surface, uint32_t flag, uint32_t key);

PROTOHEADER void ADL_BlitBitPlane(struct BitMap *pixels, SDL_Rect *srcrect, SDL_Rect *dstrect);
PROTOHEADER int SDL_BlitSurface(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect);

PROTOHEADER int ADL_FillRect(SDL_Rect *dstrect, uint32_t color);
PROTOHEADER int SDL_FillRect(SDL_Surface *dst, SDL_Rect *dstrect, uint32_t color);

PROTOHEADER int SDL_SetPalette(SDL_Surface *surface, int flags, SDL_Color *colors, int firstcolor, int ncolors);

PROTOHEADER int ADL_SetPalette(const unsigned char *colors, int firstcolor, int ncolors);


PROTOHEADER SDL_Surface *SDL_SetVideoMode(int width, int height, int bpp, uint32_t flags);

PROTOHEADER SDL_Surface *SDL_CreateRGBSurface(uint32_t flags, int width, int height, int depth, uint32_t Rmask, uint32_t Gmask, uint32_t Bmask, uint32_t Amask);

PROTOHEADER SDL_Surface *SDL_CreateRGBSurfaceFrom(void *pixels, int width, int height, int depth, int pitch, uint32_t Rmask, uint32_t Gmask, uint32_t Bmask, uint32_t Amask);

#endif
