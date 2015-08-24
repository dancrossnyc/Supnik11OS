/* sim_timer.c: simulator timer library

   Copyright (c) 1993-2010, Robert M Supnik

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
   ROBERT M SUPNIK BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
   IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

   Except as contained in this notice, the name of Robert M Supnik shall not be
   used in advertising or otherwise to promote the sale, use or other dealings
   in this Software without prior written authorization from Robert M Supnik.

   29-Dec-10    MP      Fixed clock resolution determination for Unix platforms
   22-Sep-08    RMS     Added "stability threshold" for idle routine
   27-May-08    RMS     Fixed bug in Linux idle routines (from Walter Mueller)
   18-Jun-07    RMS     Modified idle to exclude counted delays
   22-Mar-07    RMS     Added sim_rtcn_init_all
   17-Oct-06    RMS     Added idle support (based on work by Mark Pizzolato)
                        Added throttle support
   16-Aug-05    RMS     Fixed C++ declaration and cast problems
   02-Jan-04    RMS     Split out from SCP

   This library includes the following routines:

   sim_timer_init -     initialize timing system
   sim_rtc_init -       initialize calibration
   sim_rtc_calb -       calibrate clock
   sim_timer_init -     initialize timing system
   sim_idle -           virtual machine idle
   sim_os_msec  -       return elapsed time in msec
   sim_os_sleep -       sleep specified number of seconds
   sim_os_ms_sleep -    sleep specified number of milliseconds

   The calibration, idle, and throttle routines are OS-independent; the _os_
   routines are not.
*/

#include "sim_defs.h"

t_bool sim_idle_enab = FALSE;	/* global flag */

static uint32 sim_idle_rate_ms = 0;
static uint32 sim_idle_stable = SIM_IDLE_STDFLT;
extern int32 sim_interval, sim_switches;
extern FILE *sim_log;
extern UNIT *sim_clock_queue;

/* OS-dependent timer and clock routines */

/* UNIX routines */

#include <time.h>
#include <unistd.h>

#define NANOS_PER_MILLI     1000000
#define MILLIS_PER_SEC      1000
#define sleep1Samples       100

const t_bool rtc_avail = TRUE;

#if 0
// XXX pic32
#include <sys/time.h>
#endif

uint32
sim_os_msec()
{
	uint32 msec = 0;
#if 0
	// XXX pic32.
	struct timeval cur;
	struct timezone foo;

	gettimeofday (&cur, &foo);
	msec = (((uint32) cur.tv_sec) * 1000) + (((uint32) cur.tv_usec) / 1000);
#endif
	return msec;
}

void
sim_os_sleep(unsigned int sec)
{
#if 0
	// XXX pic32.
	sleep(sec);
#endif
}

uint32
sim_os_ms_sleep_init(void)
{
	uint32 i, t1, t2, tot, tim;

	for (i = 0, tot = 0; i < sleep1Samples; i++) {
		t1 = sim_os_msec();
		sim_os_ms_sleep(1);
		t2 = sim_os_msec();
		tot += (t2 - t1);
	}
	tim = (tot + (sleep1Samples - 1)) / sleep1Samples;
	if (tim > SIM_IDLE_MAX)
		tim = 0;
	return tim;
}

uint32
sim_os_ms_sleep(unsigned int milliseconds)
{
	uint32 stime = sim_os_msec();
#if 0
	// XXX pic32.
	struct timespec treq;

	treq.tv_sec = milliseconds / MILLIS_PER_SEC;
	treq.tv_nsec = (milliseconds % MILLIS_PER_SEC) * NANOS_PER_MILLI;
	(void)nanosleep(&treq, NULL);
#endif

	return sim_os_msec() - stime;
}

/* OS independent clock calibration package */

static int32 rtc_ticks[SIM_NTIMERS] = { 
	0 };	/* ticks */
static int32 rtc_hz[SIM_NTIMERS] = { 
	0 };	/* tick rate */
