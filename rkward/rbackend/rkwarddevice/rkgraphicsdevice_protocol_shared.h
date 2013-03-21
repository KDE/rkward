/***************************************************************************
                          rkgraphicsdevice  -  description
                             -------------------
    begin                : Mon Mar 18 20:06:08 CET 2013
    copyright            : (C) 2013 by Thomas Friedrichsmeier 
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

enum {
	// Asynchronous operations
	RKDCreate,
	RKDCircle,
	RKDLine,
	RKDPolygon,
	RKDPolyline,
	RKDRect,
	RKDTextUTF8,
	RKDNewPage,
	RKDClose,
	RKDActivate,
	RKDDeActivate,
	RKDClip,
	RKDMode,

	// Synchronous operations
	RKD_First_Synchronous_Request,
	RKDStrWidthUTF8,
	RKDMetricInfo,
	RKDLocator,
	RKDNewPageConfirm
} OpCodes;
