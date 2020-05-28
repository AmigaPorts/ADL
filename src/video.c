
#include <exec/exec.h>
#include <dos/dos.h>
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <graphics/gfxmacros.h>
#include <proto/intuition.h>
#include <libraries/asl.h>
#include <libraries/lowlevel.h>
#include <devices/gameport.h>
#include <devices/timer.h>
#include <devices/keymap.h>
#include <devices/input.h>
#include <hardware/custom.h>
//#include <hardware/cia.h>
#include <unistd.h>


#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/graphics.h>

#include <cybergraphx/cybergraphics.h>
#include <proto/cybergraphics.h>
#include <math.h>
#include "video.h"
#include "timer.h"
#include "canvas.h"
#include "control.h"
#include <string.h>
#include <stdio.h>

#define REG(xn, parm) parm __asm(#xn)
#define REGARGS __regargs
#define STDARGS __stdargs
#define SAVEDS __saveds
#define ALIGNED __attribute__ ((aligned(4))
#define FAR
#define CHIP
#define INLINE __inline__

#define PLOG(fmt, args...) do {   fprintf(stdout, fmt, ## args); } while (0)

extern void REGARGS c2p1x1_8_c5_bm_040(REG(d0, UWORD chunky_x), REG(d1, UWORD chunky_y), REG(d2, UWORD offset_x), REG(d3, UWORD offset_y), REG(a0, UBYTE *chunky_buffer), REG(a1, struct BitMap *bitmap));

extern void REGARGS c2p1x1_8_c5_bm(REG(d0, UWORD chunky_x), REG(d1, UWORD chunky_y), REG(d2, UWORD offset_x), REG(d3, UWORD offset_y), REG(a0, UBYTE *chunky_buffer), REG(a1, struct BitMap *bitmap));

/** START TEST **/
extern struct Custom *custom = (struct Custom *)0xdff000;
//volatile struct CIA *ciaa = (struct CIA *)0xbfe001;

#define COP_MOVE(addr, data) addr, data
#define COP_WAIT 0xfffe
#define COP_WAIT_1_3 0x7c07, COP_WAIT
#define COP_WAIT_2_3 0xda07, COP_WAIT
#define COP_WAIT_END 0xffff, COP_WAIT
#define COLOR00 (0x180)
#define BPLCON0 (0x100)
#define BPLCON0_COMPOSITE_COLOR (0 << 9)
#define GETBYTE_HI(x) (UBYTE)(x >> 8)
#define GETBYTE_LO(x) (UBYTE)(x & 0xff)

static UWORD __chip coplist[] = {
		COP_MOVE(BPLCON0, BPLCON0_COMPOSITE_COLOR),
		COP_MOVE(COLOR00, 0x000),
		COP_WAIT_1_3,
		COP_MOVE(COLOR00, 0xf00),
		COP_WAIT_2_3,
		COP_MOVE(COLOR00, 0xff0),
		COP_WAIT_END
};
/** END TEST **/

/*
 * functions
 */

extern UBYTE REGARGS mmu_mark(REG(a0, void *start), REG(d0, ULONG length), REG(d1, ULONG cm), REG(a6, struct ExecBase *SysBase));

int colorKey = 0;
static int isInitialized = 0;
static ULONG colorsAGA[770];

/** Hardware window */
struct Window *_hardwareWindow;
/** Hardware screen */
struct Screen *_hardwareScreen;
/** Hardware double buffering */
struct ScreenBuffer *_hardwareScreenBuffer[2];

BYTE _currentScreenBuffer;

static unsigned masks[4][4] = {{ 0x00000000, 0x00000000, 0x00000000, 0x00000000 },
							   { 0x0000001F, 0x000007E0, 0x0000F800, 0x00000000 },
							   { 0x000000FF, 0x0000FF00, 0x00FF0000, 0x00000000 },
							   { 0x000000FF, 0x0000FF00, 0x00FF0000, 0x00000000 }};

enum videoMode {
	VideoModeAGA,
	VideoModeRTG
};

enum videoMode vidMode = VideoModeAGA;

// RTG Stuff

struct Library *CyberGfxBase = NULL;
static APTR video_bitmap_handle = NULL;

