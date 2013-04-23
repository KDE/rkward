/***************************************************************************
                          rkreventloop  -  description
                             -------------------
    begin                : Tue Apr 23 2013
    copyright            : (C) 2013 by Thomas Friedrichsmeier
    email                : tfry@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "rkreventloop.h"

#include <R_ext/eventloop.h>
#include <Rinternals.h>

#include "rkrbackend.h"

#include "../debug.h"

extern "C" void RK_doIntr ();

static void processX11EventsWorker (void *) {
// this basically copied from R's unix/sys-std.c (Rstd_ReadConsole)
#ifndef Q_WS_WIN
	for (;;) {
		fd_set *what;
		what = R_checkActivityEx(R_wait_usec > 0 ? R_wait_usec : 50, 1, RK_doIntr);
		R_runHandlers(R_InputHandlers, what);
		if (what == NULL) break;
	}
	/* This seems to be needed to make Rcmdr react to events. Has this always been the case? It was commented out for a long time, without anybody noticing. */
	R_PolledEvents ();
#else
#warning TODO: correct?
	R_ProcessEvents();
#endif

#if 0
// TODO: The remainder of this function had been commented out since R 2.3.x and is not in Rstd_ReadConsole. Do we still need this?
	/* I don't really understand what I'm doing here, but apparently this is necessary for Tcl-Tk windows to function properly. */
	R_PolledEvents ();
	
/* Maybe we also need to also call R_timeout_handler once in a while? Obviously this is extremely crude code! 
TODO: verify we really need this. */
	if (++timeout_counter > 100) {
//		extern void (* R_timeout_handler) ();	// already defined in Rinferface.h
		if (R_timeout_handler) R_timeout_handler ();
		timeout_counter = 0;
	}
#endif
}

void RKREventLoop::processX11Events() {
	// do not trace
	if (!RKRBackend::this_pointer->r_running) return;
	if (RKRBackend::this_pointer->isKilled ()) return;

	RKRBackend::RKRBackend::repl_status.eval_depth++;
// In case an error (or user interrupt) is caught inside processX11EventsWorker, we don't want to long-jump out.
	R_ToplevelExec (processX11EventsWorker, 0);
	RKRBackend::RKRBackend::repl_status.eval_depth--;
}

static void (* RK_old_R_PolledEvents)();
static void (* RK_eventHandlerFunction)() = 0;

#ifndef Q_OS_WIN
// NOTE: input-handler-based event loop mechanism is heavily inspired by (but not quite the same as in) package qtbase version 1.0.4 by Michael Lawrence, Deepayan Sarkar.
// URL: http://qtinterfaces.r-forge.r-project.org
static int ifd = 0;
static int ofd = 0;
static char buf[16];
static bool rk_event_handler_triggered = false;
#	include <unistd.h>
static void RK_eventHandlerWrapper (void *data) {
	Q_UNUSED (data);
	rk_event_handler_triggered = false;
	char buf[16];
	bool read_ok = read (ifd, buf, 16);
	RK_ASSERT (read_ok);
	RK_eventHandlerFunction ();
}
#endif

static void RK_eventHandlerChain () {
	if (RK_eventHandlerFunction) RK_eventHandlerFunction ();
	if (RK_old_R_PolledEvents) RK_old_R_PolledEvents ();
}

void RKREventLoop::setRKEventHandler (void (* handler) ()) {
	RK_TRACE (RBACKEND);
	RK_ASSERT (!RK_eventHandlerFunction);
	RK_eventHandlerFunction = handler;

	bool ok = false;
#ifndef Q_OS_WIN
	int fds[2];

	if (!pipe (fds)) {
		ifd = fds[0];
		ofd = fds[1];
		addInputHandler (R_InputHandlers, ifd, RK_eventHandlerWrapper, 32);
		ok = true;
	}
#endif
	if (ok) return;

	// if pipe method did not work, fall back to R_PolledEvents
	RK_old_R_PolledEvents = R_PolledEvents;
	R_PolledEvents = RK_eventHandlerChain;
}

void RKREventLoop::wakeRKEventHandler () {
#ifndef Q_OS_WIN
	if (!ofd) return;
	if (rk_event_handler_triggered) return;
	rk_event_handler_triggered = true;
	*buf = 0;
	bool write_ok = write (ofd, buf, 1);
	RK_ASSERT (write_ok);
#endif
}
