/*
rksignalsupport - This file is part of RKWard (https://rkward.kde.org). Created: Thu Nov 22 2007
SPDX-FileCopyrightText: 2007-2010 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rksignalsupport.h"

#include <signal.h>

#include "rkrbackend.h"

#include "../debug.h"

// On some platforms sighandler_t is defined, on others it is not, but it's required to be the same everywhere, anyway.
// To avoid re-definition errors, we just use our own "type".
typedef void (*rk_sighandler_t) (int);

namespace RKSignalSupportPrivate {
#ifdef Q_OS_WIN
	rk_sighandler_t r_sigsegv_handler = 0;
	rk_sighandler_t default_sigsegv_handler = 0;
	rk_sighandler_t r_sigill_handler = 0;
	rk_sighandler_t default_sigill_handler = 0;
	rk_sighandler_t r_sigabrt_handler = 0;
	rk_sighandler_t default_sigabrt_handler = 0;
#else
	struct sigaction r_sigsegv_handler;
	struct sigaction default_sigsegv_handler;
	struct sigaction r_sigill_handler;
	struct sigaction default_sigill_handler;
	struct sigaction r_sigabrt_handler;
	struct sigaction default_sigabrt_handler;
#endif
	rk_sighandler_t r_sigint_handler = nullptr;
	void (*new_sigint_handler) (void) = nullptr;
	void internal_sigint_handler (int num) {
		new_sigint_handler ();
		signal (num, internal_sigint_handler);
	}

#ifdef Q_OS_WIN
	void signal_proxy (int signum) {
		rk_sighandler_t r_handler = r_sigsegv_handler;
		rk_sighandler_t default_handler = default_sigsegv_handler;
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

		RKRBackend::tryToDoEmergencySave ();

		// if we are not in the R thread, handling the signal in R does more harm than good.
		if (RKRBackendProtocolBackend::inRThread ()) {
#ifdef Q_OS_WIN
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

#ifdef Q_OS_WIN
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

		RK_DEBUG(RBACKEND, DL_ERROR, "Got unhandled signal %d", signum);

		signal (signum, SIG_DFL);
		raise (signum);
	}
}

void RKSignalSupport::saveDefaultSignalHandlers () {
	RK_TRACE (RBACKEND);

#ifdef Q_OS_WIN
	RKSignalSupportPrivate::default_sigsegv_handler = signal (SIGSEGV, SIG_DFL);
	RKSignalSupportPrivate::default_sigill_handler = signal (SIGILL, SIG_DFL);
	RKSignalSupportPrivate::default_sigabrt_handler = signal (SIGABRT, SIG_DFL);
#else
	sigaction (SIGSEGV, nullptr, &RKSignalSupportPrivate::default_sigsegv_handler);
	sigaction (SIGILL, nullptr, &RKSignalSupportPrivate::default_sigill_handler);
	sigaction (SIGABRT, nullptr, &RKSignalSupportPrivate::default_sigabrt_handler);
#endif
}

void RKSignalSupport::installSignalProxies () {
	RK_TRACE (RBACKEND);

#ifdef Q_OS_WIN
	RKSignalSupportPrivate::r_sigsegv_handler = signal (SIGSEGV, &RKSignalSupportPrivate::signal_proxy);
	RKSignalSupportPrivate::r_sigill_handler = signal (SIGILL, &RKSignalSupportPrivate::signal_proxy);
	RKSignalSupportPrivate::r_sigabrt_handler = signal (SIGABRT, &RKSignalSupportPrivate::signal_proxy);
#else
	// retrieve R signal handler
	sigaction (SIGSEGV, nullptr, &RKSignalSupportPrivate::r_sigsegv_handler);
	sigaction (SIGILL, nullptr, &RKSignalSupportPrivate::r_sigill_handler);
	sigaction (SIGABRT, nullptr, &RKSignalSupportPrivate::r_sigabrt_handler);

	// set new proxy handlers
	struct sigaction proxy_action;
	proxy_action = RKSignalSupportPrivate::r_sigsegv_handler;
	proxy_action.sa_flags |= SA_SIGINFO;
	proxy_action.sa_sigaction = &RKSignalSupportPrivate::signal_proxy;
	sigaction (SIGSEGV, &proxy_action, nullptr);

	proxy_action = RKSignalSupportPrivate::r_sigill_handler;
	proxy_action.sa_flags |= SA_SIGINFO;
	proxy_action.sa_sigaction = &RKSignalSupportPrivate::signal_proxy;
	sigaction (SIGILL, &proxy_action, nullptr);

	proxy_action = RKSignalSupportPrivate::default_sigabrt_handler;
	proxy_action.sa_flags |= SA_SIGINFO;
	proxy_action.sa_sigaction = &RKSignalSupportPrivate::signal_proxy;
	sigaction (SIGABRT, &proxy_action, nullptr);
#endif
}

void RKSignalSupport::installSigIntAndUsrHandlers (void (*handler) (void)) {
	RK_TRACE (RBACKEND);

	RK_ASSERT (!RKSignalSupportPrivate::r_sigint_handler);
	RKSignalSupportPrivate::new_sigint_handler = handler;
	RKSignalSupportPrivate::r_sigint_handler = signal (SIGINT, &RKSignalSupportPrivate::internal_sigint_handler);
#ifndef Q_OS_WIN
	// default action in R: save and quit. We use these as a proxy for SIGINT, instead.
	signal (SIGUSR1, &RKSignalSupportPrivate::internal_sigint_handler);
	signal (SIGUSR2, &RKSignalSupportPrivate::internal_sigint_handler);
#endif
}

void RKSignalSupport::callOldSigIntHandler () {
	RK_TRACE (RBACKEND);

	RKSignalSupportPrivate::r_sigint_handler (SIGINT);
}
