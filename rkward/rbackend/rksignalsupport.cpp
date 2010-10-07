/***************************************************************************
                          rksignalsupport  -  description
                             -------------------
    begin                : Thu Nov 22 2007
    copyright            : (C) 2007, 2009, 2010 by Thomas Friedrichsmeier
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

#include "rksignalsupport.h"

#include <signal.h>

#include "rinterface.h"

#include "../debug.h"

#ifdef Q_WS_WIN
#	ifndef __sighandler_t
		typedef void (*__sighandler_t) (int);
#	endif
#endif

namespace RKSignalSupportPrivate {
#ifdef Q_WS_WIN
	__sighandler_t r_sigsegv_handler = 0;
	__sighandler_t default_sigsegv_handler = 0;
	__sighandler_t r_sigill_handler = 0;
	__sighandler_t default_sigill_handler = 0;
	__sighandler_t r_sigabrt_handler = 0;
	__sighandler_t default_sigabrt_handler = 0;
#else
	struct sigaction r_sigsegv_handler;
	struct sigaction default_sigsegv_handler;
	struct sigaction r_sigill_handler;
	struct sigaction default_sigill_handler;
	struct sigaction r_sigabrt_handler;
	struct sigaction default_sigabrt_handler;
#endif

#ifdef Q_WS_WIN
	void signal_proxy (int signum) {
		__sighandler_t r_handler = r_sigsegv_handler;
		__sighandler_t default_handler = default_sigsegv_handler;
#else
	void signal_proxy (int signum, siginfo_t *info, void *context) {
		struct sigaction r_handler = r_sigsegv_handler;
		struct sigaction default_handler = default_sigsegv_handler;
#endif
		if (signum == SIGILL) {
			r_handler = r_sigill_handler;
			default_handler = default_sigill_handler;
		} else if (signum == SIGABRT) {
			r_handler = r_sigabrt_handler;
			default_handler = default_sigabrt_handler;
		} else {
			RK_ASSERT (signum == SIGSEGV);
		}

		RInterface::tryToDoEmergencySave ();

		// if we are not in the R thread, handling the signal in R does more harm than good.
		if (RInterface::inRThread ()) {
#ifdef Q_WS_WIN
			if (r_handler) {
				r_handler (signum);
				return;
			}
#else
			if (r_handler.sa_sigaction) {
				r_handler.sa_sigaction (signum, info, context);
				return;
			} else if (r_handler.sa_handler) {
				r_handler.sa_handler (signum);
				return;
			}
#endif
		}

#ifdef Q_WS_WIN
		if (default_handler) {
			default_handler (signum);
			return;
		}
#else
		// this might be a Qt/KDE override or default handling
		if (default_handler.sa_sigaction) {
			default_handler.sa_sigaction (signum, info, context);
			return;
		} else if (default_handler.sa_handler) {
			default_handler.sa_handler (signum);
			return;
		}
#endif
	}
}

void RKSignalSupport::saveDefaultSignalHandlers () {
	RK_TRACE (RBACKEND);

#ifdef Q_WS_WIN
	RKSignalSupportPrivate::default_sigsegv_handler = signal (SIGSEGV, SIG_DFL);
	RKSignalSupportPrivate::default_sigill_handler = signal (SIGILL, SIG_DFL);
	RKSignalSupportPrivate::default_sigabrt_handler = signal (SIGABRT, SIG_DFL);
#else
	sigaction (SIGSEGV, 0, &RKSignalSupportPrivate::default_sigsegv_handler);
	sigaction (SIGILL, 0, &RKSignalSupportPrivate::default_sigill_handler);
	sigaction (SIGABRT, 0, &RKSignalSupportPrivate::default_sigabrt_handler);
#endif
}

void RKSignalSupport::installSignalProxies () {
	RK_TRACE (RBACKEND);

#ifdef Q_WS_WIN
	RKSignalSupportPrivate::r_sigsegv_handler = signal (SIGSEGV, &RKSignalSupportPrivate::signal_proxy);
	RKSignalSupportPrivate::r_sigill_handler = signal (SIGILL, &RKSignalSupportPrivate::signal_proxy);
	RKSignalSupportPrivate::r_sigabrt_handler = signal (SIGABRT, &RKSignalSupportPrivate::signal_proxy);
#else
	// retrieve R signal handler
	sigaction (SIGSEGV, 0, &RKSignalSupportPrivate::r_sigsegv_handler);
	sigaction (SIGILL, 0, &RKSignalSupportPrivate::r_sigill_handler);
	sigaction (SIGABRT, 0, &RKSignalSupportPrivate::r_sigabrt_handler);

	// set new proxy handlers
	struct sigaction proxy_action;
	proxy_action = RKSignalSupportPrivate::r_sigsegv_handler;
	proxy_action.sa_flags |= SA_SIGINFO;
	proxy_action.sa_sigaction = &RKSignalSupportPrivate::signal_proxy;
	sigaction (SIGSEGV, &proxy_action, 0);

	proxy_action = RKSignalSupportPrivate::r_sigill_handler;
	proxy_action.sa_flags |= SA_SIGINFO;
	proxy_action.sa_sigaction = &RKSignalSupportPrivate::signal_proxy;
	sigaction (SIGILL, &proxy_action, 0);

	proxy_action = RKSignalSupportPrivate::default_sigabrt_handler;
	proxy_action.sa_flags |= SA_SIGINFO;
	proxy_action.sa_sigaction = &RKSignalSupportPrivate::signal_proxy;
	sigaction (SIGABRT, &proxy_action, 0);
#endif
}
