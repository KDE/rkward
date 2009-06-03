/***************************************************************************
                          rkpthreadsupport  -  description
                             -------------------
    begin                : Fri Feb 23 2007
    copyright            : (C) 2007, 2009 by Thomas Friedrichsmeier
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

#include "rkpthreadsupport.h"

#include "pthread_config.h"

#include <qstring.h>
#include "../debug.h"

#ifdef Q_WS_WIN
#	include <windows.h>
#	include <stdint.h>	// for uintptr_t
#else
#	include <pthread.h>
#endif
#ifdef HAVE_PTHREAD_NP_H
#	include <pthread_np.h>
#endif

/* Much of this code is borrowed from WINE (http://www.winehq.org) */

void RKGetCurrentThreadStackLimits (size_t *size, void **base, char *reference) {
	int direction;
#ifdef HAVE_PTHREAD_GETATTR_NP
	pthread_attr_t attr;
	pthread_getattr_np (pthread_self (), &attr);
	pthread_attr_getstack (&attr, base, size);
	pthread_attr_destroy (&attr);
#elif defined(HAVE_PTHREAD_ATTR_GET_NP)
	pthread_attr_t attr;
	pthread_attr_init (&attr);
	pthread_attr_get_np (pthread_self (), &attr);
	pthread_attr_getstack (&attr, base, size);
	pthread_attr_destroy (&attr);
#elif defined(HAVE_PTHREAD_GET_STACKSIZE_NP) && defined(HAVE_PTHREAD_GET_STACKADDR_NP)
	*size = pthread_get_stacksize_np (pthread_self ());
	*base = pthread_get_stackaddr_np (pthread_self ());
#elif defined Q_WS_WIN
// This section (up to #endif) copied almost verbatim from R src/gnuwin32/system.c:
/*  Copyright (C) 1995, 1996  Robert Gentleman and Ross Ihaka
 *  Copyright (C) 1997--2007  Robert Gentleman, Ross Ihaka and the
 *                            R Development Core Team */
	{
		MEMORY_BASIC_INFORMATION buf;
		uintptr_t bottom, top;
		
		VirtualQuery(reference, &buf, sizeof(buf));
		bottom = (uintptr_t) buf.AllocationBase;
		top = (uintptr_t) buf.BaseAddress + buf.RegionSize;

		*base = (void*) top;
		*size = (size_t) (top - bottom);
	}
#else
#	warning Cannot determine the stack limits of a pthread on this system
#	warning R C stack checking will be disabled
	*base = reference;
	*size = (unsigned long) -1;
	return;
#endif
	// in which direction does the stack grow?
	{
		char dummy2;
		direction = reference > (&dummy2) ? 1 : -1;
	}

	// in which direction does the stack base lie?
	int base_direction = (*base) > reference ? 1 : -1;

	// switch base / top, if necessary
	if (base_direction != direction) {
		*base = ((char *) *base) + (direction * ((unsigned long) *size));
	}

	// sanity check, as on some systems the stack direction is mis-detected somehow.
	long usage = direction * ((unsigned long) (*base) - (unsigned long) (reference));
	if ((usage < 0) || (unsigned long) usage > (unsigned long) (*size)) {
		RK_DO (qDebug ("Stack boundaries detection produced bad results. Disabling stack checking."), RBACKEND, DL_WARNING);
		*size = (unsigned long) -1;
	}
}
