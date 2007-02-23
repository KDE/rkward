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

/* This code is mostly borrowed from WINE (http://www.winehq.org) */

void RKGetCurrentThreadStackLimits (size_t *size, void **base) {
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
	char dummy;
	size = pthread_get_stacksize_np (pthread_self ());
	base = pthread_get_stackaddr_np (pthread_self ());
	/* if base is too large assume it's the top of the stack instead */
	if ((char *) base > &dummy)
	base = (char *) base - size;
#else
#	warning Can't determine the stack limits of a pthread on this system
#	warning R C stack checking will be disabled
	char dummy;
	*base = &dummy;
	*size = (unsigned long) -1;
#endif
}
