/***************************************************************************
                          rkpthreadsupport  -  description
                             -------------------
    begin                : Fri Feb 23 2007
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

#include "rkpthreadsupport.h"

#include "../../config.h"

#include <pthread.h>
#ifdef HAVE_PTHREAD_NP_H
#	include <pthread_np.h>
#endif

/* Much of this code is borrowed from WINE (http://www.winehq.org) */

void RKGetCurrentThreadStackLimits (size_t *size, void **base) {
	char dummy;
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
#else
#	warning Can't determine the stack limits of a pthread on this system
#	warning R C stack checking will be disabled
	*base = &dummy;
	*size = (unsigned long) -1;
	return;
#endif
	// in which direction does the stack grow?
	{
		char dummy2;
		direction = (&dummy) > (&dummy2) ? 1 : -1;
	}

	// in which direction does the stack base lie?
	int base_direction = (*base) > (&dummy) ? 1 : -1;

	// switch base / top, if necessary
	if (base_direction != direction) {
		*base = ((char *) *base) + (direction * ((unsigned long) *size));
	}
}
