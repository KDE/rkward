/***************************************************************************
                          rkasyncdatastreamhelper  -  description
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

#ifndef RKASYNCDATASTREAMHELPER_H
#define RKASYNCDATASTREAMHELPER_H

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
 * 
 * Class is implemented as a template, so you can squeeze some bytes out of the protocol, if you know transmission chunks to be short
 * (e.g. never to exceed quint32). For maximum flexibility, use RKAsyncDataStreamHelper<quint64>.
 */
template <typename LENGTH_TYPE>
class RKAsyncDataStreamHelper {
public:
	RKAsyncDataStreamHelper () : 
			auxbuffer(), 
			inbuffer(),
			outbuffer(),
			auxstream (&auxbuffer, QIODevice::ReadWrite),
			instream (&inbuffer, QIODevice::ReadOnly), 
			outstream (&outbuffer, QIODevice::WriteOnly) {
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
		auxstream << (LENGTH_TYPE) outbuffer.size ();
		device->write (auxbuffer);
		device->write (outbuffer);
		outstream.device ()->seek (0);
		outbuffer.resize (0);
	}

/** @returns false if no complete chunk of data is available, yet. true, if the next chunk of data is available for
 * processing from instream. */
	bool readInBuffer () {
		if (!expected_read_size) {
			if (device->bytesAvailable () < (unsigned int) sizeof (LENGTH_TYPE)) {
				return false;
			} else {
				auxbuffer = device->read (sizeof (LENGTH_TYPE));
				auxstream.device ()->seek (0);
				auxstream >> expected_read_size;
			}
		}

		if ((LENGTH_TYPE) device->bytesAvailable () < expected_read_size) {
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

	int outSize () const {
		return outbuffer.size ();
	}
private:
	QIODevice *device;
	LENGTH_TYPE expected_read_size;
	// NOTE: Order of declaration of the buffers and streams is important, as these are initialized during construction, and depend on each other
	QByteArray auxbuffer;
	QByteArray inbuffer;
	QByteArray outbuffer;
	QDataStream auxstream;
public:
	QDataStream instream;
	QDataStream outstream;
};
 
#endif
