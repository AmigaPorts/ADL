#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <proto/timer.h>
#include <devices/timer.h>
#include <proto/exec.h>

static ULONG basetime = 0;
struct MsgPort *timer_msgport;
struct timerequest *timer_ioreq;
struct Device *TimerBase;

static int OpenTimer(ULONG unit) {
	timer_msgport = CreateMsgPort();
	timer_ioreq = CreateIORequest(timer_msgport, sizeof(*timer_ioreq));
	if ( timer_ioreq ) {
		if ( OpenDevice(TIMERNAME, unit, (APTR)timer_ioreq, 0) == 0 ) {
			TimerBase = (APTR)timer_ioreq->tr_node.io_Device;
			return 1;
		}
	}
	return 0;
}

static void CloseTimer(void) {
	if ( TimerBase ) {
		CloseDevice((APTR)timer_ioreq);

		DeleteIORequest(timer_ioreq);
		DeleteMsgPort(timer_msgport);
		TimerBase = NULL;
		timer_ioreq = 0;
		timer_msgport = 0;
	}
}

static struct timeval startTime;

void StartupTimer() {
	GetSysTime(&startTime);
}

ULONG GetMilliseconds() {
	struct timeval endTime;

	GetSysTime(&endTime);
	SubTime(&endTime, &startTime);

	return (endTime.tv_secs * 1000 + endTime.tv_micro / 1000);
}

int GetTimeMS(void) {
	ULONG ticks;

	ticks = GetMilliseconds();

	if ( basetime == 0 )
		basetime = ticks;

	return ticks - basetime;
}

void DestroyTimer() {
	CloseTimer();
}

void sleep(int ms) {
	usleep(ms);
}

void waitVBL(int count) {
	sleep((count * 1000) / 70);
}

void InitTimer() {
	OpenTimer(UNIT_VBLANK);
	StartupTimer();
}

#define GETTIME_FREQ (1000)

static unsigned lastinterval = 0;


uint32_t SDL_GetTicks() {
	return GetMilliseconds();
}


unsigned GetInterval(unsigned freq) {
	unsigned tickspassed, ebx, blocksize, now;
	now = SDL_GetTicks();
	ebx = now - lastinterval;
	blocksize = GETTIME_FREQ / freq;
	ebx += GETTIME_FREQ % freq;
	tickspassed = ebx / blocksize;
	ebx -= ebx % blocksize;
	lastinterval += ebx;
	return tickspassed;
}