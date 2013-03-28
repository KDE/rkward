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

	// Synchronous operations
	RKD_First_Synchronous_Request,
	RKDStrWidthUTF8,
	RKDMetricInfo, // 15
	RKDLocator,
	RKDNewPageConfirm,

	// Protocol operations
	RKDCancel
};

#include <QIODevice>
#include <QDataStream>
#include <QByteArray>

/** Using a QDataStream on an asynchronous connection is somewhat cumbersome due to the need to ensure that chunks of
 * data are complete, when we process them. This small class helps with that. Essentially:
 *
 * - write to outstream
 * - when a chunk is done, push it to the device using writeOutBuffer().
 * 
 * To read a chunk call
 * - readInBuffer() repeatedly, until it returns true
 * - read from instream
 */
class RKAsyncDataStreamHelper {
public:
	RKAsyncDataStreamHelper () : instream (&inbuffer, QIODevice::ReadOnly), outstream (&outbuffer, QIODevice::WriteOnly), auxstream (&auxbuffer, QIODevice::ReadWrite) {
		device = 0;
		expected_read_size = 0;
	}
	~RKAsyncDataStreamHelper () {};

	void setIODevice (QIODevice *_device) {
		device = _device;
	}

	void writeOutBuffer () {
		auxstream.device ()->seek (0);
		auxbuffer.resize (0);
		auxstream << (quint32) outbuffer.size ();
		device->write (auxbuffer);
		device->write (outbuffer);
		outstream.device ()->seek (0);
		outbuffer.resize (0);
	}

/** @returns false if no complete chunk of data is available, yet. true, if the next chunk of data is available for
 * processing from instream. */
	bool readInBuffer () {
		if (!expected_read_size) {
			if (device->bytesAvailable () < sizeof (quint32)) {
				return false;
			} else {
				auxbuffer = device->read (sizeof (quint32));
				auxstream.device ()->seek (0);
				auxstream >> expected_read_size;
			}
		}

		if (device->bytesAvailable () < expected_read_size) {
			return false;
		}

		inbuffer = device->read (expected_read_size);
		instream.device ()->seek (0);
		expected_read_size = 0;
		return true;
	}

	int inSize () const {
		return inbuffer.size ();
	}
	
	QDataStream instream;
	QDataStream outstream;
private:
	QIODevice *device;
	quint32 expected_read_size;
	QByteArray inbuffer;
	QByteArray outbuffer;
	QByteArray auxbuffer;
	QDataStream auxstream;
};

#endif