static uint32 rtc_rtime[SIM_NTIMERS] = { 
	0 };	/* real time */
static uint32 rtc_vtime[SIM_NTIMERS] = { 
	0 };	/* virtual time */
static uint32 rtc_nxintv[SIM_NTIMERS] = { 
	0 };	/* next interval */
static int32 rtc_based[SIM_NTIMERS] = { 
	0 };	/* base delay */
static int32 rtc_currd[SIM_NTIMERS] = { 
	0 };	/* current delay */
static int32 rtc_initd[SIM_NTIMERS] = { 
	0 };	/* initial delay */
static uint32 rtc_elapsed[SIM_NTIMERS] = { 
	0 };	/* sec since init */

void
sim_rtcn_init_all(void)
{
	uint32 i;

	for (i = 0; i < SIM_NTIMERS; i++) {
		if (rtc_initd[i] != 0)
			sim_rtcn_init(rtc_initd[i], i);
	}
	return;
}

int32
sim_rtcn_init(int32 time, int32 tmr)
{
	if (time == 0)
		time = 1;
	if ((tmr < 0) || (tmr >= SIM_NTIMERS))
		return time;
	rtc_rtime[tmr] = sim_os_msec();
	rtc_vtime[tmr] = rtc_rtime[tmr];
	rtc_nxintv[tmr] = 1000;
	rtc_ticks[tmr] = 0;
	rtc_hz[tmr] = 0;
	rtc_based[tmr] = time;
	rtc_currd[tmr] = time;
	rtc_initd[tmr] = time;
	rtc_elapsed[tmr] = 0;
	return time;
}

int32
sim_rtcn_calb(int32 ticksper, int32 tmr)
{
	uint32 new_rtime, delta_rtime;
	int32 delta_vtime;

	if ((tmr < 0) || (tmr >= SIM_NTIMERS))
		return 10000;
	rtc_hz[tmr] = ticksper;
	rtc_ticks[tmr] = rtc_ticks[tmr] + 1;	/* count ticks */
	if (rtc_ticks[tmr] < ticksper)	/* 1 sec yet? */
		return rtc_currd[tmr];
	rtc_ticks[tmr] = 0;	/* reset ticks */
	rtc_elapsed[tmr] = rtc_elapsed[tmr] + 1;	/* count sec */
	if (!rtc_avail)		/* no timer? */
		return rtc_currd[tmr];
	new_rtime = sim_os_msec();	/* wall time */
	if (new_rtime < rtc_rtime[tmr]) {	/* time running backwards? */
		rtc_rtime[tmr] = new_rtime;	/* reset wall time */
		return rtc_currd[tmr];	/* can't calibrate */
	}
	delta_rtime = new_rtime - rtc_rtime[tmr];	/* elapsed wtime */
	rtc_rtime[tmr] = new_rtime;	/* adv wall time */
	rtc_vtime[tmr] = rtc_vtime[tmr] + 1000;	/* adv sim time */
	if (delta_rtime > 30000)	/* gap too big? */
		return rtc_initd[tmr];	/* can't calibr */
	if (delta_rtime == 0)	/* gap too small? */
		rtc_based[tmr] = rtc_based[tmr] * ticksper;	/* slew wide */
	else
		rtc_based[tmr] = (int32) (((double)rtc_based[tmr] * (double)rtc_nxintv[tmr]) / ((double)delta_rtime));	/* new base rate */
	delta_vtime = rtc_vtime[tmr] - rtc_rtime[tmr];	/* gap */
	if (delta_vtime > SIM_TMAX)	/* limit gap */
		delta_vtime = SIM_TMAX;
	else if (delta_vtime < -SIM_TMAX)
		delta_vtime = -SIM_TMAX;
	rtc_nxintv[tmr] = 1000 + delta_vtime;	/* next wtime */
	rtc_currd[tmr] = (int32) (((double)rtc_based[tmr] * (double)rtc_nxintv[tmr]) / 1000.0);	/* next delay */
	if (rtc_based[tmr] <= 0)	/* never negative or zero! */
		rtc_based[tmr] = 1;
	if (rtc_currd[tmr] <= 0)	/* never negative or zero! */
		rtc_currd[tmr] = 1;
	return rtc_currd[tmr];
}

