/*
rkreventloop - This file is part of the RKWard project. Created: Tue Apr 23 2013
SPDX-FileCopyrightText: 2013 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKREVENTLOOP_H
#define RKREVENTLOOP_H

#include <qglobal.h>  // For Q_OS_WIN

namespace RKREventLoop {
/** This - somewhat misnamed - function takes care of processing R's events (not only X11).
 *  It is like R_ProcessEvents (except for the scary warning in the header file, that R_ProcessEvents does not work on Unix),
 *  but for safety, event processing is done inside a toplevel context. Thus, this function is guaranteed to return.
 *  Call this periodically to make R's x11 windows process their events */
	void processX11Events ();
/** Register a function that will be called inside R's event loop. Implementation differs across platforms. The handler function
 * @em might get called unconditionally (hooked into R_PolledEvents()), periodically, or only when needed. To make sure, the
 * latter works, call wakeRKEventHandler() whenever there is(are) some new task(s) for the handler.
 * 
 * Event on platforms where the event handler is called "on demand", there is no guarantee, that this happens exactly once per demand.
 * The registered event handling function should be prepared to handle zero, one, or several new events.
 * 
 * Currently, only one handler may be registered. */
	void setRKEventHandler (void (* handler) ());
/** Call this (potentially from a separate thread) to wake the handler set by setRKEventHandler() in the next iteration of 
 * R's event loop. */
	void wakeRKEventHandler ();
#ifdef Q_OS_WIN
/** On Windows, Rp->Callback has to be set to this function during startup. */
	void winRKEventHandlerWrapper (void);
#endif
};

#endif
