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
	__sighandler_t r_sigsegv_handler = 0;
	__sighandler_t default_sigsegv_handler = 0;

	void sigsegv_proxy (int signum) {
		RK_ASSERT (signum == SIGSEGV);
	
		// if we are not in the R thread, handling the signal in R does more harm than good.
		if (RInterface::inRThread ()) {
			if (r_sigsegv_handler) {
				r_sigsegv_handler (signum);
				return;
			}
		}
	
		// this might be a Qt/KDE override
		if (default_sigsegv_handler) {
			default_sigsegv_handler (signum);
			return;
		}
	
		// do the default handling
		signal (SIGSEGV, SIG_DFL);
		raise (SIGSEGV);
	}
}

void RKSignalSupport::saveDefaultSigSegvHandler () {
	RK_TRACE (RBACKEND);

	RKSignalSupportPrivate::default_sigsegv_handler = signal (SIGSEGV, SIG_DFL);
}

void RKSignalSupport::installSigSegvProxy () {
	RK_TRACE (RBACKEND);

	RKSignalSupportPrivate::r_sigsegv_handler = signal (SIGSEGV, &RKSignalSupportPrivate::sigsegv_proxy);
}