static UWORD emptypointer[] = {
		0x0000, 0x0000,    /* reserved, must be NULL */
		0x0000, 0x0000,     /* 1 row of image data */
		0x0000, 0x0000    /* reserved, must be NULL */
};

/** Amiga Window/Screen Initialization **/
void InitAmigaVideo(int width, int height, int bpp) {

	ULONG modeId = INVALID_ID;

	if ( vidMode == VideoModeRTG ) {
		if ( !CyberGfxBase ) CyberGfxBase = (struct Library *)OpenLibrary((UBYTE *)"cybergraphics.library", 41L);

		modeId = BestCModeIDTags(
				CYBRBIDTG_NominalWidth, width,
				CYBRBIDTG_NominalHeight, height,
				CYBRBIDTG_Depth, bpp,
				TAG_DONE);
	} else if ( vidMode == VideoModeAGA ) {
		modeId = BestModeID(BIDTAG_NominalWidth, width,
							BIDTAG_NominalHeight, height,
							BIDTAG_DesiredWidth, width,
							BIDTAG_DesiredHeight, height,
							BIDTAG_Depth, bpp,
							BIDTAG_MonitorID, PAL_MONITOR_ID,
							TAG_END);
	} else {
		printf("ERROR: Invalid video mode: %d", vidMode);
		exit(0);
	}

	_hardwareScreen = OpenScreenTags(NULL,
									 SA_Depth, bpp,
									 SA_DisplayID, modeId,
									 SA_Width, width,
									 SA_Height, height,
									 SA_Type, CUSTOMSCREEN | CUSTOMBITMAP,
									 SA_Overscan, OSCAN_TEXT,
									 SA_Quiet, TRUE,
									 SA_ShowTitle, FALSE,
									 SA_Draggable, FALSE,
									 SA_Exclusive, TRUE,
									 SA_AutoScroll, FALSE,
									 TAG_END);

	// Create the hardware screen.
	if ( vidMode == VideoModeAGA ) {
		_hardwareScreenBuffer[0] = AllocScreenBuffer(_hardwareScreen, NULL, SB_SCREEN_BITMAP);
		_hardwareScreenBuffer[1] = AllocScreenBuffer(_hardwareScreen, NULL, 0);
		_currentScreenBuffer = 1;
	}

	_hardwareWindow = OpenWindowTags(NULL,
									 WA_Left, 0,
									 WA_Top, 0,
									 WA_Width, width,
									 WA_Height, height,
									 WA_Title, NULL,
									 SA_AutoScroll, FALSE,
									 WA_CustomScreen, (ULONG)_hardwareScreen,
									 WA_Backdrop, FALSE,
									 WA_Borderless, TRUE,
									 WA_DragBar, FALSE,
									 WA_Activate, TRUE,
									 WA_SimpleRefresh, TRUE,
									 WA_NoCareRefresh, TRUE,
									 WA_ReportMouse, TRUE,
									 WA_RMBTrap, TRUE,
									 WA_IDCMP, IDCMP_RAWKEY | IDCMP_MOUSEMOVE | IDCMP_DELTAMOVE | IDCMP_MOUSEBUTTONS,
									 TAG_END);

	if ( vidMode == VideoModeRTG ) {
		SetPointer (_hardwareWindow, emptypointer, 0, 0, 0, 0);
	} else {
		SetPointer (_hardwareWindow, emptypointer, 1, 16, 0, 0);

		//ChangeScreenBuffer(_hardwareScreen, _hardwareScreenBuffer[_currentScreenBuffer]);

		//	_hardwareScreen->RastPort.BitMap = &screenBitmap;
		//	InitBitMap(&screenBitmap, bpp, width, height);

		// init offscreen bitmap to hold our sprites
		//	InitRastPort(&spriteContext);
		//	spriteContext.BitMap = &spriteBitmap;

		//	InitBitMap(&spriteBitmap, bpp, 20, 40);
/*
		for(int i = 0; i < bpp; i++) {
			spriteBitmap.Planes[i] = AllocRaster(20,40);
			if (spriteBitmap.Planes[i] == 0) {
				fprintf(stderr, "failed to allocate sprite bitmap\n");
				return(1);
			}
		}*/
	}
}