/* Prior interfaces - default to timer 0 */

int32
sim_rtc_init(int32 time)
{
	return sim_rtcn_init(time, 0);
}

int32
sim_rtc_calb(int32 ticksper)
{
	return sim_rtcn_calb(ticksper, 0);
}

/* sim_timer_init - get minimum sleep time available on this host */

t_bool
sim_timer_init(void)
{
	sim_idle_enab = FALSE;	/* init idle off */
	sim_idle_rate_ms = sim_os_ms_sleep_init();	/* get OS timer rate */
	return (sim_idle_rate_ms != 0);
}

/* sim_idle - idle simulator until next event or for specified interval

   Inputs:
        tmr =   calibrated timer to use

   Must solve the linear equation

        ms_to_wait = w * ms_per_wait

   Or
        w = ms_to_wait / ms_per_wait
*/

t_bool
sim_idle(uint32 tmr, t_bool sin_cyc)
{
	static uint32 cyc_ms = 0;
	uint32 w_ms, w_idle, act_ms;
	int32 act_cyc;

	if ((sim_clock_queue == NULL) ||	/* clock queue empty? */
	((sim_clock_queue->flags & UNIT_IDLE) == 0) ||	/* event not idle-able? */
	(rtc_elapsed[tmr] < sim_idle_stable)) {	/* timer not stable? */
		if (sin_cyc)
			sim_interval = sim_interval - 1;
		return FALSE;
	}
	if (cyc_ms == 0)	/* not computed yet? */
		cyc_ms = (rtc_currd[tmr] * rtc_hz[tmr]) / 1000;	/* cycles per msec */
	if ((sim_idle_rate_ms == 0) || (cyc_ms == 0)) {	/* not possible? */
		if (sin_cyc)
			sim_interval = sim_interval - 1;
		return FALSE;
	}
	w_ms = (uint32) sim_interval / cyc_ms;	/* ms to wait */
	w_idle = w_ms / sim_idle_rate_ms;	/* intervals to wait */
	if (w_idle == 0) {	/* none? */
		if (sin_cyc)
			sim_interval = sim_interval - 1;
		return FALSE;
	}
	act_ms = sim_os_ms_sleep(w_ms);	/* wait */
	act_cyc = act_ms * cyc_ms;
	if (sim_interval > act_cyc)
		sim_interval = sim_interval - act_cyc;	/* count down sim_interval */
	else
		sim_interval = 0;	/* or fire immediately */
	return TRUE;
}

/* Set idling - implicitly disables throttling */

t_stat
sim_set_idle(UNIT * uptr, int32 val, char *cptr, void *desc)
{
	t_stat r;
	uint32 v;

	if (sim_idle_rate_ms == 0)
		return SCPE_NOFNC;
	if ((val != 0) && (sim_idle_rate_ms > (uint32) val))
		return SCPE_NOFNC;
	if (cptr) {
		v = (uint32) get_uint(cptr, 10, SIM_IDLE_STMAX, &r);
		if ((r != SCPE_OK) || (v < SIM_IDLE_STMIN))
			return SCPE_ARG;
		sim_idle_stable = v;
	}
	sim_idle_enab = TRUE;
	return SCPE_OK;
}

/* Clear idling */

t_stat
sim_clr_idle(UNIT * uptr, int32 val, char *cptr, void *desc)
{
	sim_idle_enab = FALSE;
	return SCPE_OK;
}

/* Show idling */

t_stat
sim_show_idle(FILE * st, UNIT * uptr, int32 val, void *desc)
{
	if (sim_idle_enab)
		fprintf(st, "idle enabled, stability wait = %ds", sim_idle_stable);
	else
		fprintf(st, "idle disabled");
	return SCPE_OK;
}
