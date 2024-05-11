/*
rkreventloop - This file is part of RKWard (https://rkward.kde.org). Created: Tue Apr 23 2013
SPDX-FileCopyrightText: 2013-2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkreventloop.h"
#include "rkrbackend.h"

#include "rkrapi.h"

#include "../debug.h"

void RK_doIntr ();

static void processX11EventsWorker (void *) {
// this basically copied from R's unix/sys-std.c (Rstd_ReadConsole)
#ifndef Q_OS_WIN
	for (;;) {
		fd_set *what;
		what = RFn::R_checkActivityEx(ROb(R_wait_usec) > 0 ? ROb(R_wait_usec) : 50, 1, RK_doIntr);
		RFn::R_runHandlers(ROb(R_InputHandlers), what);
		if (!what) break;
	}
	/* This seems to be needed to make Rcmdr react to events. Has this always been the case? It was commented out for a long time, without anybody noticing. */
	(*ROb(R_PolledEvents))();
#else
	// TODO: correct?
	// NOTE: We essentially process events while waiting. Perhaps we should simply use the equivalent of "try(sleep(0.01))", instead.
	RFn::R_ProcessEvents();
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

	RKRBackend::repl_status.eval_depth++;
// In case an error (or user interrupt) is caught inside processX11EventsWorker, we don't want to long-jump out.
	RFn::R_ToplevelExec(processX11EventsWorker, nullptr);
	RKRBackend::repl_status.eval_depth--;
}

static void (* RK_eventHandlerFunction)() = nullptr;

#ifndef Q_OS_WIN
static void (* RK_old_R_PolledEvents)();
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

static void RK_eventHandlerChain () {
	if (RK_eventHandlerFunction) RK_eventHandlerFunction ();
	if (RK_old_R_PolledEvents) RK_old_R_PolledEvents ();
}
#else
void RKREventLoop::winRKEventHandlerWrapper (void) {
        if (RK_eventHandlerFunction) RK_eventHandlerFunction ();
}
#endif

void RKREventLoop::setRKEventHandler (void (* handler) ()) {
	RK_TRACE (RBACKEND);
	RK_ASSERT (!RK_eventHandlerFunction);
	RK_eventHandlerFunction = handler;

#ifndef Q_OS_WIN
        bool ok = false;
	int fds[2];

	if (!pipe (fds)) {
		ifd = fds[0];
		ofd = fds[1];
		RFn::addInputHandler(ROb(R_InputHandlers), ifd, RK_eventHandlerWrapper, 32);
		ok = true;
	}
	if (ok) return;

	// if pipe method did not work, fall back to R_PolledEvents
	RK_old_R_PolledEvents = ROb(R_PolledEvents);
	ROb(R_PolledEvents) = RK_eventHandlerChain;
#endif
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