/** Timing **/
void vga_wait(void) {
	static int prevtick = 0;
	int now = SDL_GetTicks();
	int wait = 1000 / 60 - (now - prevtick);
	if ( wait > 0 ) {
		usleep(wait);
	} else
		usleep(1);
	prevtick = now;
}

UBYTE yPos = 0xa0;
int test() {
	// line starting position


	// line direction
	BYTE yDir = 1;

	// Save interrupts and DMA
	UWORD oldInt = custom->intenar;

	// disable all interrupts
	custom->intena = 0x7fff;

	// loop until mouse clicked
	//while(ciaa->ciapra & CIAF_GAMEPORT0)
	{
		// wait for start of frame
		if ((custom->vposr & 1) == 0 && GETBYTE_HI(custom->vhposr) == 0x2c ) {

			custom->color[0] = 0;    // bg black

			// update line y position
			yPos += yDir;

			// bounce line
			if ( yPos > 0xf0 || yPos < 0x40 ) {
				yDir = -yDir;
			}

			// do literally nothing until we reach yPos
			while ( GETBYTE_HI(custom->vhposr) != yPos ) {}
			custom->color[0] = 0xfff;    // bg white

			// now do nothing until we're not on yPos
			while ( GETBYTE_HI(custom->vhposr) == yPos ) {}
			custom->color[0] = 0x116;    // bg blue

			if (( GETBYTE_HI(custom->vhposr) != 0x2a )) return 0;
		}
	}

	// restore interrupts and DMA
	custom->intena = oldInt | 0x8000;

	return 0;
}

void SDL_Delay(uint32_t ms) {
	//if ( vidMode == VideoModeAGA ) {
		//waitFrame();
	//	waitFrame2();
	//	ClearScreen(&_hardwareScreen->RastPort);
	//} else {
		usleep(ms);
	//}
}

void waitFrame() {
	do {
		while ( GETBYTE_LO(custom->vposr) & 1 );
	} while ( GETBYTE_HI(custom->vhposr) != 0x2c );
}

void waitFrame2() {
	while ( GETBYTE_HI(custom->vhposr) != 0x2c );
}


/** Colors **/
int SDL_SetColors(SDL_Surface *surface, SDL_Color *colors, int firstcolor, int ncolors) {
	return SDL_SetPalette(surface, 0, colors, firstcolor, ncolors);
}

int SDL_SetPalette(SDL_Surface *surface, int flags, SDL_Color *colors, int firstcolor, int ncolors) {
	int i = 0;

	if ( !firstcolor )
		firstcolor = i;

	int j = 1;

	//if(bpp>1) return 0;

	for ( i = firstcolor; i < ncolors; ++i ) {
		colorsAGA[j] = colors[i].r << 24;
		colorsAGA[j + 1] = colors[i].g << 24;
		colorsAGA[j + 2] = colors[i].b << 24;

		j += 3;
	}

	colorsAGA[0] = ((256) << 16);
	colorsAGA[((256 * 3) + 1)] = 0x00000000;
	LoadRGB32(&_hardwareScreen->ViewPort, &colorsAGA);

	return 1;
}

int ADL_SetPalette(const unsigned char *colors, int firstcolor, int ncolors) {
	int i = 0;

	if ( !firstcolor )
		firstcolor = i;

	int j = 1;

	//if(bpp>1) return 0;

	for ( i = firstcolor; i < ncolors; ++i ) {
		colorsAGA[j] = colors[i] << 24;
		colorsAGA[j + 1] = colors[i + 1] << 24;
		colorsAGA[j + 2] = colors[i + 2] << 24;

		j += 3;
	}

	colorsAGA[0] = ((ncolors) << 16);
	colorsAGA[((ncolors * 3) + 1)] = 0x00000000;
	LoadRGB32(&_hardwareScreen->ViewPort, &colorsAGA);

	return 1;
}

int SDL_SetColorKey(SDL_Surface *surface, uint32_t flag, uint32_t key) {
	colorKey = key;
}

/** Drawing **/
int ADL_FillRect(SDL_Rect *dstrect, uint32_t color) {
	SetAPen(&_hardwareScreen->RastPort, color);
	//RectFill(&_hardwareScreen->RastPort, dstrect->x, dstrect->y, dstrect->x + dstrect->w-1, dstrect->y + dstrect->h);
	//Draw
	WritePixelLine8(&_hardwareScreen->RastPort, dstrect->x, dstrect->y, dstrect->x + dstrect->w-1, NULL, NULL);
	//custom->cop1lc = (ULONG)((struct GfxBase*)GfxBase)->copinit;
}

