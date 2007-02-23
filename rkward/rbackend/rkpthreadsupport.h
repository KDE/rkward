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

#ifndef RKPTHREADSUPPORT_H
#define RKPTHREADSUPPORT_H

#include <stddef.h>

/** Try different ways to get the stack limits of the currently running thread */
void RKGetCurrentThreadStackLimits (size_t *size, void **base);

#endif
