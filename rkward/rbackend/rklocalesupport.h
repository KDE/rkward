/***************************************************************************
                          rklocalesupport  -  description
                             -------------------
    begin                : Sun Mar 11 2007
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

#ifndef RKLOCALESUPPORT_H
#define RKLOCALESUPPORT_H

class QTextCodec;

/** Helper function to determine the QTextCodec best suited to recode the current CTYPE to UTF-8 */
QTextCodec *RKGetCurrentLocaleCodec ();

#endif