int SDL_FillRect(SDL_Surface *dst, SDL_Rect *dstrect, uint32_t color) {
	int x, y;
	uint8_t *row;

	if ( dstrect == NULL) {
		dstrect = AllocVec(sizeof(SDL_Rect), MEMF_FAST);
		dstrect->w = dst->w;
		dstrect->h = dst->h;
		dstrect->x = 0;
		dstrect->y = 0;
	}

	row = (uint8_t *)dst->pixels + dstrect->y * dst->w + dstrect->x;

	x = dstrect->w;

	for ( y = dstrect->h; y; y-- ) {
		memset(row, color, x);

		row += dst->w;
	}

	return (0);
}

void Print(const char *text) {
	//ScrollRaster(&_hardwareScreen->RastPort,0,8,195,119,300,245);
	Move(&_hardwareScreen->RastPort, 195, 243);
	Text(&_hardwareScreen->RastPort, text, strlen(text));
}

void c2p(SDL_Surface *src, struct BitMap *output) {
	InitBitMap(output, 8, src->w, src->h);
	for ( int i = 0; i < 8; i++ ) {
		output->Planes[i] = AllocRaster(src->w, src->h);

		if ( output->Planes[i] == 0 ) {
			fprintf(stderr, "failed to allocate sprite bitmap\n");
			return (1);
		}
	}
	//c2p1x1_8_c5_bm_040(src->w, src->h, 0, 0, src->pixels, &output);
}

void c2p2(SDL_Rect *srcrect, void *src, struct BitMap *output) {
	InitBitMap(&output, 8, srcrect->w, srcrect->h);
	for ( int i = 0; i < 8; i++ ) {
		output->Planes[i] = AllocRaster(srcrect->w, srcrect->h);
		if ( output->Planes[i] == 0 ) {
			fprintf(stderr, "failed to allocate sprite bitmap\n");
			return (1);
		}
	}
	c2p1x1_8_c5_bm_040(srcrect->w, srcrect->h, 0, 0, src, &output);
}

void ADL_BlitBitPlane(struct BitMap *pixels, SDL_Rect *srcrect, SDL_Rect *dstrect) {
	BltBitMapRastPort(
			&pixels,   // Source
			srcrect->x,
			srcrect->y,      // Position, source.
			&_hardwareScreen->RastPort,   // Destination.
			dstrect->x,          // Position, destination.
			dstrect->y,
			srcrect->w,
			srcrect->h,          // Width and height.
			0xC0/*,            // Normal copy.
			0xFF,            // All bitplanes.
			NULL*/);          // No temporary storage.
}

void putpixel(unsigned x, unsigned y, int color, SDL_Surface *screen) {
	int pixind;
	if ( x > screen->w || y > screen->h )
		return;
	pixind = x + y * screen->w;

	((unsigned char *)screen->pixels)[pixind] = color;
}

int SDL_Flip(SDL_Surface *src) {

	UBYTE *base_address;

	if ( vidMode == VideoModeAGA ) {
		c2p1x1_8_c5_bm(src->w, src->h, 0, 0, src->pixels, _hardwareScreenBuffer[_currentScreenBuffer]->sb_BitMap);
		ChangeScreenBuffer(_hardwareScreen, _hardwareScreenBuffer[_currentScreenBuffer]);
		_currentScreenBuffer = _currentScreenBuffer ^ 1;
	} else {
		video_bitmap_handle = LockBitMapTags (_hardwareScreen->ViewPort.RasInfo->BitMap,
											  LBMI_BASEADDRESS, &base_address,
											  TAG_DONE);
		if ( video_bitmap_handle ) {
			CopyMem (src->pixels, base_address, src->w * src->h);
			UnLockBitMap (video_bitmap_handle);
			video_bitmap_handle = NULL;
		}
	}

	return 1;
}

