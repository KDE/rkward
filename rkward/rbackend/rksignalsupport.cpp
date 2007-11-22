/***************************************************************************
                          rksignalsupport  -  description
                             -------------------
    begin                : Thu Nov 22 2007
    copyright            : (C) 2007 by Thomas Friedrichsmeier
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

#ifndef __sighandler_t
typedef void (*__sighandler_t) (int);
#endif

namespace RKSignalSupportPrivate {
	struct sigaction r_sigsegv_action;
	struct sigaction default_sigsegv_action;

/*	__sighandler_t r_sigsegv_handler = 0;
	__sighandler_t default_sigsegv_handler = 0; */

	void sigsegv_proxy (int signum, siginfo_t *info, void *context) {
		RK_ASSERT (signum == SIGSEGV);
	
		// if we are not in the R thread, handling the signal in R does more harm than good.
		if (RInterface::inRThread ()) {
			if (r_sigsegv_action.sa_sigaction) {
				r_sigsegv_action.sa_sigaction (signum, info, context);
				return;
			} else if (r_sigsegv_action.sa_handler) {
				r_sigsegv_action.sa_handler (signum);
				return;
			}
		}
	
		// this might be a Qt/KDE override or default handling
		if (default_sigsegv_action.sa_sigaction) {
			default_sigsegv_action.sa_sigaction (signum, info, context);
			return;
		} else if (default_sigsegv_action.sa_handler) {
			default_sigsegv_action.sa_handler (signum);
			return;
		}

		// not handled? should not happen
		RK_ASSERT (false);

		// do the default fallback handling
		signal (SIGSEGV, SIG_DFL);
		raise (SIGSEGV);
	}
}

void RKSignalSupport::saveDefaultSigSegvHandler () {
	RK_TRACE (RBACKEND);

	sigaction (SIGSEGV, 0, &RKSignalSupportPrivate::default_sigsegv_action);
//	RKSignalSupportPrivate::default_sigsegv_handler = signal (SIGSEGV, SIG_DFL);
}

void RKSignalSupport::installSigSegvProxy () {
	RK_TRACE (RBACKEND);

	// retrieve R signal handler
	sigaction (SIGSEGV, 0, &RKSignalSupportPrivate::r_sigsegv_action);

	struct sigaction proxy_action;
	proxy_action = RKSignalSupportPrivate::r_sigsegv_action;
	proxy_action.sa_flags |= SA_SIGINFO;
	proxy_action.sa_sigaction = &RKSignalSupportPrivate::sigsegv_proxy;

	// set new proxy handler
	sigaction (SIGSEGV, &proxy_action, 0);

//	RKSignalSupportPrivate::r_sigsegv_handler = signal (SIGSEGV, &RKSignalSupportPrivate::sigsegv_proxy);
}
