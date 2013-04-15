/***************************************************************************
                          rkgraphicsdevice_protocol_shared  -  description
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

#ifndef RKGRAPHICSDEVICE_PROTOCOL_SHARED_H
#define RKGRAPHICSDEVICE_PROTOCOL_SHARED_H

/** @page RKGraphicsDeviceProtocol
 * 
 * The key feature of the RKWard Graphics Device is that it serializes all drawing operations, so they
 * can be sent to a separate process (the frontend) or even computer (well, not yet). Some notes on the protocol:
 * 
 * This is not the same protocol as used for other communication between frontend and backend, and not even
 * the same connection. The key idea behind this protocol here, is that it should have very low overhead,
 * even when sending many @em small requests.
 * 
 * All communication is initiated from the backend. The backend sends a request, starting with the size of the
 * request in bytes (quint32), then an opcode (quint8), then the device number (quint8), then all applicable parameters.
 * Most requests are asynchronous, but a few await a reply from the frontend.
 * 
 * At any time, there can only be one request waiting for a reply, and the request waiting for a reply is always the most
 * recent one. This makes the protocol very simple.
 * 
 * If the frontend has spontaneous need for communication, it will have to use some separate channel.
 *
 * How do we handle cancellation of interactive ops (e.g. locator()) from the backend? If an interrupt is pending
 * in the backend, _while waiting for the reply_, we push an RKD_Cancel request down the line. This tells the frontend to
 * send a reply to the last request ASAP (if the frontend has already sent the reply, it will ignore the RKD_Cancel). From
 * there, we simply process the reply as usual, and leave it to R to actually do the interrupt. If the frontend takes more than
 * fives seconds to respond at this point, the connection will be killed.
 * 
 */

/** This enum simply repeats R's line end definitions. It is used to ensure compatiblity, without the need to include
 * any R headers in the frontend. */
enum RKLineEndStyles {
	RoundLineCap = 1,
	ButtLineCap = 2,
	SquareLineCap = 3
};

/** This enum simply repeats R's line join definitions. It is used to ensure compatiblity, without the need to include
 * any R headers in the frontend. */
enum RKLineJoinStyles {
	RoundJoin = 1,
	MitreJoin = 2,
	BevelJoin = 3
};

enum RKDOpcodes {
	// Asynchronous operations
	RKDCreate,     // 0
	RKDCircle,
	RKDLine,
	RKDPolygon,
	RKDPolyline,
	RKDRect,       // 5
	RKDTextUTF8,
	RKDNewPage,
	RKDClose,
	RKDActivate,
	RKDDeActivate, // 10
	RKDClip,
	RKDMode,
	RKDRaster,

	// Synchronous operations
	RKDStrWidthUTF8,
	RKDMetricInfo, // 15
	RKDLocator,
	RKDNewPageConfirm,
	RKDCapture,
	RKDQueryResolution,

	// Protocol operations
	RKDCancel      // 20
};

#include <QtGlobal>
typedef quint32 RKGraphicsDeviceTransmittionLengthType;

#endif