int SDL_BlitSurface(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect) {
	int x, y;
	uint8_t *row, *srcrow;

	if ( srcrect->x > src->w || srcrect->y > src->h )
		return (0);

	if ( srcrect->x < 0 || srcrect->y < 0 )
		return (0);

	if ( dstrect->x > dst->w || dstrect->y > dst->h )
		return (0);

	if ( dstrect->x < 0 || dstrect->y < 0 )
		return (0);

	if ( dstrect->x + dstrect->w > dst->w ) {
		return (0);//dstrect->w -= dst->w - dstrect->x + dstrect->w;
	}

	if ( dstrect->y + dstrect->h > dst->h ) {
		return (0);//dstrect->h -= dst->h - dstrect->y + dstrect->h;
	}

	srcrow = (uint8_t *)src->pixels + srcrect->y * src->w + srcrect->x - 1;
	row = (uint8_t *)dst->pixels + dstrect->y * dst->w + dstrect->x - 1;

	for ( y = srcrect->h; y; y-- ) {
		for ( x = srcrect->w; x; x-- ) {
			if ( srcrow[x] == colorKey )
				continue;
			row[x] = srcrow[x];
		}
		row += dst->w;
		srcrow += src->w;
	}

	return (0);
}

/** Surfaces **/
SDL_Surface *SDL_CreateRGBSurface(uint32_t flags, int width, int height, int depth, uint32_t Rmask, uint32_t Gmask, uint32_t Bmask, uint32_t Amask) {
	return SDL_CreateRGBSurfaceFrom((void *)hexcanvas, width, height, depth, width, Rmask, Gmask, Bmask, Amask);
}

SDL_Surface *SDL_CreateRGBSurfaceFrom(void *pixels, int width, int height, int depth, int pitch, uint32_t Rmask, uint32_t Gmask, uint32_t Bmask, uint32_t Amask) {
	SDL_Surface *surface;
	surface = (SDL_Surface *)AllocVec(sizeof(SDL_Surface), MEMF_FAST);
	surface->w = width;
	surface->h = height;
	surface->pixels = pixels;

	return surface;
}

SDL_Surface *SDL_DisplayFormat(SDL_Surface *surface) {
	return surface;
}

char *SDL_GetError(void) {
	//Todo: This is a stub, needs work
	return "";
}

int SDL_ShowCursor(int toggle) {
	//Todo: This is a stub, needs work
	return (0);
}

/** Initialization **/
int SDL_Init(uint32_t flags) {
	SetTaskPri(FindTask(NULL), 21);
	isInitialized = 1;
	InitTimer();
	InitControls();
	return (0);
}

SDL_Surface *SDL_SetVideoMode(int width, int height, int bpp, uint32_t flags) {
	static int firsttime = 1;

	_hardwareWindow = NULL;
	_hardwareScreenBuffer[0] = NULL;
	_hardwareScreenBuffer[1] = NULL;
	_currentScreenBuffer = 0;
	_hardwareScreen = NULL;


	if ( height >= 480 ) vidMode = VideoModeRTG;

	if ( firsttime ) {
		firsttime = 0;

		InitAmigaVideo(width, height, bpp);
	}

	return SDL_CreateRGBSurface(0, width, height, bpp, 0, 0, 0, 0);
}

/** Destruction **/
void SDL_FreeSurface(SDL_Surface *surface) {
	if ( surface )
		FreeVec(surface);
}

void SDL_Quit(void) {
	if ( isInitialized ) {
		DestroyTimer();
		DestroyControls();

		if ( _hardwareWindow ) {
			ClearPointer(_hardwareWindow);
			CloseWindow(_hardwareWindow);
			_hardwareWindow = NULL;
		}

		if ( _hardwareScreenBuffer[0] ) {
			WaitBlit();
			FreeScreenBuffer(_hardwareScreen, _hardwareScreenBuffer[0]);
			_hardwareScreenBuffer[0] = NULL;
		}

		if ( _hardwareScreenBuffer[1] ) {
			WaitBlit();
			FreeScreenBuffer(_hardwareScreen, _hardwareScreenBuffer[1]);
			_hardwareScreenBuffer[1] = NULL;
		}

		if ( _hardwareScreen ) {
			CloseScreen(_hardwareScreen);
			_hardwareScreen = NULL;
		}

		if ( CyberGfxBase ) {
			CloseLibrary(CyberGfxBase);
			CyberGfxBase = NULL;
		}
	}
}